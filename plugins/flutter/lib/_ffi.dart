// ignore_for_file: non_constant_identifier_names

import 'dart:convert';
import 'dart:ffi' as ffi;
import 'package:ffi/ffi.dart';

Map<String, Function> _mappings = {};

class StrWrapper {
  static final Finalizer<ffi.Pointer<Utf8>> finalizer =
      Finalizer((p) => malloc.free(p));

  late final ffi.Pointer<Utf8> _p;
  StrWrapper(String s) {
    _p = s.toNativeUtf8();
    finalizer.attach(this, _p);
  }

  ffi.Pointer<Utf8> get p => _p;
}

dynamic invoke_f_any(ffi.Pointer<Utf8> p) {
  String s = p.toDartString();
  malloc.free(p);
  var parts = s.split(' ');
  List<dynamic> args = [];
  for (int i = 1; i < parts.length; i++) {
    args.add(jsonDecode(parts[i]));
  }
  var f = _mappings[parts[0]];
  return Function.apply(f!, args);
}

int invoke_f_int(ffi.Pointer<Utf8> p) => invoke_f_any(p);
double invoke_f_float(ffi.Pointer<Utf8> p) => invoke_f_any(p);
bool invoke_f_bool(ffi.Pointer<Utf8> p) => invoke_f_any(p);
ffi.Pointer<Utf8> invoke_f_str(ffi.Pointer<Utf8> p) =>
    StrWrapper(invoke_f_any(p)).p;
void invoke_f_None(ffi.Pointer<Utf8> p) => invoke_f_any(p);

ffi.Pointer f_int() {
  return ffi.Pointer.fromFunction<ffi.Int64 Function(ffi.Pointer<Utf8>)>(
      invoke_f_int, 0);
}

ffi.Pointer f_float() {
  return ffi.Pointer.fromFunction<ffi.Double Function(ffi.Pointer<Utf8>)>(
      invoke_f_float, 0.0);
}

ffi.Pointer f_bool() {
  return ffi.Pointer.fromFunction<ffi.Bool Function(ffi.Pointer<Utf8>)>(
      invoke_f_bool, false);
}

ffi.Pointer f_str() {
  return ffi.Pointer.fromFunction<
      ffi.Pointer<Utf8> Function(ffi.Pointer<Utf8>)>(invoke_f_str);
}

ffi.Pointer f_None() {
  return ffi.Pointer.fromFunction<ffi.Void Function(ffi.Pointer<Utf8>)>(
      invoke_f_None);
}

void register(String? key, Function value) {
  _mappings[key!] = value;
}

int t_code<T>() {
  if (T == int) return 'i'.codeUnitAt(0);
  if (T == double) return 'f'.codeUnitAt(0);
  if (T == bool) return 'b'.codeUnitAt(0);
  if (T == String) return 's'.codeUnitAt(0);
  return 'N'.codeUnitAt(0);
  // throw Exception("Type must be int/double/bool/String");
}
