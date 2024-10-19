import 'dart:ffi';
import 'package:ffi/ffi.dart';
import 'package:flutter/material.dart';

import 'package:pocketpy/pocketpy.dart';
import 'package:pocketpy/pocketpy_bindings_generated.dart';

void main() {
  runApp(const MyApp());
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  @override
  Widget build(BuildContext context) {
    bool ok = pocket.py_exec(
        'import sys\nprint(sys.version)'.toNativeUtf8().cast(),
        'main.py'.toNativeUtf8().cast(),
        py_CompileMode.EXEC_MODE,
        Pointer.fromAddress(0));

    if (!ok) {
      pocket.py_printexc();
    }

    return MaterialApp(
      home: Scaffold(
        appBar: AppBar(
          title: const Text('Native Packages'),
        ),
        body: SingleChildScrollView(
          child: Container(
            padding: const EdgeInsets.all(10),
            child: const Text(""),
          ),
        ),
      ),
    );
  }
}
