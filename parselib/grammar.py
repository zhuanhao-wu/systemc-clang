from lark import Lark

lark_grammar = Lark('''
        start: modulelist typelist

        modulelist: (hmodule)*
        typelist: (htypedef)*
        hmodule:  "hModule" ID "[" modportsiglist? (portbindinglist|processlist)* "]"

        modportsiglist: "hPortsigvarlist" "NONAME" "[" modportsigdecl+ "]" 

        ?modportsigdecl: portdecltype 
                      | sigdecltype 
                      | vardeclinit 
                      | moddecl
        moddecl: "hModdecl" ID "[" htypeinfo "]"
        portdecltype: portdecl "[" htypeinfo "]"
        sigdecltype: sigdecl "[" htypeinfo "]"
        sigdecl:  "hSigdecl" ID  
        ?portdecl: inportdecl | outportdecl
        inportdecl:  "hPortin" ID 
        outportdecl: "hPortout" ID
        vardeclinit: "hVardecl" ID "[" htypeinfo (hvarinit | hvarinitint)? "]"
        ?hvarinit: "hVarInit" "NONAME" expression
        ?hvarinitint: "hVarInit" NUM "NOLIST"
        // can be no process at all in the module
        ?processlist:  "hProcesses" "NONAME" "[" hprocess* "]"
        // could be nothing
        // temporarily ignore the hMethod node
        hprocess:  "hProcess" ID  "[" hsenslist*   "hMethod" "NONAME" "[" prevardecl hcstmt "]" "]"
        prevardecl: vardecl*
        vardecl: vardeclinit

        // can be just an empty statement
        hcstmt:  "hCStmt" "NONAME" "[" modportsiglist* stmts "]" // useful for raising variable decls
              |  "hCStmt" "NONAME" "NOLIST"
        stmts: stmt+
        stmt : expression_in_stmt
             | syscwrite
             | ifstmt
             | forstmt
             | hcstmt
             | switchstmt
             | blkassign

        // Port Bindings
        portbindinglist: "hPortbindings" ID "[" portbinding* "]"
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
                 |  "hSensvar" ID "NOLIST"
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

        // Separate '=' out from so that it is not an expression but a standalone statement
        blkassign: "hBinop" "=" "[" (hvarref | hliteral) (hvarref | hliteral | harrayref | hnsbinop | hunimp | syscread) "]"
                 | "hBinop" "=" "[" harrayref (hvarref | hliteral | harrayref | hnsbinop | hunimp | syscread) "]"
                 | "hSigAssignL" "write" "[" hliteral (syscread | hliteral)  "]"
        harrayref: "hBinop" "ARRAYSUBSCRIPT" "[" hliteral expression  "]"
        hnsbinop:  "hBinop" NONSUBBINOP "[" expression expression "]"

        hmethodcall: "hMethodCall" hidorstr  "[" expression expression* "]" 
        ?hidorstr: ID | STRING
        hliteral:  idlit | numlit
        idlit : "hLiteral" ID "NOLIST"
        numlit : "hLiteral" NUM "NOLIST"
        htypeinfo: "hTypeinfo" "NONAME" "[" htype "]"
                 | "hTypeinfo" "NONAME" "NOLIST" // ?
                 | hscbv
        hscbv    : "hTypeinfo" "NONAME" "[" "hType" "sc_bv" "NOLIST"  "hType" NUM "NOLIST" "]" // TODO: for legacy format, to be removed
        htype: "hType" TYPESTR "NOLIST" 
             | "hType" TYPESTR "[" (htype|htypeint)+ "]"     // nested types, type parameters
        htypeint: "hLiteral" NUM "NOLIST"  // integer type parameters
        htypedef: "hTypedef" TYPESTR "[" htypetemplateparams htypefields "]"
        
        
        htypetemplateparams: htypetemplateparam*
        htypetemplateparam: "hTypeTemplateParam" TYPESTR "NOLIST"
        
        htypefields: htypefield*
        htypefield: "hTypeField" ID "[" htype "]"

        ID: /[a-zA-Z_][a-zA-Z_0-9]*/
        NUM: /(\+|\-)?[0-9]+/
        TYPESTR: /[a-zA-Z_][a-zA-Z_0-9]*/
        BINOP: NONSUBBINOP | "ARRAYSUBSCRIPT"
        NONSUBBINOP: "==" | "&&" | "||" | "-" | ">" | "+" | "*" | "^" | "<=" | "<" | "%" | "!=" | "&"
        UNOP: "!" | "++" | "-"
        %import common.WS
        %ignore WS
        %import common.ESCAPED_STRING -> STRING
        ''', parser='lalr', debug=True)

