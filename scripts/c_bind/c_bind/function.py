from .writer import Writer
from .converters import get_converter
from .schema import Function


def gen_function(w: Writer, pyi_w: Writer, function: Function):
    name = function.name
    w.write(f'static bool cfunc__{name}(int argc, py_Ref argv) {{')
    w.indent()

    w.write(f'PY_CHECK_ARGC({len(function.params)});')

    args_cvt = [get_converter(arg.type) for arg in function.params]
    ret_cvt = get_converter(function.ret_type)

    for i in range(len(args_cvt)):
        w.write(f'{args_cvt[i].T} _{i};')
        args_cvt[i].py2c(w, f'_{i}', f'py_arg({i})')

    call_args = ', '.join([f'_{i}' for i in range(len(args_cvt))])

    # gen retval
    if function.ret_type == 'void':
        w.write(f'{name}({call_args});')
        w.write(f'py_newnone(py_retval());')
    else:
        w.write(f'{ret_cvt.T} res = {name}({call_args});')
        ret_cvt.c2py(w, 'py_retval()', 'res')

    w.write('return true;')

    w.dedent()
    w.write('}')

    # pyi
    py_args = []
    # arg_names = [f'_{i}' for i in range(len(args_cvt))]
    arg_names = [arg.name for arg in function.params]
    for i in range(len(args_cvt)):
        py_args.append(f'{arg_names[i]}: {args_cvt[i].py_T}')

    pyi_w.write(f'def {name}({", ".join(py_args)}) -> {ret_cvt.py_T}:')
    if function.desc:
        pyi_w.write(f'    """Wraps `{function.signature()}`\n\n    {function.desc}"""')
    else:
        pyi_w.write(f'    """Wraps `{function.signature()}`"""')
    pyi_w.write('')