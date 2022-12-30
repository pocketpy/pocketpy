// ignore_for_file: non_constant_identifier_names, prefer_typing_uninitialized_variables, constant_identifier_names, no_leading_underscores_for_local_identifiers

import 'dart:convert' as cvt;
import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:ffi/ffi.dart';
import 'common.dart';

class _Bindings
{
  static ffi.DynamicLibrary _load() {
    String _libName = "pocketpy";
    if (Platform.isIOS) {
      return ffi.DynamicLibrary.process();
    }
    if (Platform.isAndroid || Platform.isLinux) {
      return ffi.DynamicLibrary.open('lib$_libName.so');
    }
    if (Platform.isWindows) {
      return ffi.DynamicLibrary.open('$_libName.dll');
    }
    throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
  }

  static final _lib = _load();

  static final pkpy_delete = _lib.lookupFunction<ffi.Void Function(ffi.Pointer p), void Function(ffi.Pointer p)>("pkpy_delete");
  static final pkpy_new_repl = _lib.lookupFunction<ffi.Pointer Function(ffi.Pointer vm), ffi.Pointer Function(ffi.Pointer vm)>("pkpy_new_repl");
  static final pkpy_repl_input = _lib.lookupFunction<ffi.Int32 Function(ffi.Pointer r, ffi.Pointer<Utf8> line), int Function(ffi.Pointer r, ffi.Pointer<Utf8> line)>("pkpy_repl_input");
  static final pkpy_new_tvm = _lib.lookupFunction<ffi.Pointer Function(ffi.Bool use_stdio), ffi.Pointer Function(bool use_stdio)>("pkpy_new_tvm");
  static final pkpy_tvm_exec_async = _lib.lookupFunction<ffi.Void Function(ffi.Pointer vm, ffi.Pointer<Utf8> source), void Function(ffi.Pointer vm, ffi.Pointer<Utf8> source)>("pkpy_tvm_exec_async");
  static final pkpy_tvm_get_state = _lib.lookupFunction<ffi.Int32 Function(ffi.Pointer vm), int Function(ffi.Pointer vm)>("pkpy_tvm_get_state");
  static final pkpy_tvm_read_jsonrpc_request = _lib.lookupFunction<ffi.Pointer<Utf8> Function(ffi.Pointer vm), ffi.Pointer<Utf8> Function(ffi.Pointer vm)>("pkpy_tvm_read_jsonrpc_request");
  static final pkpy_tvm_reset_state = _lib.lookupFunction<ffi.Void Function(ffi.Pointer vm), void Function(ffi.Pointer vm)>("pkpy_tvm_reset_state");
  static final pkpy_tvm_terminate = _lib.lookupFunction<ffi.Void Function(ffi.Pointer vm), void Function(ffi.Pointer vm)>("pkpy_tvm_terminate");
  static final pkpy_tvm_write_jsonrpc_response = _lib.lookupFunction<ffi.Void Function(ffi.Pointer vm, ffi.Pointer<Utf8> value), void Function(ffi.Pointer vm, ffi.Pointer<Utf8> value)>("pkpy_tvm_write_jsonrpc_response");
  static final pkpy_new_vm = _lib.lookupFunction<ffi.Pointer Function(ffi.Bool use_stdio), ffi.Pointer Function(bool use_stdio)>("pkpy_new_vm");
  static final pkpy_vm_add_module = _lib.lookupFunction<ffi.Void Function(ffi.Pointer vm, ffi.Pointer<Utf8> name, ffi.Pointer<Utf8> source), void Function(ffi.Pointer vm, ffi.Pointer<Utf8> name, ffi.Pointer<Utf8> source)>("pkpy_vm_add_module");
  static final pkpy_vm_eval = _lib.lookupFunction<ffi.Pointer<Utf8> Function(ffi.Pointer vm, ffi.Pointer<Utf8> source), ffi.Pointer<Utf8> Function(ffi.Pointer vm, ffi.Pointer<Utf8> source)>("pkpy_vm_eval");
  static final pkpy_vm_exec = _lib.lookupFunction<ffi.Void Function(ffi.Pointer vm, ffi.Pointer<Utf8> source), void Function(ffi.Pointer vm, ffi.Pointer<Utf8> source)>("pkpy_vm_exec");
  static final pkpy_vm_get_global = _lib.lookupFunction<ffi.Pointer<Utf8> Function(ffi.Pointer vm, ffi.Pointer<Utf8> name), ffi.Pointer<Utf8> Function(ffi.Pointer vm, ffi.Pointer<Utf8> name)>("pkpy_vm_get_global");
  static final pkpy_vm_read_output = _lib.lookupFunction<ffi.Pointer<Utf8> Function(ffi.Pointer vm), ffi.Pointer<Utf8> Function(ffi.Pointer vm)>("pkpy_vm_read_output");
}

class _Str {
  static final Finalizer<ffi.Pointer<Utf8>> finalizer = Finalizer((p) => malloc.free(p));

  late final ffi.Pointer<Utf8> _p;
  _Str(String s) {
    _p = s.toNativeUtf8();
    finalizer.attach(this, _p);
  }

  ffi.Pointer<Utf8> get p => _p;
}
class VM {
  late final ffi.Pointer pointer;

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
    String _j = _o.toDartString();
    var ret = PyOutput.fromJson(cvt.jsonDecode(_j));
    _Bindings.pkpy_delete(_o);
    return ret;
  }

  /// Add a source module into a virtual machine.
  void add_module(String name, String source)
  {
    _Bindings.pkpy_vm_add_module(pointer, _Str(name).p, _Str(source).p);
  }

  /// Evaluate an expression.  Return a json representing the result. If there is any error, return `nullptr`.
  String? eval(String source)
  {
    var ret = _Bindings.pkpy_vm_eval(pointer, _Str(source).p);
    if (ret == ffi.nullptr) return null;
    String s = ret.toDartString();
    _Bindings.pkpy_delete(ret);
    return s;
  }

  /// Run a given source on a virtual machine.
  void exec(String source)
  {
    _Bindings.pkpy_vm_exec(pointer, _Str(source).p);
  }

  /// Get a global variable of a virtual machine.  Return a json representing the result. If the variable is not found, return `nullptr`.
  String? get_global(String name)
  {
    var ret = _Bindings.pkpy_vm_get_global(pointer, _Str(name).p);
    if (ret == ffi.nullptr) return null;
    String s = ret.toDartString();
    _Bindings.pkpy_delete(ret);
    return s;
  }

}

enum ThreadState { ready, running, suspended, finished }

class ThreadedVM extends VM {
  ThreadState get state => ThreadState.values[_Bindings.pkpy_tvm_get_state(pointer)];
  
  /// Run a given source on a threaded virtual machine. The excution will be started in a new thread.
  void exec_async(String source)
  {
    _Bindings.pkpy_tvm_exec_async(pointer, _Str(source).p);
  }

  /// Read the current JSONRPC request from shared string buffer.
  String? read_jsonrpc_request()
  {
    var ret = _Bindings.pkpy_tvm_read_jsonrpc_request(pointer);
    if (ret == ffi.nullptr) return null;
    String s = ret.toDartString();
    _Bindings.pkpy_delete(ret);
    return s;
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
    _Bindings.pkpy_tvm_write_jsonrpc_response(pointer, _Str(value).p);
  }

}

class REPL {
  late final ffi.Pointer pointer;

  REPL(VM vm) {
    pointer = _Bindings.pkpy_new_repl(vm.pointer);
  }

  void dispose() {
    _Bindings.pkpy_delete(pointer);
  }

  /// Input a source line to an interactive console.  Return `0` if need more lines, `1` if execution happened, `2` if execution skipped (compile error or empty input).
  int input(String line)
  {
    var ret = _Bindings.pkpy_repl_input(pointer, _Str(line).p);
    return ret;
  }

}

