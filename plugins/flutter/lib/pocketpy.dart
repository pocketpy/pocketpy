library pocketpy;

export 'jsonrpc.dart';
export 'no_web.dart' if (dart.library.html) 'web.dart';