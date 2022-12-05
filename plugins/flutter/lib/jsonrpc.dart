// ignore_for_file: no_leading_underscores_for_local_identifiers, non_constant_identifier_names

import 'dart:async';
import 'dart:convert';
import 'dart:io';

import 'package:pocketpy/pocketpy.dart';

class _JsonRpcError {
  final Map<String, dynamic> payload = {};

  _JsonRpcError(int code, String message, {dynamic data}) {
    payload['code'] = code;
    payload['message'] = message;
    if (data != null) {
      payload['data'] = data;
    }
  }
}

class JsonRpcServer {
  final Map<String, FutureOr<dynamic> Function(List)> _methods = {};

  final void Function()? onPreDispatch;
  final void Function()? onPostDispatch;
  final bool enableFileAccess;

  JsonRpcServer(
      {this.onPreDispatch,
      this.onPostDispatch,
      this.enableFileAccess = false}) {
    if (!enableFileAccess) return;
    registerOS(this);
  }

  /// Register a JSONRPC handler.
  void register(String name, FutureOr<dynamic> Function(List) method) {
    _methods[name] = method;
  }

  FutureOr<dynamic> _dispatch(ThreadedVM vm) {
    if (vm.state != ThreadState.suspended) throw Exception("Unexpected state");
    String? json = vm.read_jsonrpc_request();
    if (json == null) throw Exception("JSONRPC request is null");
    var request = jsonDecode(json);
    var f = _methods[request['method']];
    if (f == null) throw _JsonRpcError(-32601, "Method not found");
    try {
      return f(request['params'] as List);
    } catch (e) {
      throw _JsonRpcError(-32000, e.toString());
    }
  }

  /// Dispatch a JSONRPC request.
  FutureOr<void> dispatch(ThreadedVM vm) async {
    onPreDispatch?.call();
    try {
      dynamic ret = _dispatch(vm);
      if (ret is Future<dynamic>) ret = await ret;
      vm.write_jsonrpc_response(jsonEncode({"result": ret}));
      onPostDispatch?.call();
    } on _JsonRpcError catch (e) {
      vm.write_jsonrpc_response(jsonEncode({"error": e.payload}));
      return;
    }
  }

  /// Attach the JsonRpcServer into a ThreadedVM. Once the ThreadedVM encounters JSONRPC request, it takes care of it automatically. This process will be stopped when the whole execution is done.
  Future<void> attach(ThreadedVM vm,
      {Duration? spinFreq = const Duration(milliseconds: 20)}) async {
    while (vm.state.index <= ThreadState.running.index) {
      if (spinFreq != null) await Future.delayed(spinFreq);
    }
    switch (vm.state) {
      case ThreadState.suspended:
        await dispatch(vm);
        await attach(vm, spinFreq: spinFreq);
        break;
      case ThreadState.finished:
        break;
      default:
        throw Exception("Unexpected state");
    }
  }

  int _fileId = 0;
  final Map<int, File> _files = {};

  void registerOS(JsonRpcServer rpcServer) {
    rpcServer.register("fopen", (params) {
      var path = params[0];
      //var mode = params[1];
      var fp = File(path);
      _fileId += 1;
      _files[_fileId] = fp;
      return _fileId;
    });

    rpcServer.register("fclose", (params) {
      var fp = _files[params[0]];
      if (fp == null) throw Exception("FileIO was closed");
      _files.remove(params[0]);
    });

    rpcServer.register("fread", (params) {
      var fp = _files[params[0]];
      if (fp == null) throw Exception("FileIO was closed");
      return fp.readAsStringSync();
    });

    rpcServer.register("fwrite", (params) {
      var fp = _files[params[0]];
      if (fp == null) throw Exception("FileIO was closed");
      fp.writeAsStringSync(params[1]);
    });

    rpcServer.register("os.listdir", (params) {
      String path = params[0];
      var entries = Directory(path).listSync(followLinks: false);
      var ret = entries.map((e) {
        return e.path.split(Platform.pathSeparator).last;
      }).toList();
      return ret;
    });

    rpcServer.register("os.mkdir", (params) {
      String path = params[0];
      Directory(path).createSync();
    });

    rpcServer.register("os.rmdir", (params) {
      String path = params[0];
      Directory(path).deleteSync(recursive: true);
    });

    rpcServer.register("os.remove", (params) {
      String path = params[0];
      File(path).deleteSync();
    });

    rpcServer.register("os.path.exists", (params) {
      String path = params[0];
      bool _0 = Directory(path).existsSync();
      bool _1 = File(path).existsSync();
      return (_0 || _1);
    });
  }
}
