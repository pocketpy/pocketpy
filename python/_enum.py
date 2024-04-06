class Enum:
    def __init__(self, name, value):
        self.name = name
        self.value = value

    def __str__(self):
        return f'{type(self).__name__}.{self.name}'
    
    def __repr__(self):
        return f'<{str(self)}: {self.value!r}>'
    
