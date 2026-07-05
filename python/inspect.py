class _empty:
    pass


class Parameter:
    POSITIONAL_ONLY = 0
    POSITIONAL_OR_KEYWORD = 1
    VAR_POSITIONAL = 2
    KEYWORD_ONLY = 3
    VAR_KEYWORD = 4

    empty = _empty

    def __init__(self, name, kind, *default):
        self.name = name
        self.kind = kind
        # pocketpy only allows literal defaults, so use *default as sentinel
        self.default = default[0] if default else _empty

    def __str__(self):
        res = self.name
        if self.default is not _empty:
            res += '=' + repr(self.default)
        if self.kind == Parameter.VAR_POSITIONAL:
            res = '*' + res
        elif self.kind == Parameter.VAR_KEYWORD:
            res = '**' + res
        return res

    def __repr__(self):
        return '<Parameter "' + str(self) + '">'


class Signature:
    empty = _empty

    def __init__(self, parameters):
        self.parameters = {p.name: p for p in parameters}

    def __str__(self):
        return '(' + ', '.join([str(p) for p in self.parameters.values()]) + ')'

    def __repr__(self):
        return '<Signature ' + str(self) + '>'


def _make_params(func):
    return [Parameter(*entry) for entry in _signature_data(func)]


def signature(obj):
    if not callable(obj):
        raise TypeError(repr(obj) + ' is not a callable object')
    if isinstance(obj, type):
        return Signature(_make_params(obj.__init__)[1:])
    if hasattr(obj, '__func__'):
        return Signature(_make_params(obj.__func__)[1:])
    return Signature(_make_params(obj))
