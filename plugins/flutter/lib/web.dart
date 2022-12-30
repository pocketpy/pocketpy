// ignore_for_file: prefer_function_declarations_over_variables, non_constant_identifier_names, no_leading_underscores_for_local_identifiers
import 'dart:convert' as cvt;
import 'package:js/js.dart';
import 'common.dart';

@JS("Module.ccall")
external dynamic ccall(String name, String? returnType, List<String> argTypes, List<dynamic> args);

class _Bindings
{
  static final pkpy_delete = (dynamic p) => ccall("pkpy_delete", null, ["number"], [p]);
  static final pkpy_new_repl = (dynamic vm) => ccall("pkpy_new_repl", "number", ["number"], [vm]);
  static final pkpy_repl_input = (dynamic r, String line) => ccall("pkpy_repl_input", null, ["number", "string"], [r, line]);
  static final pkpy_repl_last_input_result = (dynamic r) => ccall("pkpy_repl_last_input_result", "number", ["number"], [r]);
  static final pkpy_new_tvm = (bool use_stdio) => ccall("pkpy_new_tvm", "number", ["boolean"], [use_stdio]);
  static final pkpy_tvm_exec_async = (dynamic vm, String source) => ccall("pkpy_tvm_exec_async", null, ["number", "string"], [vm, source]);
  static final pkpy_tvm_get_state = (dynamic vm) => ccall("pkpy_tvm_get_state", "number", ["number"], [vm]);
  static final pkpy_tvm_read_jsonrpc_request = (dynamic vm) => ccall("pkpy_tvm_read_jsonrpc_request", "string", ["number"], [vm]);
  static final pkpy_tvm_reset_state = (dynamic vm) => ccall("pkpy_tvm_reset_state", null, ["number"], [vm]);
  static final pkpy_tvm_terminate = (dynamic vm) => ccall("pkpy_tvm_terminate", null, ["number"], [vm]);
  static final pkpy_tvm_write_jsonrpc_response = (dynamic vm, String value) => ccall("pkpy_tvm_write_jsonrpc_response", null, ["number", "string"], [vm, value]);
  static final pkpy_new_vm = (bool use_stdio) => ccall("pkpy_new_vm", "number", ["boolean"], [use_stdio]);
  static final pkpy_vm_add_module = (dynamic vm, String name, String source) => ccall("pkpy_vm_add_module", null, ["number", "string", "string"], [vm, name, source]);
  static final pkpy_vm_eval = (dynamic vm, String source) => ccall("pkpy_vm_eval", "string", ["number", "string"], [vm, source]);
  static final pkpy_vm_exec = (dynamic vm, String source) => ccall("pkpy_vm_exec", null, ["number", "string"], [vm, source]);
  static final pkpy_vm_get_global = (dynamic vm, String name) => ccall("pkpy_vm_get_global", "string", ["number", "string"], [vm, name]);
  static final pkpy_vm_read_output = (dynamic vm) => ccall("pkpy_vm_read_output", "string", ["number"], [vm]);
}

class VM {
  late final dynamic pointer;

  VM() {
    if (this is ThreadedVM) {
      pointer = _Bindings.pkpy_new_tvm(false);
    } else {
      pointer = _Bindings.pkpy_new_vm(false);
    }
  }

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
  void add_module(String name, String source)
  {
    _Bindings.pkpy_vm_add_module(pointer, name, source);
  }

  /// Evaluate an expression.  Return a json representing the result. If there is any error, return `nullptr`.
  String? eval(String source)
  {
    var ret = _Bindings.pkpy_vm_eval(pointer, source);
    return ret;
  }

  /// Run a given source on a virtual machine.
  void exec(String source)
  {
    _Bindings.pkpy_vm_exec(pointer, source);
  }

  /// Get a global variable of a virtual machine.  Return a json representing the result. If the variable is not found, return `nullptr`.
  String? get_global(String name)
  {
    var ret = _Bindings.pkpy_vm_get_global(pointer, name);
    return ret;
  }

}

enum ThreadState { ready, running, suspended, finished }

class ThreadedVM extends VM {
  ThreadState get state => ThreadState.values[_Bindings.pkpy_tvm_get_state(pointer)];
  
  /// Run a given source on a threaded virtual machine. The excution will be started in a new thread.
  void exec_async(String source)
  {
    _Bindings.pkpy_tvm_exec_async(pointer, source);
  }

  /// Read the current JSONRPC request from shared string buffer.
  String? read_jsonrpc_request()
  {
    var ret = _Bindings.pkpy_tvm_read_jsonrpc_request(pointer);
    return ret;
  }

  /// Set the state of a threaded virtual machine to `THREAD_READY`. The current state should be `THREAD_FINISHED`.
  void reset_state()
  {
    _Bindings.pkpy_tvm_reset_state(pointer);
  }

  /// Emit a KeyboardInterrupt signal to stop a running threaded virtual machine. 
  void terminate()
  {
    _Bindings.pkpy_tvm_terminate(pointer);
  }

  /// Write a JSONRPC response to shared string buffer.
  void write_jsonrpc_response(String value)
  {
    _Bindings.pkpy_tvm_write_jsonrpc_response(pointer, value);
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

  /// Input a source line to an interactive console.
  void input(String line)
  {
    _Bindings.pkpy_repl_input(pointer, line);
  }

  /// Check if the REPL needs more lines.
  int last_input_result()
  {
    var ret = _Bindings.pkpy_repl_last_input_result(pointer);
    return ret;
  }

}

