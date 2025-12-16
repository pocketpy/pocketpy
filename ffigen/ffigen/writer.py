class Writer:
    def __init__(self) -> None:
        self.buffer = []
        self.indent_level = 0

    def indent(self):
        self.indent_level += 1

    def dedent(self):
        self.indent_level -= 1

    def write(self, line: str):
        self.buffer.append('    ' * self.indent_level + line)

    def __str__(self) -> str:
        return '\n'.join(self.buffer)