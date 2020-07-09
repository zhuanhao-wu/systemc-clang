"""Utility library"""

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


debug = True
def p(decorated):
    """a decorator that helps printing out the transformation results"""
    if debug:
        def wrapper(self, args):
            print(f'[DBG] {decorated.__name__}: \n{args} \n\n')
            res = decorated(self, args)
            print(f'[DBG] returns: {res}\n')
            return res
        return wrapper
    else:
        return decorated


