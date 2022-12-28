import 'package:js/js.dart';

@JS("Module.ccall")
external int ccall(String name, String returnType, List<String> argTypes,
    List<dynamic> args);
