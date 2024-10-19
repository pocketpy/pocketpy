import 'dart:ffi';
import 'dart:io';
import 'package:ffi/ffi.dart';

import 'pocketpy_bindings_generated.dart';

const String _libName = 'pocketpy';

void flutterPrint(Pointer<Char> text) {
  // ignore: avoid_print
  print(text.cast<Utf8>().toDartString());
}

final PocketpyBindings pocket = () {
  DynamicLibrary dylib;
  if (Platform.isMacOS || Platform.isIOS) {
    dylib = DynamicLibrary.open('$_libName.framework/$_libName');
  } else if (Platform.isAndroid || Platform.isLinux) {
    dylib = DynamicLibrary.open('lib$_libName.so');
  } else if (Platform.isWindows) {
    dylib = DynamicLibrary.open('$_libName.dll');
  } else {
    throw UnsupportedError('Unknown platform: ${Platform.operatingSystem}');
  }
  PocketpyBindings retval = PocketpyBindings(dylib);
  retval.py_initialize();
  retval.py_callbacks().ref.print = Pointer.fromFunction(flutterPrint);
  return retval;
}();
