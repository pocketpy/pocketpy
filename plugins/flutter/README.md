# Welcome to PocketPy

PocketPy is a lightweight Python interpreter for game engines.

![](https://pocketpy.dev/static/logo_flat.png)
# Basic Features

The following table shows the basic features of PocketPy with respect to [CPython](https://github.com/python/cpython).
The features marked with `YES` are supported, and the features marked with `NO` are not supported.

| Name            | Example                    | Supported |
| --------------- | -------------------------- | --------- |
| If Else         | `if..else..elif`           | YES       |
| Loop            | `for/while/break/continue` | YES       |
| Function        | `def f(x,*args,y=1):`      | YES       |
| Function `**`   | `def f(**kwargs):`         | NO        |
| Subclass        | `class A(B):`              | YES       |
| List            | `[1, 2, 'a']`              | YES       |
| ListComp        | `[i for i in range(5)]`    | YES       |
| Slice           | `a[1:2], a[:2], a[1:]`     | YES       |
| Tuple           | `(1, 2, 'a')`              | YES       |
| Dict            | `{'a': 1, 'b': 2}`         | YES       |
| F-String        | `f'value is {x}'`          | YES       |
| Unpacking       | `a, b = 1, 2`              | YES       |
| Star Unpacking  | `a, *b = [1, 2, 3]`        | NO        |
| Throw Exception | `assert/raise`             | YES       |
| Catch Exception | `try..catch`               | NO        |
| Eval            | `eval()`                   | YES       |
| Import          | `import/from..import`      | YES       |
| Context Block   | `with <expr> as <id>:`     | YES       |

## Introduction

<p>
  <a title="Pub" href="https://pub.dev/packages/pocketpy" ><img src="https://img.shields.io/pub/v/pocketpy" /></a>
</p>

This plugin provides object-oriented interfaces including full functionality of PocketPy [C-API](https://pocketpy.dev/c-api/vm).
It also provides `JsonRpcServer` class and `os` module bindings.

Run the following script to install this plugin.

```
flutter pub add pocketpy
```

## Requirements

#### For Android

You may need to set the Android NDK version to "21.4.7075529" or higher in `android/app/build.gradle`.
```
android {
    ndkVersion "21.4.7075529"
}
```

#### For iOS

It should work without any setup.

#### For Web

Download an artifact from https://github.com/blueloveTH/pocketpy/releases/latest.

Unzip it and copy `web/lib` into your root folder where `index.html` locates.

```
...
lib/pocketpy.js
lib/pocketpy.wasm
index.html
...
```

Then open `index.html` and add this line before `flutter.js` tag.

```
...
  <!-- This script initializes WASM of pocketpy -->
  <script src="./lib/pocketpy.js"></script>

  <!-- This script adds the flutter initialization JS code -->
  <script src="flutter.js" defer></script>
...
```


#### For Windows

VS2017 or higher is required to build the windows `.dll`.
Make sure you have C++ component installed.


## Basic Example

```dart
import 'package:pocketpy/pocketpy.dart' as pkpy;

// Create a virtual machine
pkpy.VM vm = pkpy.VM();

// Run a script
String code = 'print("Hello World!")';
vm.exec(code);

// Read the output
var _o = vm.read_output();
print(_o.stdout)	// "Hello world!\n"
print(_o.stderr)	// ""
```



## REPL Widget Example

```dart
import 'dart:io';

import 'package:flutter/material.dart';
import 'package:pocketpy/pocketpy.dart' as pkpy;

void main() {
  runApp(const MaterialApp(home: MyApp()));
}

class MyApp extends StatefulWidget {
  const MyApp({super.key});

  @override
  State<MyApp> createState() => _MyAppState();
}

class _MyAppState extends State<MyApp> {
  late final pkpy.VM vm;
  late final pkpy.REPL repl;
  bool needMoreLines = false;

  final TextEditingController _controller = TextEditingController();
  final StringBuffer buffer = StringBuffer();

  @override
  void initState() {
    super.initState();

    // create a pocketpy virtual machine
    vm = pkpy.VM();

    // create a REPL
    repl = pkpy.REPL(vm);

    WidgetsBinding.instance.addPostFrameCallback((timeStamp) {
      refresh();
    });
  }

  void addMessage(String text) {
    setState(() {
      buffer.write(text);
    });
  }

  void submitCode() {
    var text = _controller.text;
    _controller.clear();
    setState(() {
      buffer.write(needMoreLines ? '... $text' : '>>> $text\n');
    });
    if (text == "exit()") exit(0);
    needMoreLines = repl.input(text) == 0;
    refresh();
  }

  void refresh() {
    // ignore: no_leading_underscores_for_local_identifiers
    var _o = vm.read_output();
    if (_o.stdout.isNotEmpty) buffer.write(_o.stdout);
    if (_o.stderr.isNotEmpty) buffer.write(_o.stderr);
    setState(() {});
  }

  @override
  Widget build(BuildContext context) {
    var style = const TextStyle(fontSize: 16);
    return Scaffold(
      appBar: AppBar(
        title: const Text('Demo'),
      ),
      body: Padding(
        padding: const EdgeInsets.all(8.0),
        child: Column(crossAxisAlignment: CrossAxisAlignment.start, children: [
          Expanded(
            child: SingleChildScrollView(
              reverse: true,
              child: Text(
                buffer.toString(),
                style: style,
                textAlign: TextAlign.left,
              ),
            ),
          ),
          const SizedBox(
            height: 16,
          ),
          SizedBox(
            height: 50,
            child: TextFormField(
              controller: _controller,
              style: style,
              maxLines: 1,
              decoration: const InputDecoration(
                border: OutlineInputBorder(),
                hintText: 'Enter Python code',
              ),
            ),
          ),
          Container(
            height: 60,
            alignment: Alignment.centerRight,
            child: MaterialButton(
                onPressed: submitCode,
                color: Colors.blue,
                textColor: Colors.white,
                child: const Text('Run')),
          ),
        ]),
      ),
    );
  }
}
```

