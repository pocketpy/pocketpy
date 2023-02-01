mapping = {
    'int': 'i64',
    'float': 'f64',
    'str': 'const char*',
    'bool': 'bool',
    'None': 'void'
}

def args(n, a: list, res: list, first=True):
    if n == 0:
        # 3 args must be the same type
        if len(a) == 4 and len(set(a[1:])) != 1:
            return
        res.append(tuple(a))
        return
    for p_ret, c_ret in mapping.items():
        if not first and p_ret == 'None':
            continue
        a.append(p_ret)
        args(n-1, a, res, first=False)
        a.pop()

data = []
for n in [4,3,2,1]:
    res = []
    args(n, [], res)
    for p_ret,*p_args in res:
        c_args = [mapping[i] for i in p_args]
        c_ret = mapping[p_ret]
        name = f'__f_{p_ret}__{"_".join(p_args)}'
        # if c_ret == 'const char*':
        #     c_ret = 'char*'
        s = f'typedef {c_ret} (*{name})({", ".join(c_args)});'
        s += '\n'

        impl = []
        for i, p_arg in enumerate(p_args):
            impl.append( f'{mapping[p_arg]} _{i} = vm->Py{p_arg.capitalize()}_AS_C(args[{i}]);' )
        call_impl = f'f({", ".join([f"_{j}" for j in range(len(p_args))])})';
        if p_ret == 'None':
            impl.append( call_impl + ';' )
            impl.append( 'return vm->None;' )
        else:
            impl.append ( f'{mapping[p_ret]} ret = {call_impl};' )
            impl.append( f'return vm->Py{p_ret.capitalize()}(ret);' )
        impl = '\n'.join([' '*8 + i for i in impl])
        s += f'''__EXPORT\nvoid pkpy_vm_bind{name}(VM* vm, const char* mod, const char* name, {name} f) {{
    PyVar obj = vm->new_module_if_not_existed(mod);
    vm->bindFunc<{len(p_args)}>(obj, name, [f](VM* vm, const pkpy::ArgList& args) {{
{impl}
    }});
}}''' + '\n'
        data.append(s)

with open('src/_bindings.h', 'w') as f:
    text = '\n'.join(data)
    f.write('extern "C" {\n' + text + '}')