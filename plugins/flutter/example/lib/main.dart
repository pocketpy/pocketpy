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
    repl.input(text);
    needMoreLines = repl.last_input_result() == 0;
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
