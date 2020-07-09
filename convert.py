from parselib.grammar import lark_grammar
from parselib.utils import tidify
from parselib.transforms import VerilogTranslator
import sys
import traceback
import logging
import argparse
logging.basicConfig(level=logging.DEBUG)


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
        t = lark_grammar.parse(file_content)
    except Exception as e:
        print(e)
        exit(2)  # This code has specific meanings, should be tested in the verilog tests
    try:
        x = VerilogTranslator
        res = x.translate(t)
        # print(res)
        #  share the type information
        assert False
    except Exception as e:
        exc_type, exc_value, exc_traceback = sys.exc_info()
        print("***** print_exception:")
        # exc_type below is ignored on 3.5 and later
        traceback.print_exception(exc_type, exc_value, exc_traceback, file=sys.stdout, limit=-60)
        exit(3)
    res = tidify(res)
    with open(outputname, 'w+') as f:
        f.writelines(res)
        f.write("\n\n")
    print(res)


if __name__ == '__main__':
    main()
    # l.parse('')
