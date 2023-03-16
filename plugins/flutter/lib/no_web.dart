// ignore_for_file: non_constant_identifier_names, prefer_typing_uninitialized_variables, constant_identifier_names, no_leading_underscores_for_local_identifiers

import 'dart:convert' as cvt;
import 'dart:ffi' as ffi;
import 'dart:io';
import 'package:ffi/ffi.dart';
import '_ffi.dart';
import 'common.dart';

class _Bindings {
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

  static final pkpy_delete = _lib.lookupFunction<
      ffi.Void Function(ffi.Pointer p),
      void Function(ffi.Pointer p)>("pkpy_delete");
  static final pkpy_setup_callbacks = _lib.lookupFunction<
      ffi.Void Function(ffi.Pointer _f_int, ffi.Pointer _f_float,
          ffi.Pointer _f_bool, ffi.Pointer _f_str, ffi.Pointer _f_None),
      void Function(
          ffi.Pointer _f_int,
          ffi.Pointer _f_float,
          ffi.Pointer _f_bool,
          ffi.Pointer _f_str,
          ffi.Pointer _f_None)>("pkpy_setup_callbacks");
  static final pkpy_new_repl = _lib.lookupFunction<
      ffi.Pointer Function(ffi.Pointer vm),
      ffi.Pointer Function(ffi.Pointer vm)>("pkpy_new_repl");
  static final pkpy_repl_input = _lib.lookupFunction<
      ffi.Bool Function(ffi.Pointer r, ffi.Pointer<Utf8> line),
      bool Function(ffi.Pointer r, ffi.Pointer<Utf8> line)>("pkpy_repl_input");
  static final pkpy_new_vm = _lib.lookupFunction<
      ffi.Pointer Function(ffi.Bool use_stdio),
      ffi.Pointer Function(bool use_stdio)>("pkpy_new_vm");
  static final pkpy_vm_add_module = _lib.lookupFunction<
      ffi.Void Function(
          ffi.Pointer vm, ffi.Pointer<Utf8> name, ffi.Pointer<Utf8> source),
      void Function(ffi.Pointer vm, ffi.Pointer<Utf8> name,
          ffi.Pointer<Utf8> source)>("pkpy_vm_add_module");
  static final pkpy_vm_bind = _lib.lookupFunction<
      ffi.Pointer<Utf8> Function(ffi.Pointer vm, ffi.Pointer<Utf8> mod,
          ffi.Pointer<Utf8> name, ffi.Int32 ret_code),
      ffi.Pointer<Utf8> Function(ffi.Pointer vm, ffi.Pointer<Utf8> mod,
          ffi.Pointer<Utf8> name, int ret_code)>("pkpy_vm_bind");
  static final pkpy_vm_eval = _lib.lookupFunction<
      ffi.Pointer<Utf8> Function(ffi.Pointer vm, ffi.Pointer<Utf8> source),
      ffi.Pointer<Utf8> Function(
          ffi.Pointer vm, ffi.Pointer<Utf8> source)>("pkpy_vm_eval");
  static final pkpy_vm_exec = _lib.lookupFunction<
      ffi.Void Function(ffi.Pointer vm, ffi.Pointer<Utf8> source),
      void Function(ffi.Pointer vm, ffi.Pointer<Utf8> source)>("pkpy_vm_exec");
  static final pkpy_vm_get_global = _lib.lookupFunction<
      ffi.Pointer<Utf8> Function(ffi.Pointer vm, ffi.Pointer<Utf8> name),
      ffi.Pointer<Utf8> Function(
          ffi.Pointer vm, ffi.Pointer<Utf8> name)>("pkpy_vm_get_global");
  static final pkpy_vm_read_output = _lib.lookupFunction<
      ffi.Pointer<Utf8> Function(ffi.Pointer vm),
      ffi.Pointer<Utf8> Function(ffi.Pointer vm)>("pkpy_vm_read_output");
}

class VM {
  late final pointer = _Bindings.pkpy_new_vm(false);
  static bool _firstNew = true;

  VM() {
    if (!_firstNew) return;
    _firstNew = false;
    _Bindings.pkpy_setup_callbacks(
        f_int(), f_float(), f_bool(), f_str(), f_None());
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
  void add_module(String name, String source) {
    _Bindings.pkpy_vm_add_module(
        pointer, StrWrapper(name).p, StrWrapper(source).p);
  }

  /// Evaluate an expression.  Return `__repr__` of the result. If there is any error, return `nullptr`.
  String? eval(String source) {
    var ret = _Bindings.pkpy_vm_eval(pointer, StrWrapper(source).p);
    if (ret == ffi.nullptr) return null;
    String s = ret.toDartString();
    _Bindings.pkpy_delete(ret);
    return s;
  }

  /// Run a given source on a virtual machine.
  void exec(String source) {
    _Bindings.pkpy_vm_exec(pointer, StrWrapper(source).p);
  }

  /// Get a global variable of a virtual machine.  Return `__repr__` of the result. If the variable is not found, return `nullptr`.
  String? get_global(String name) {
    var ret = _Bindings.pkpy_vm_get_global(pointer, StrWrapper(name).p);
    if (ret == ffi.nullptr) return null;
    String s = ret.toDartString();
    _Bindings.pkpy_delete(ret);
    return s;
  }

  void bind<T>(String mod, String name, Function f) {
    ffi.Pointer<Utf8> p = _Bindings.pkpy_vm_bind(
        pointer, StrWrapper(mod).p, StrWrapper(name).p, t_code<T>());
    if (p == ffi.nullptr) throw Exception("vm.bind() failed");
    String s = p.toDartString();
    _Bindings.pkpy_delete(p);
    register(s, f);
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
    var ret = _Bindings.pkpy_repl_input(pointer, StrWrapper(line).p);
    return ret;
  }
}
