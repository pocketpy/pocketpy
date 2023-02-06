rm -rf web/lib/
mkdir -p web/lib/
em++ src/main.cpp -fno-rtti -fexceptions -O3 -sEXPORTED_FUNCTIONS=_pkpy_delete,_pkpy_setup_callbacks,_pkpy_new_repl,_pkpy_repl_input,_pkpy_new_vm,_pkpy_vm_add_module,_pkpy_vm_bind,_pkpy_vm_eval,_pkpy_vm_exec,_pkpy_vm_get_global,_pkpy_vm_read_output -sEXPORTED_RUNTIME_METHODS=ccall -o web/lib/pocketpy.js