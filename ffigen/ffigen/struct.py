from .writer import Writer
from .converters import get_converter, Converter
from .schema import Struct, StructField

def gen_getter(w: Writer, struct: Struct, cvt: Converter, field: StructField):
    w.write(f'static bool {struct.identifier}__get_{field.name}(int argc, py_Ref argv) {{')
    w.indent()
    w.write(f'PY_CHECK_ARGC(1);')
    w.write(f'{struct.code_name}* self = py_touserdata(argv);')
    cvt.c2py(w, 'py_retval()', f'self->{field.name}')
    w.write('return true;')
    w.dedent()
    w.write('}')

def gen_setter(w: Writer, struct: Struct, cvt: Converter, field: StructField):
    w.write(f'static bool {struct.identifier}__set_{field.name}(int argc, py_Ref argv) {{')
    w.indent()
    w.write(f'PY_CHECK_ARGC(2);')
    w.write(f'{struct.code_name}* self = py_touserdata(argv);')
    cvt.py2c(w, f'self->{field.name}', 'py_arg(1)')
    w.write('py_newnone(py_retval());')
    w.write('return true;')
    w.dedent()
    w.write('}')

def gen_struct(w: Writer, pyi_w: Writer, struct: Struct):
    name = struct.code_name
    identifier = struct.identifier
    converters = [get_converter(field.type) for field in struct.fields or []]
    # default __new__
    w.write(f'static bool {identifier}__new__(int argc, py_Ref argv) {{')
    w.indent()
    w.write(f'py_Type cls = py_totype(argv);')
    w.write(f'py_newobject(py_retval(), cls, 0, sizeof({name}));')
    w.write('return true;')
    w.dedent()
    w.write('}')

    # default __init__
    w.write(f'static bool {identifier}__init__(int argc, py_Ref argv) {{')
    w.indent()
    w.write(f'{name}* self = py_touserdata(argv);')
    w.write(f'if(argc == 1) {{')
    w.indent()
    w.write(f'memset(self, 0, sizeof({name}));')
    w.dedent()
    w.write(f'}} else if(argc == 1 + {len(struct.fields)}) {{')
    w.indent()
    for i, field in enumerate(struct.fields):
        cvt = converters[i]
        if not cvt.is_const():
            cvt.py2c(w, f'self->{field.name}', f'py_arg({i+1})')
        else:
            w.write(f'// _{i} {field.name} is read-only')
    w.dedent()
    w.write('} else {')
    w.indent()
    w.write(f'return TypeError("expected 1 or {len(struct.fields)+1} arguments");')
    w.dedent()
    w.write('}')
    w.write('py_newnone(py_retval());')
    w.write('return true;')
    w.dedent()
    w.write('}')

    for i, field in enumerate(struct.fields):
        cvt = converters[i]
        gen_getter(w, struct, cvt, field)
        if not cvt.is_const():
            gen_setter(w, struct, cvt, field)

    w.write(f'static py_Type register__{identifier}(py_GlobalRef mod) {{')
    w.indent()
    w.write(f'py_Type type = py_newtype("{identifier}", tp_stdc_Memory, mod, NULL);')
    w.write(f'py_newint(py_emplacedict(py_tpobject(type), py_name("size")), sizeof({name}));')

    w.write(f'py_bindmethod(type, "__new__", {identifier}__new__);')
    w.write(f'py_bindmethod(type, "__init__", {identifier}__init__);')

    for i, field in enumerate(struct.fields):
        cvt = converters[i]
        if cvt.is_const():
            setter = 'NULL'
        else:
            setter = f'{identifier}__set_{field.name}'
        w.write(f'py_bindproperty(type, "{field.name}", {identifier}__get_{field.name}, {setter});')

    w.write(f'return type;')
    w.dedent()
    w.write('}')

    # pyi
    pyi_w.write(f'class {identifier}:')
    pyi_w.indent()

    py_args = []
    for i, field in enumerate(struct.fields):
        cvt = converters[i]
        desc = (field.desc or '') + f' ({field.type})'
        py_args.append(f"{field.name}: {cvt.py_T}")
        pyi_w.write(f"{py_args[-1]} # {desc}")
    pyi_w.write('')

    pyi_w.write(f'@overload')
    pyi_w.write(f'def __init__(self): ...')
    pyi_w.write(f'@overload')
    pyi_w.write(f'def __init__(self, {", ".join(py_args)}): ...')
    pyi_w.write('')
    pyi_w.dedent()

    w.write(f'static py_Type tp_user_{identifier};')
    return f'tp_user_{identifier} = register__{identifier}(mod);'