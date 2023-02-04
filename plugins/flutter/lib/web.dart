// ignore_for_file: prefer_function_declarations_over_variables, non_constant_identifier_names, no_leading_underscores_for_local_identifiers
import 'dart:convert' as cvt;
import 'package:js/js.dart';
import 'common.dart';

@JS("Module.ccall")
external dynamic ccall(
    String name, String? returnType, List<String> argTypes, List<dynamic> args);

class _Bindings {
  static final pkpy_delete =
      (dynamic p) => ccall("pkpy_delete", null, ["number"], [p]);
  static final pkpy_new_repl =
      (dynamic vm) => ccall("pkpy_new_repl", "number", ["number"], [vm]);
  static final pkpy_repl_input = (dynamic r, String line) =>
      ccall("pkpy_repl_input", "boolean", ["number", "string"], [r, line]);
  static final pkpy_new_vm = (bool use_stdio) =>
      ccall("pkpy_new_vm", "number", ["boolean"], [use_stdio]);
  static final pkpy_vm_add_module = (dynamic vm, String name, String source) =>
      ccall("pkpy_vm_add_module", null, ["number", "string", "string"],
          [vm, name, source]);
  static final pkpy_vm_eval = (dynamic vm, String source) =>
      ccall("pkpy_vm_eval", "string", ["number", "string"], [vm, source]);
  static final pkpy_vm_exec = (dynamic vm, String source) =>
      ccall("pkpy_vm_exec", null, ["number", "string"], [vm, source]);
  static final pkpy_vm_get_global = (dynamic vm, String name) =>
      ccall("pkpy_vm_get_global", "string", ["number", "string"], [vm, name]);
  static final pkpy_vm_read_output =
      (dynamic vm) => ccall("pkpy_vm_read_output", "string", ["number"], [vm]);
}

class VM {
  late final pointer = _Bindings.pkpy_new_vm(false);

  void dispose() {
    _Bindings.pkpy_delete(pointer);
  }

  PyOutput read_output() {
    var _o = _Bindings.pkpy_vm_read_output(pointer);
    String _j = _o;
    var ret = PyOutput.fromJson(cvt.jsonDecode(_j));

    return ret;
  }

  /// Add a source module into a virtual machine.
  void add_module(String name, String source) {
    _Bindings.pkpy_vm_add_module(pointer, name, source);
  }

  /// Evaluate an expression.  Return `__repr__` of the result. If there is any error, return `nullptr`.
  String? eval(String source) {
    var ret = _Bindings.pkpy_vm_eval(pointer, source);
    return ret;
  }

  /// Run a given source on a virtual machine.
  void exec(String source) {
    _Bindings.pkpy_vm_exec(pointer, source);
  }

  /// Get a global variable of a virtual machine.  Return `__repr__` of the result. If the variable is not found, return `nullptr`.
  String? get_global(String name) {
    var ret = _Bindings.pkpy_vm_get_global(pointer, name);
    return ret;
  }
}

class REPL {
  late final dynamic pointer;

  REPL(VM vm) {
    pointer = _Bindings.pkpy_new_repl(vm.pointer);
  }

  void dispose() {
    _Bindings.pkpy_delete(pointer);
  }

  /// Input a source line to an interactive console. Return true if need more lines.
  bool input(String line) {
    var ret = _Bindings.pkpy_repl_input(pointer, line);
    return ret;
  }
}
