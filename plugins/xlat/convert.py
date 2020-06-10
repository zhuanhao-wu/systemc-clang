from lark import Lark, Transformer, Visitor
import sys, traceback
import logging
import warnings
import re
import argparse
logging.basicConfig(level=logging.DEBUG)

# TODO: group syntax together to reflect the construct

l = Lark('''
        start: modulelist typelist

        modulelist: (hmodule)*
        typelist: (htypedef)*
        hmodule:  "hModule" ID "[" modportsiglist? processlist* portbindinglist? "]"

        modportsiglist: "hPortsigvarlist" "NONAME" "[" modportsigdecl+ "]" 

        ?modportsigdecl: portdecltype 
                      | sigdecltype 
                      | vardeclinit 
        portdecltype: portdecl "[" htypeinfo "]"
        sigdecltype: sigdecl "[" htypeinfo "]"
        sigdecl:  "hSigdecl" ID  
        ?portdecl: inportdecl | outportdecl
        inportdecl:  "hPortin" ID 
        outportdecl: "hPortout" ID
        vardeclinit: "hVardecl" ID "[" htypeinfo hvarinit? "]"
        ?hvarinit: "hVarInit" "NONAME" expression
        // can be no process at all in the module
        ?processlist:  "hProcesses" "NONAME" "[" hprocess* "]"
        // could be nothing
        // temporarily ignore the hMethod node
        hprocess:  "hProcess" ID  "[" hsenslist*   "hMethod" "NONAME" "[" prevardecl hcstmt "]" "]"
        prevardecl: vardecl*
        vardecl: vardeclinit

        // can be just an empty statement
        hcstmt:  "hCStmt" "NONAME" "[" modportsiglist* stmt+ "]" // useful for raising variable decls
              |  "hCStmt" "NONAME" "NOLIST"

        stmt : expression_in_stmt
             | syscwrite
             | ifstmt
             | forstmt
             | hcstmt
             | switchstmt

        // Port Bindings
        portbindinglist: "hPortbindings" "NONAME" "[" portbinding* "]"
        portbinding: "hPortbinding" "NONAME" "[" hvarref hvarref "]"


        // This is solely for maintaining the semicolon
        expression_in_stmt: expression

        // for(forinit; forcond; forpostcond) stmts
        forstmt: "hForStmt" "NONAME" "[" forinit forcond forpostcond forbody "]"
        forinit: "hPortsigvarlist" "NONAME" "[" vardeclinit  "]"
                 | vardeclinit
                 | "<null child>"
        forcond: expression
        forpostcond: expression
        forbody: stmt

        switchstmt: "hSwitchStmt" "NONAME" "[" switchcond switchbody "]"
        switchcond: expression
        // Note: we don't make this a noraml statement as in the context of switch, 
        // we don't use general statements
        switchbody: "hCStmt" "NONAME" "[" casestmt+ "]"
        casestmt: "hSwitchCase" "NONAME" "[" casevalue stmt "]" "hNoop" "NONAME" "NOLIST"
                | "hSwitchCase" "NONAME" "[" casevalue "hNoop" "NONAME" "NOLIST" "]"
        casevalue: expression

        hsenslist : "hSenslist" "NONAME" "[" hsensvars "]"
                  | "hSenslist" "NONAME" "NOLIST"
        hsensvar :  "hSensvar" ID "[" hsensedge "]"
        hsensvars : hsensvar*

        hsensedge : "hSensedge" npa "NOLIST"
        !npa : "neg" | "pos" | "always"

        // if and if-else, not handling if-elseif case
        ifstmt: "hIfStmt" "NONAME" "[" expression  stmt stmt?"]"

         
        ?expression: hbinop
                  | hunop
                  | hliteral
                  | vardeclinit
                  | hvarref
                  | hunimp
                  | syscread
                  | hmethodcall
                  |  "[" expression "]"

        syscread : hsigassignr "[" expression "]"
        syscwrite : hsigassignl "["  expression  expression "]"
        ?hsigassignr :  "hSigAssignR" "read" 
        ?hsigassignl :  "hSigAssignL" "write" 
        // function call
        hvarref : "hVarref" ID "NOLIST"
        hunimp:  "hUnimpl" ID "NOLIST"
        hbinop:  "hBinop" BINOP "[" expression expression "]"
        hunop:  "hUnop" UNOP "[" expression "]"
        hmethodcall: "hMethodCall" hidorstr  "[" expression expression* "]" 
        ?hidorstr: ID | STRING
        hliteral:  "hLiteral" ID "NOLIST"
        htypeinfo: "hTypeinfo" "NONAME" "[" htype+ "]"
                 | "hTypeinfo" "NONAME" "NOLIST" // ?
        htype: "hType" TYPESTR "NOLIST" 
             | "hType" TYPESTR "[" htype+ "]"     // nested types
             | "hType" "unsigned" "int" "NOLIST"  // a work around, should be surrounded with quotes
             | htypeint
        htypeint: "hType" NUM "NOLIST"  // integer type parameters
        htypedef: "hTypedef" TYPESTR "[" htype+ "]"
        ID: /[a-zA-Z_0-9]+/
        NUM: /(\+|\-)?[0-9]+/
        TYPESTR: /[a-zA-Z_][a-zA-Z_0-9]*/
        BINOP: "==" | "&&" | "=" | "||" | "-" | ">" | "+" | "*" | "^" | "ARRAYSUBSCRIPT" | "<=" | "<" | "%" | "!=" | "&"
        UNOP: "!" | "++" | "-"
        %import common.WS
        %ignore WS
        %import common.ESCAPED_STRING -> STRING
        ''', parser='lalr', debug=True)


class CType2VerilogType(object):
    type_map = { '_Bool': '[0:0]',
                 'sc_in': 'input logic',
                 'sc_out': 'output logic',
                 'sc_signal': 'logic',
                 'int': 'integer'}

    @staticmethod
    def varref_wrapper(t, v):
        if t == 'double':
            return '$bitstoreal(v)'
        else:
            return v

    @staticmethod
    def convert_type_list(l):
        """converts a list of htype info into proper type, also annotate for intended types for necessary conversion"""
        res = []
        in_port = 'sc_in' in l
        out_port = 'sc_out' in l
        # we almost always use reg, reg also for implicit values
        wire_or_reg = 'logic'
        if in_port:
            wire_or_reg = 'logic'
        bw = None
        vtype = None
        l_iter = iter(l)
        for arg in l_iter:
            if arg == 'sc_in':
                in_port = True
                wire_or_reg = 'logic'
            elif arg == 'sc_out':
                out_port = True
            else: 
                if arg in ['sc_bv', 'sc_int']:
                    if bw is None:
                        bw = int(next(l_iter))
                    else:
                        assert False, 'incorrect htype specification'
                elif arg in ['double', 'int']:
                    # bw = CType2VerilogType.get_width(arg)
                    if arg == 'double':
                        vtype = 'real'
                    elif arg == 'int':
                        vtype = 'integer'
        inout = ''
        if in_port:
            inout = 'input'
        elif out_port:
            inout = 'output'
        if bw is None:
            width_or_type_str = ''
            if vtype == 'real':
                width_or_type_str = ' real'
            elif vtype == 'integer':
                width_or_type_str = ' [31:0]'
        else:
            width_or_type_str = f' [{bw-1}:0]'
        if inout == '':
            space_inout = ''
        else:
            space_inout = ' '
        return [f'{inout}{space_inout}{wire_or_reg}{width_or_type_str}', vtype]

        return ''
    @staticmethod
    def get_width(t):
        if t == 'double':
            return 64
        elif t == 'int':
            return 32
        else:
            assert False


# TODO: use decorators to embed context
# TODO: use context object instead of string
# TODO: not clear when to decide the `logic' part
class sc_in(object):
    def __init__(self, T):
        self.T = T

    def to_str(self, context=None):
        return 'input {}'.format(self.T.to_str(context='sc_in'))


class sc_out(object):
    def __init__(self, T):
        self.T = T

    def to_str(self, context=None):
        return 'output {}'.format(self.T.to_str(context='sc_out'))


class sc_uint(object):
    def __init__(self, width):
        self.width = width

    def to_str(self, context=None):
        if context == 'sc_signal':
            return f'logic [{self.width-1}:0]'
        else:
            return f'logic [{self.width-1}:0]'

class sc_int(object):
    def __init__(self, width):
        self.width = width

    def to_str(self, context=None):
        if context == 'sc_signal':
            return f'logic [{self.width-1}:0]'
        else:
            return f'logic [{self.width-1}:0]'


class sc_signal(object):
    def __init__(self, T):
        self.T = T

    def to_str(self, context=None):
        return self.T.to_str(context='sc_signal')


class cppbool(object):
    def __new__(cls):
        return sc_uint(1)


class cppint(object):
    def __new__(cls):
        return sc_int(32)


class cppuint(object):
    def __new__(cls):
        return sc_uint(32)

# aggregated type?


class CType(object):
    """a helper class for generating signal/ports with correct type"""
    def __init__(self, node_type, var_name, type_info):
        self.node_type = node_type
        self.var_name = var_name
        self.type_info = type_info

    @staticmethod
    def type_from_str(type_name, *params):
        if type_name == '_Bool':
            return cppbool()
        elif type_name == 'unsigned int':
            return cppuint()
        elif type_name == 'int':
            return cppint()
        elif type_name == 'sc_in':
            return sc_in(params[0])
        elif type_name == 'sc_out':
            return sc_out(params[0])
        elif type_name == 'sc_uint':
            return sc_uint(params[0])
        elif type_name == 'sc_signal':
            return sc_signal(params[0])
        elif type(type_name) == type(0):
            return type_name
        else:
            raise

    def generate(self):
        print(self.var_name)
        if self.node_type == 'hPortin':
            return self.__as_port_in()
        elif self.node_type == 'hPortout':
            return self.__as_port_out()
        elif self.node_type == 'hSigdecl':
            return self.__as_signal()
        elif self.node_type == 'hVardecl':
            return self.__as_var_decl()
        else:
            assert False

    @staticmethod
    def get_width(type_info):
        print(type_info)
        assert False

    @staticmethod
    def flatten(prefix, type_info):
        """
        flatten returns the flattend field, all type information is put here
        the method returns a list of (field_name, width) tuples
        NOTE: this method does not handle port direction, also not handling nested types yet
        """
        print(type_info)
        if type_info[0] in ['sc_in', 'sc_out']:
            return CType.flatten(prefix, type_info[1:])
        elif type_info[0] == '_Bool':
            return [(prefix, 1)]
        elif type_info[0] in ['sc_uint', 'sc_bv', 'sc_int']:
            return [(prefix, int(type_info[1]))]
        elif type_info[0] == 'fp_t':
            return [(prefix, 1 + int(type_info[2]) + int(type_info[1]))]
        elif type_info[0] in ['int', 'unsigned_int']:
            return [(prefix, 32)]
        else:
            assert False


    def __str__(self):
        return f'CType(node={self.node_type}, var={self.var_name})'

    def __repr__(self):
        return self.__str__()

    def __as_port_in(self):
        print('...', self.type_info)
        print('...', self.type_info[0].to_str() + ' ' + self.var_name)
        return self.type_info[0].to_str() + ' ' + self.var_name
        flattened = CType.flatten(self.var_name, self.type_info)
        print('flattend: ', flattened)
        res = ',\n'.join(f'input logic [{p[1] - 1}:0] {p[0]}' for p in flattened)
        return res

    def __as_port_out(self):
        print('...', self.type_info)
        print('...', self.type_info[0].to_str() + ' ' + self.var_name)
        return self.type_info[0].to_str() + ' ' + self.var_name
        if self.var_name == 'm_block':
            warnings.warn('Temporarily bypassing array of ports')
            return '/* m_block not implemented */'
        print('...', self.type_info)
        flattened = CType.flatten(self.var_name, self.type_info)
        print('flattend: ', flattened)
        res = ',\n'.join(f'output logic [{p[1] - 1}:0] {p[0]}' for p in flattened)
        return res

    def __as_signal(self):
        print('SIG', self.type_info)
        print('SIG', self.type_info[0].to_str())
        assert len(self.type_info) == 1
        return self.type_info[0].to_str() + ' ' + self.var_name
        if len(self.type_info) == 0:
            if self.var_name == 'c_fc_block':
                warnings.warn('Temporarily bypassing array declaration for c_fc_block')
                return 'logic [10:0] c_fc_block[0:3];'
            elif self.var_name == 'data': 
                warnings.warn('Temporarily bypassing array declaration for data')
                return 'logic [10:0] data[0:depth-1];'
            else:
                assert False
        if self.type_info[0] == 'sc_signal':
            print('...', self.type_info)
            flattened = CType.flatten(self.var_name, self.type_info[1:])
            print('flattend: ', flattened)
            res = '\n'.join(f'logic [{p[1] - 1}:0] {p[0]};' for p in flattened)
            return res
        else:
            assert False

    def __as_var_decl(self):
        """Could also be module declaration as well"""
        # print('>>>', self.type_info)
        # print('===', self.type_info[0].to_str())
        assert len(self.type_info) == 1
        return self.type_info[0].to_str() + ' ' + self.var_name
        if self.type_info[0] in ['sc_uint', 'sc_int', '_Bool', 'fp_t', 'sc_bv', 'int', 'unsigned_int']:
            print('...', self.type_info)
            flattened = CType.flatten(self.var_name, self.type_info)
            print('flattend: ', flattened)
            res = '\n'.join(f'logic [{p[1] - 1}:0] {p[0]};' for p in flattened)
            return res
        elif self.type_info[0] == 'sc_rvd':
            print('...', self.type_info)
            flattened = CType.flatten(self.var_name, self.type_info[1:])
            print('flattend: ', flattened)
            res = ',\n'.join(f'logic [{p[1] - 1}:0] {p[0]}' for p in flattened)
            res += ',\n' + f'logic {self.var_name}_valid'
            res += ',\n' + f'logic {self.var_name}_ready'
            return res
        elif self.var_name in ['u_find_emax', 'u_fwd_cast', 'u_que_fp', 'u_reg_ex']:
            warnings.warn('TODO: manually instantiating modules')
            return f'{self.type_info[0]} {self.var_name}();'
        elif self.type_info[0] in ['sc_signal']: # we sort of mixing signal declarations and vardecl
            print('...', self.type_info)
            flattened = CType.flatten(self.var_name, self.type_info[1:])
            print('flattend: ', flattened)
            res = '\n'.join(f'logic [{p[1] - 1}:0] {p[0]};' for p in flattened)
            return res
        elif self.type_info[0] in ['sc_in', 'sc_out']:
            # these are for signal declaration
            print('...', self.type_info)
            flattened = CType.flatten(self.var_name, self.type_info[1:])
            print('flattend: ', flattened)
            res = '\n'.join(f'logic [{p[1] - 1}:0] {p[0]};' for p in flattened)
            return res
        else:
            assert False


debug = True
def p(decorated):
    """a decorator that helps printing out the transformation results"""
    if debug:
        def wrapper(self, args):
            print(f'[DBG] {decorated.__name__}: \n{args} \n\n')
            res = decorated(self, args)
            return res
        return wrapper
    else:
        return decorated


def flatten(L):
    """a simple flatten helper"""
    for item in L:
        try:
            if isinstance(item, str) or isinstance(item, tuple):
                yield item
            else:
                yield from flatten(item)
        except TypeError:
            yield item



class VerilogTransformer(Transformer):

    def __init__(self, skip=None, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self.vardecl_map = dict()
        self.global_type_map = dict()
        self.indent_width = 2

        if skip:
            self.skip = skip
        else:
            self.skip = []

    def start(self, args):
        return args[0]

    @p
    def htypeinfo(self, args):
        """resolving type information"""

        # drop sc_signal for types without width
        # if args[0] == 'reg' and args[1] != 'sc_bv':
        #     args = args[1:]

        # res = CType2VerilogType.convert_type_list(args)
        # return res
        return args

    def hliteral(self, args):
        return str(args[0])

    @p
    def hlitdecl(self, args):
        return args

    @p
    def hcstmt(self, args):
        stmt_list = []
        for idx, stmt in enumerate(args):
            if isinstance(stmt, list):
                stmt_list.extend(stmt)
            else:
                if stmt:
                    stmt_list.append(stmt)
        # currently it's ok to append a comma
        print("stmtlist in hcstmt is ", args, stmt_list)
        res = '\n'.join(x for x in stmt_list)
        return res

    def hbinop(self, args):
        op = str(args[0])
        if op == '=':
            return f'{args[1]} {args[0]} {args[2]}'
        elif op == 'ARRAYSUBSCRIPT':
            return f'{args[1]} [ {args[2]} ]'
        else:
            return f'({args[1]}) {args[0]} ({args[2]})'

    def hunop(self, args):
        if len(args) == 1:
            return f'{args[0]}'
        elif len(args) == 2:
            op = str(args[0])
            if op == '++':
                # only used in the for loop
                return f'{args[1]}={args[1]}+1'
            elif op == '--':
                # only used in the for loop
                return f'{args[1]}={args[1]}-1'
            else:
                return f'{args[0]}({args[1]})'
    @p
    def hmethodcall(self, args):
        print("in hmethodcall, returning ", f'{args[0]}',"(", f'{",".join(args[1:])}', ")")
        return (f'{args[0]}(' f'{",".join(args[1:])})')

    def stmts(self, args):
        return args

    @p
    def vardeclonly(self, args):
        assert(isinstance(args[1], list))
        assert(len(args[1]) == 2)
        self.vardecl_map[str(args[0])] = args[1]
        return None

    @p
    def vardeclinit(self, args):
        print("vardeclinit: ", args)
        ctype = CType('hVardecl', args[0], args[1])
        self.vardecl_map[str(args[0])] = ctype
        if len(args)==3:
            return f'{args[0]} = {args[2]}'
        return None
    
    @p
    def htype(self, args):
        # NOTE: the name of htype will always be a STRING
        # NOTE: args[0][1:-1] removes the quotes
        # return CType2VerilogType.convert(str(args[0]))
        # return str(args[0])
        return CType.type_from_str(args[0], *args[1:])

    def hunimp(self, args):
        return f'\"//# Unimplemented: {args[0]}\"'

    @p
    def ifstmt(self, args):
        if len(args) == 2:  # if
            return (f"if({args[0]}) begin\n"
            f"{args[1]}\n"
            f"end"
            )
        elif len(args) == 3: # if-else
            return (f"if({args[0]}) begin\n"
            f"{args[1]}\n"
            f"end else begin\n"
            f"{args[2]}\n"
            f"end"
            )

    def ifcond(self, args):
        if str(args[0]) == '<null child>':
            return f'\"NULL CHILD\"'
        return f'{args[0]}'

    @p
    def hprocess(self, args):
        print(args)
        vardecl_str = '\n'.join(map(lambda x: x[1].generate(), self.vardecl_map.items()))
        senslist = '*' if len(args[1]) == 0 else args[1]
        res = (f"always @({senslist}) begin: {args[0]}\n"
               f"{vardecl_str}\n"
               f"{args[3]}\n"  # TODO: this place needs to be changed
               f"end // {args[0]}")
        self.vardecl_map.clear()
        return res

    @p
    def portsiglist(self, args):
        return args

    def processlist(self, args):
        return '\n\n'.join(args)

    @p
    def inportdecl(self, args):
        return ['hPortin', str(args[0])]

    @p
    def portdecltype(self, args):
        portType = args[0][0]
        portName = args[0][1]
        htypeInfo = args[1]
        return ('port', CType(portType, portName, htypeInfo))

    @p
    def outportdecl(self, args):
        return ['hPortout', str(args[0])]

    @p
    def sigdecltype(self, args):
        sigType = args[0][0]
        sigName = args[0][1]
        htypeInfo = args[1]

        return ('sig', CType(sigType, sigName, htypeInfo))
    
    def sigdecl(self, args):
        return ['hSigdecl', str(args[0])]

    @p
    def hmodule(self, args):
        print("Parsing hModule")
        print("args: ", args)
        print("len(args): ", len(args))
        modname = str(args[0])
        if modname in self.skip:
            return ''
        print(modname)
        if len(args)>1:
            portsiglist = args[1]
            print("portsiglist ", portsiglist)
            if (len(args)>2):
                proclist = args[2]
            else:
                proclist = None
        else:
            portsiglist = None


        # separate port from list of port, sig
        if portsiglist:
            portlist = list(map(lambda x: x[1].generate(), filter(lambda x: x is not None and  x[0] == 'port', portsiglist[0])))
            print(portlist)
            portstr = ',\n'.join(portlist)
            
            siglist = list(map(lambda x: x[1].generate(), filter(lambda x: x is not None and  x[0] == 'sig', portsiglist[0])))
            # sigstr = ';\n'.join(siglist) + (';' if len(siglist) > 0 else '')  # the last semi-colon
            sigstr = '\n'.join(siglist)
            
            varlist = portsiglist[1]# list(map(lambda x: x[1], filter(lambda x: x[0] == 'var', portsiglist[1])))
            # varstr = ';\n'.join(varlist) + (';' if len(varlist) > 0 else '')
            varstr = '\n'.join(map(lambda x: x.generate(), varlist))
        print(portstr)
        print(sigstr)

        # for the processes
        procstr = proclist

        res = (f"module {modname}(\n"
                f"{portstr} \n"
                f");\n"
                f"{varstr}\n"
                f"{sigstr}\n"
                f"{proclist}\n"
                f"endmodule // {modname}"
                )
        return res
    def modulelist(self, args):
        return "\n".join(args)


    @p
    def hsensvar(self, args):
        if args[1] == 'pos':
            return f'posedge {args[0]}'
        elif args[1] == 'neg':
            return f'negedge {args[0]}'
        elif args[1] == 'always':
            return args[0]
        else:
            raise RuntimError('edge sensitivity should be one of pos/neg/always')

    @p
    def hsenslist(self, args):
        assert len(args) <= 1
        if len(args) == 0:
            return '*'
        else:
            warnings.warn("Temporary hack to bypass the sensitivity list")
            return "posedge clock"
            # return args[0]

    def hsigassignl(self, args):
        return None
    def hsigassignr(self, args):
        return None
    def syscwrite(self, args):
        return f'{args[1]} <= {args[2]};'
    def syscread(self, args):
        return f'{args[1]}'

    @p
    def vardecltype(self, args):
        assert(len(args) == 1)
        # return ('var', f'{args[1]} {args[0]}')
        # return ('var', f'{args[0]}')
        return args[0]

    @p
    def hsensvars(self, args):
        return ' or '.join(args)

    def fcall(self, args):
        fname = str(args[0])
        caller = str(args[1])
        if fname == 'to_int':
            return caller
        else:
            raise NotImplementedError
        
    @p
    def npa(self, args):
        return str(args[0])

    @p
    def hsensedge(self, args):
        return args[0]

    def hvarref(self, args):
        return str(args[0])
        # if str(args[0]) in self.vardecl_map:
        #     return CType2VerilogType.varref_wrapper(
        #         self.vardecl_map[args[0]][1], 
        #         str(args[0])
        #     )
        # else:
        #     return str(args[0]) + '/* definition not found */'

    @p
    def hnoop(self, args):
        if len(args) == 2:
            return str(args[1])
        elif len(args) == 1:
            return str(args[0])
        elif len(args) == 3 and args[0] == 'next_trigger':
            return '#' + str(args[1])
        else:
            assert False

    @p
    def modportsiglist(self, args):
        args = list(flatten(args))
        var_list = []
        for k, v in self.vardecl_map.items():
            # var_list.append(f'{v[0]} {k}')
            # move these signals to the global scope
            var_list.append(v)
            self.global_type_map[k] = v
        self.vardecl_map.clear()
        print(var_list)
        return [args, var_list]

    @p
    def forinit(self, args):
        # note: var decl is raised up to the top
        if len(args) == 0:  # '<null child>'
            return None
        else:
            return args[0]

    @p
    def forcond(self, args):
        return args[0]

    @p
    def forpostcond(self, args):
        # note: the args might be wrong
        return args[0]

    @p
    def forbody(self, args):
        return args

    @p
    def forstmt(self, args):
        forinit, forcond, forpostcond, forbody = args
        forbody = '\n  '.join(forbody)
        res = [
                f'for({forinit};{forcond};{forpostcond}) begin',
                f'  {forbody}',
                f'end'
            ]
        return '\n'.join(res)

    @p
    def expression_in_stmt(self, args):
        if args[0] is not None:
            return args[0] + ';'
        else:
            return args[0]

    @p
    def stmt(self, args):
        assert(len(args) == 1)
        print('===stmt===', args)
        # return args[0] + ';' if args[0] is not None and args[0][-1] != ';' else args[0]
        return args[0]

    def casevalue(self, args):
        return args[0]

    def casestmt(self, args):
        case_value = args[0]
        if len(args) > 1:
            case_body = args[1]
        else:
            case_body = None
        return [case_value, case_body]

    def switchbody(self, args):
        return args

    def switchcond(self, args):
        return args[0]

    def switchstmt(self, args):
        cond = args[0]
        body = args[1]
        body_code = ''
        for v, b in body:
            if b is not None:
                body_code = body_code + f'{v} : begin\n' + f'{b}\n' + 'end\n'
            else:
                body_code = body_code + f'{v} : begin\n' + f'\n' + 'end\n'
        switch_code = f"case({cond})\n" + f"{body_code}\n" + f"endcase"
        return switch_code

    def portbindinglist(self, args):
        return args

    def portbinding(self, args):
        assert len(args) == 2
        warnings.warn('Port binding not implmented: {} - {}'.format(args[0], args[1]))
        return None

    def prevardecl(self, args):
        warnings.warn('prevardecl not implemented (currently using varinit)')
        return ""

    def htypeint(self, args):
        return int(args[0])


def tidify(verilog, current_indent = 0, indent_width = 2):
    """makes the generated verilog looks a bit better, may be subject to changes later"""
    add_indent_pattern = re.compile(r'(^(module|if|always|for|case)|(\: begin))')
    sub_indent_pattern = re.compile(r'^(endmodule|end)')
    sub_add_indent_pattern = re.compile(r'^(\)\;|end else begin)')
    res = []
    for l in verilog.splitlines():
        if add_indent_pattern.search(l):
            s = ' ' * (current_indent) + l
            current_indent += indent_width
        elif sub_add_indent_pattern.search(l):
            s = ' ' * (current_indent - indent_width) + l
        elif sub_indent_pattern.search(l):
            current_indent -= indent_width
            s = ' ' * (current_indent) + l
        else:
            s = ' ' * current_indent + l
        # print(s)
        res.append(s)

    return '\n'.join(res)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('input', type=str, help='Input file name (normally the _hdl.txt file)')
    parser.add_argument('--output', type=str, help='The outpuf filename')
    parser.add_argument('--skip', nargs='+', help='the list of modules to skip generating, useful for removing not-synthesizable modules')
    args = parser.parse_args()
    filename = args.input
    outputname = filename + ".v"
    if args.output is not None:
        outputname = args.output
    with open(filename, 'r') as f:
        file_content = f.read()
    try:
        t = l.parse(file_content)
    except Exception as e:
        print(e)
        exit(2)  # This code has specific meanings, should be tested in the verilog tests
    try:
        res = VerilogTransformer(skip=args.skip).transform(t)
    except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        print("***** print_exception:")
        # exc_type below is ignored on 3.5 and later
        traceback.print_exception(exc_type, exc_value, exc_traceback, file=sys.stdout)
        exit(3)
    res = tidify(res)
    with open(outputname, 'w+') as f:
        f.writelines(res)
        f.write("\n\n")
    print(res)




if __name__ == '__main__':
    main()
    # l.parse('')
