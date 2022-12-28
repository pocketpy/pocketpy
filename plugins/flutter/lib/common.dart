class PyOutput {
  final String stdout;
  final String stderr;
  PyOutput(this.stdout, this.stderr);

  PyOutput.fromJson(Map<String, dynamic> json)
    : stdout = json['stdout'], stderr = json['stderr'];
}
