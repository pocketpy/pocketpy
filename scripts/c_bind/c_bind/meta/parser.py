from .schema import *

class UnsupportedNode(Exception):
    def __init__(self, node: c_ast.Node):
        self.node = node

    def __str__(self):
        return f"'{type(self.node)}' is not supported\n{self.node!r}"
    

class Header:
    def __init__(self):
        self.types = [] # type: list
        self.type_aliases = {} # type: dict[str, str]
        self.functions = [] # type: list[Function]

    def remove_types(self, names: set):
        self.types = [t for t in self.types if getattr(t, 'name', None) not in names]

    def remove_functions(self, names: set):
        self.functions = [f for f in self.functions if f.name not in names]

    def build_enum(self, node: c_ast.Enum):
        enum = Enum(node.name)
        for item in node.values.enumerators:
            enum.values.append(item.name)
        self.types.append(enum)

    def unalias(self, name: str):
        while name in self.type_aliases:
            name = self.type_aliases[name]
        return name

    def build_struct(self, node):
        if isinstance(node, c_ast.Struct):
            cls = Struct
        elif isinstance(node, c_ast.Union):
            cls = Union
        else:
            raise UnsupportedNode(node)
        if node.decls is not None:
            fields = {}
            for decl in node.decls:
                try:
                    type, name = self.build_param(decl)
                    assert name
                    fields[name] = type
                except UnsupportedNode:
                    pass
            self.types.append(cls(node.name, fields))
        else:
            self.types.append(cls(node.name, None))

    def build_type(self, name, node):
        if isinstance(node, c_ast.Enum):
            self.build_enum(node)
        elif isinstance(node, (c_ast.Struct, c_ast.Union)):
            self.build_struct(node)
        elif isinstance(node, c_ast.IdentifierType):
            assert name
            self.type_aliases[name] = node.names[0]
        else:
            raise UnsupportedNode(node)
        
    def get_type_name(self, node):
        # convert array to const T*
        if isinstance(node, c_ast.ArrayDecl):
            dims = []
            while isinstance(node, c_ast.ArrayDecl):
                dims.append(node.dim.value)
                node = node.type
            base_name = self.get_type_name(node)
            # return base_name + ''.join(f'[{dim}]' for dim in dims)
            return f'const {base_name}*'

        node, level = self.eat_pointers(node)

        if isinstance(node, c_ast.FuncDecl):
            assert level == 1
            return 'void (*)()'

        if not isinstance(node, (c_ast.TypeDecl, c_ast.Typename)):
            raise UnsupportedNode(node)

        is_const = node.quals and 'const' in node.quals
        node = node.type
        base_name: str
        if isinstance(node, c_ast.IdentifierType):
            base_name = node.names[0]
        elif isinstance(node, c_ast.Enum):
            base_name = 'enum ' + node.name
        elif isinstance(node, c_ast.Struct):
            base_name = 'struct ' + node.name
        elif isinstance(node, c_ast.Union):
            base_name = 'union ' + node.name
        else:
            base_name = self.get_type_name(node)

        if is_const:
            base_name = 'const ' + base_name
        return base_name + '*' * level
        
    def build_param(self, node):
        name = None
        if isinstance(node, c_ast.Decl):
            name = node.name
            node = node.type
        return self.get_type_name(node), name
    
    def eat_pointers(self, node):
        level = 0
        while isinstance(node, c_ast.PtrDecl):
            node = node.type
            level += 1
        return node, level
        
    def build_function(self, node: c_ast.FuncDecl):
        args = node.args
        node = node.type
        node, level = self.eat_pointers(node)
        assert isinstance(node, c_ast.TypeDecl), type(node)
        name = node.declname
        ret = node.type.names[0]
        func = Function(name, ret + '*' * level)
        if args is not None:
            for param in args.params:
                if isinstance(param, c_ast.EllipsisParam):
                    func.args.append('...')
                else:
                    T, name = self.build_param(param)
                    if T != 'void':
                        func.args.append(T)
        self.functions.append(func)

    def build(self, ast: c_ast.FileAST):
        for _, node in ast.children():
            if isinstance(node, c_ast.Typedef):
                name, node = node.name, node.type
                if isinstance(node, c_ast.TypeDecl):
                    self.build_type(name, node.type)
                elif isinstance(node, c_ast.PtrDecl):
                    type_name = self.get_type_name(node)
                    self.type_aliases[name] = type_name
                else:
                    # raise UnsupportedNode(node.type)
                    # print(f"Unsupported typedef: {type(node)}")
                    continue
            elif isinstance(node, c_ast.Decl):
                if isinstance(node.type, c_ast.FuncDecl):
                    self.build_function(node.type)
                else:
                    try:
                        self.build_type(None, node.type)
                    except UnsupportedNode:
                        pass
            else:
                # raise UnsupportedNode(node.type)
                # print(f"Unsupported typedef: {type(node)}")
                continue