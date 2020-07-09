import warnings
from .top_down import TopDown
from ..primitives import *
from lark import Tree


class VerilogTranslationPass(TopDown):
    """Translate low-level format of the _hdl.txt into Verilog
    Note that type defs are already expanded at this point, so all htypeinfo/htype should only include primitive types
    This pass does not perform any tree transformation that alters the semantics, but **only** generates Verilog
    """
    def __init__(self):
        super().__init__()
        self.indent_character = ' '
        self.current_indent = 0
        self.indent_inc = 2
        self.indent_stack = list()
        self.bindings = dict()

    def start(self, tree):
        self.__push_up(tree)
        # print(tree)
        return tree

    def blkassign(self, tree):
        self.__push_up(tree)
        assert len(tree.children) == 2
        res = '{} = {}'.format(tree.children[0], tree.children[1])
        return res

    def syscwrite(self, tree):
        self.__push_up(tree)
        res = '{} {} {}'.format(tree.children[1], tree.children[0], tree.children[2])
        return res

    def hliteral(self, tree):
        """stops at literal, it is some kinds of terminal"""
        assert len(tree.children) == 1
        return tree.children[0]

    def hvarref(self, tree):
        assert len(tree.children) == 1
        return tree.children[0]

    def syscread(self, tree):
        """syscread: hsigassignr, token"""
        self.__push_up(tree)
        return tree.children[1]

    def harrayref(self, tree):
        self.__push_up(tree)
        return '{}[{}]'.format(tree.children[0], tree.children[1])

    def hsigassignl(self, tree):
        warnings.warn('Implementing SigAssignL as non-blocking')
        return '<='

    def hbinop(self, tree):
        self.__push_up(tree)
        if tree.children[0] == 'ARRAYSUBSCRIPT':
            res = '{}[({})]'.format(tree.children[1], tree.children[2])
        else:
            res = '({}) {} ({})'.format(tree.children[1], tree.children[0], tree.children[2])
        return res

    def hunop(self, tree):
        self.__push_up(tree)
        # The ++ and -- only shows in loop
        if tree.children[0] == '++':
            res = '{} = {} + 1'.format(tree.children[1], tree.children[1])
        elif tree.children[0] == '--':
            res = '{} = {} - 1'.format(tree.children[1], tree.children[1])
        else:
            res = '{}({})'.format(tree.children[0], tree.children[1])
        return res

    def hcstmt(self, tree):
        self.__push_up(tree)
        assert len(tree.children) == 1
        return tree.children[0]

    def get_current_ind_prefix(self):
        ind = self.current_indent * self.indent_character
        return ind

    def stmt(self, tree):
        indentation = []
        sep = []
        noindent = ['hcstmt', 'ifstmt', 'forstmt']
        nosemico = ['hcstmt', 'ifstmt', 'forstmt']
        for x in tree.children:
            if x.data in noindent:
                indentation.append('')
            else:
                indentation.append(self.get_current_ind_prefix())
            if x.data in nosemico:
                sep.append('')
            else:
                sep.append(';')

        self.__push_up(tree)
        def f_concat(x):
            try:
                res = x[0] + x[1] + x[2]
                return res
            except:
                print(x[0])
                print(x[1])
                print(x[2])
                raise
        res = '\n'.join(map(f_concat, zip(indentation, tree.children, sep)))
        return res

    def stmts(self, tree):
        self.__push_up(tree)
        res = '\n'.join(tree.children)
        return res

    def inc_indent(self):
        self.current_indent += self.indent_inc

    def dec_indent(self):
        self.current_indent -= self.indent_inc

    def push_indent(self):
        """used to service temporary indent removal, such as that in for condition"""
        self.indent_stack.append(self.current_indent)
        self.current_indent = 0
    def pop_indent(self):
        self.current_indent = self.indent_stack.pop()

    def ifstmt(self, tree):
        self.inc_indent()
        self.__push_up(tree)
        self.dec_indent()
        ind = self.get_current_ind_prefix()
        res = ind + 'if ({}) begin\n'.format(tree.children[0])
        res += tree.children[1] + '\n'
        res += ind + 'end'
        # print('If Body: ', tree.children[1])
        if len(tree.children) == 3:
            res += ' else begin\n' + tree.children[2]
            res += '\n'
            res += ind + 'end\n'
        return res

    def forinit(self, tree):
        self.__push_up(tree)
        if tree.children:
            return tree.children[0]
        else:
            return ''

    def forcond(self, tree):
        self.__push_up(tree)
        return tree.children[0]

    def forpostcond(self, tree):
        self.__push_up(tree)
        return tree.children[0]

    def forbody(self, tree):
        self.__push_up(tree)
        return tree.children[0]

    def forstmt(self, tree):
        new_children = []
        self.push_indent()
        new_children.extend(self.visit(t) for t in tree.children[:3])
        self.pop_indent()

        self.inc_indent()
        new_children.extend(self.visit(t) for t in tree.children[3:])
        self.dec_indent()
        for_init, for_cond, for_post, for_body = new_children

        ind = self.get_current_ind_prefix()
        res = ind + 'for ({};{};{}) begin\n'.format(for_init, for_cond, for_post)
        res += for_body + '\n'
        res += ind + 'end'
        return res

    def hsensvars(self, tree):
        self.__push_up(tree)
        return tree

    def npa(self, tree):
        return tree.children[0]

    def hsensedge(self, tree):
        self.__push_up(tree)
        return tree.children[0]

    def hsensvar(self, tree):
        self.__push_up(tree)
        if len(tree.children) == 1:
            return tree.children[0]
        else:
            var, edge = tree.children
            if edge:
                if edge == 'pos':
                    return 'posedge ' + var
                elif edge == 'neg':
                    return 'negedge ' + var
            return var

    def hsenslist(self, tree):
        self.__push_up(tree)
        assert len(tree.children) == 1
        sensvars = tree.children[0]
        if len(sensvars.children) > 0:
            return ' or '.join(sensvars.children)
        elif len(sensvars.children) == 0:
            return '*'

    def hprocess(self, tree):
        self.inc_indent()
        self.__push_up(tree)
        self.dec_indent()

        proc_name, senslist, prevardecl, *body = tree.children

        ind = self.get_current_ind_prefix()
        decls = list(map(lambda x: x[0] + ';', prevardecl.children))
        decls_init = list(map(lambda x: '{} = {};'.format(x[1], x[2]), filter(lambda x: len(x) == 3 and x[2] is not None, prevardecl.children)))
        res = ind + 'always @({}) begin: {}\n'.format(senslist, proc_name)
        self.inc_indent()
        ind = self.get_current_ind_prefix()
        res += ind + ('\n' + ind).join(decls) + '\n'
        res += ind + ('\n' + ind).join(decls_init) + '\n'
        self.dec_indent()
        ind = self.get_current_ind_prefix()
        res += '\n'.join(body) + '\n'
        res += ind + 'end'
        return res

    def htype(self, tree):
        self.__push_up(tree)
        name, *args = tree.children
        tpe = Primitive.get_primitive(name)
        return tpe(*args)

    def vardeclinit(self, tree):
        self.__push_up(tree)
        init_val = None
        if len(tree.children) == 2:
            var_name, tpe = tree.children
        if len(tree.children) == 3:
            var_name, tpe, init_val = tree.children
        ctx = TypeContext(suffix='')
        decl = tpe.to_str(var_name, context=ctx)
        return (decl, var_name, init_val)

    def moduleinst(self, tree):
        warnings.warn('Type parameters for modules are not supported')
        mod_name, mod_type = tree.children
        mod_type_name = mod_type.children[0].children[0]
        bindings = self.bindings[mod_name]
        ind = self.get_current_ind_prefix()
        res = ind + '{} {}('.format(mod_type_name, mod_name) + '\n'
        self.inc_indent()
        ind = self.get_current_ind_prefix()
        binding_str = []
        for binding in bindings:
            sub, par = binding.children
            binding_str.append(ind + '.{}({})'.format(sub.children[0].value, par.children[0].value))
        res += ',\n'.join(binding_str)
        res += '\n'
        self.dec_indent()
        ind = self.get_current_ind_prefix()
        res += ind + ');'
        tree.children = [res]
        return tree

    def vardecl(self, tree):
        self.__push_up(tree)
        return tree.children[0]

    def htypeinfo(self, tree):
        # Note: htypeinfo should return an object that can be used to apply to a variable
        self.__push_up(tree)
        return tree.children[0]

    def hmodule(self, tree):
        # print("Processing Module: ", tree.children[0])
        # print("Retrieving Portbindings")
        for t in tree.children:
            if isinstance(t, Tree) and t.data == 'portbindinglist':
                self.bindings[t.children[0]] = t.children[1]
        tree.children = list(filter(lambda x: not isinstance(x, Tree) or x.data != 'portbindinglist', tree.children))
        self.inc_indent()
        self.__push_up(tree)
        self.dec_indent()

        module_name = tree.children[0]
        modportsiglist = None
        processlist = None
        vars = None
        mods = []
        for t in tree.children:
            if isinstance(t, Tree):
                if t.data == 'modportsiglist':
                    modportsiglist = t
                elif t.data == 'processlist':
                    processlist = t

        # module_name, modportsiglist, processlist, portbindinglist = tree.children
        if modportsiglist:
            ports = list(filter(lambda x: isinstance(x, Tree) and x.data == 'portdecltype', modportsiglist.children))
            sigs = list(filter(lambda x: isinstance(x, Tree) and x.data == 'sigdecltype', modportsiglist.children))
            vars = list(filter(lambda x: isinstance(x, tuple), modportsiglist.children))
            mods = list(filter(lambda x: isinstance(x, Tree) and x.data == 'moduleinst', modportsiglist.children))
        else:
            ports, sigs = None, None

        res = 'module {} ('.format(module_name) + '\n'
        # Generate ports
        if ports:
            self.inc_indent()
            ind = self.get_current_ind_prefix()
            for idx, p in enumerate(ports):
                name, tpe = p.children
                name = name.children[0].value
                type_context = None
                if idx == len(ports) - 1:
                    type_context = TypeContext(suffix='')
                res += ind + tpe.to_str(name, type_context) + '\n'
            self.dec_indent()
        res += ');\n'
        # Generate signals
        if sigs:
            self.inc_indent()
            ind = self.get_current_ind_prefix()
            for idx, p in enumerate(sigs):
                name, tpe = p.children
                name = name.children[0].value
                type_context = None
                res += ind + tpe.to_str(name, type_context) + '\n'
            self.dec_indent()
        # generate vars (including modules)
        if vars:
            self.inc_indent()
            ind = self.get_current_ind_prefix()
            for decl, name, init in vars:
                # print(name, init)
                if init:
                    decl = decl + ' = ' + str(init) + ';'
                else:
                    decl += ';'
                res += ind + decl + '\n'
            self.dec_indent()
        # generate module instantiations
        if len(mods) > 0:
            for m in mods:
                res += m.children[0] + '\n'
        # Generate processes
        if processlist:
            for proc in processlist.children:
                res += proc + '\n'
        res += "endmodule"
        print(res)
        return tree
