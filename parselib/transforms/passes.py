"""Different pass of the translation"""
from .top_down import TopDown
from .node import TypeDefType
from .typedef_expansion import TypedefExpansion
from .verilog_tranlation import VerilogTranslationPass
from .port_expansion import PortExpansion
from lark import Tree, Token
import copy
import warnings


class LiteralExpansion(TopDown):
    """Expands integer literal into int"""
    def __init__(self):
        super().__init__()

    def idlit(self, tree):
        str_literal = tree.children[0]
        return str_literal

    def numlit(self, tree):
        num_literal = int(tree.children[0])
        return num_literal

    def htypeint(self, tree):
        return int(tree.children[0])

    def htype(self, tree):
        self.__push_up(tree)
        if len(tree.children) == 1 and isinstance(tree.children[0], int):
            return tree.children[0]
        else:
            return tree


class TypeDefFilter(TopDown):
    """This pass recognizes the custom typedef type and remove it from the grammar tree"""
    def __init__(self):
        super().__init__()
        self.types = dict()
        self.current_params = None

    def htypedef(self, tree):
        type_name = tree.children[0]
        type_params = list(map(lambda x: x.children[0], tree.find_data('htypetemplateparam')))

        self.current_params = type_params
        self.__push_up(tree)
        self.current_params = None

        type_fields = list(tree.find_data('htypefield'))
        t = TypeDefType(type_name, type_params, type_fields)
        self.types[type_name] = t
        return tree

    def htype(self, tree):
        # extract type parameters
        if len(tree.children) == 1 and self.current_params and tree.children[0] in self.current_params:
            return tree.children[0]
        else:
            self.__push_up(tree)
            return tree

    def start(self, tree):
        tree.children = self.visit_children(tree)
        # remove typedefs from the syntax tree
        tree.children = list(filter(lambda x: x.data != 'typelist', tree.children))
        return tree


class NodeMergePass(TopDown):
    """This pass merges separate nodes that are created for easy recognition of grammar,
    but actually shares the same semantics
    """
    def hnsbinop(self, tree):
        self.__push_up(tree)
        tree.data = 'hbinop'
        return tree

class VerilogTranslator:
    """Translate xlat to verilog"""
    @staticmethod
    def translate(tree):
        prev = tree
        prev = LiteralExpansion().visit(prev)
        f = TypeDefFilter()
        prev = f.visit(prev)
        prev = NodeMergePass().visit(prev)
        prev = PortExpansion().visit(prev)
        # note typedef should be after port expansion to prevent duplicate valid/ready
        prev = TypedefExpansion(f.types).visit(prev)
        prev = VerilogTranslationPass().visit(prev)
        return prev

