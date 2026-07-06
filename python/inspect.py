class _empty:
    pass


class Parameter:
    POSITIONAL_ONLY = 0
    POSITIONAL_OR_KEYWORD = 1
    VAR_POSITIONAL = 2
    KEYWORD_ONLY = 3
    VAR_KEYWORD = 4

    empty = _empty

    def __init__(self, name, kind, *default, annotation=None):
        self.name = name
        self.kind = kind
        # pocketpy only allows literal defaults, so use *default as sentinel
        self.default = default[0] if default else _empty
        self.annotation = _empty if annotation is None else annotation

    def __str__(self):
        res = self.name
        if self.annotation is not _empty:
            res += ': ' + self.annotation
            if self.default is not _empty:
                res += ' = ' + repr(self.default)
        elif self.default is not _empty:
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

    def __init__(self, parameters, return_annotation=None):
        self.parameters = {p.name: p for p in parameters}
        self.return_annotation = _empty if return_annotation is None else return_annotation

    def __str__(self):
        res = '(' + ', '.join([str(p) for p in self.parameters.values()]) + ')'
        if self.return_annotation is not _empty:
            res += ' -> ' + self.return_annotation
        return res

    def __repr__(self):
        return '<Signature ' + str(self) + '>'


def _from_function(func, drop_first):
    annotations = func.__annotations__
    params = [Parameter(*entry, annotation=annotations.get(entry[0]))
              for entry in _signature_data(func)]
    if drop_first:
        params = params[1:]
    return Signature(params, annotations.get('return'))


def signature(obj):
    if not callable(obj):
        raise TypeError(repr(obj) + ' is not a callable object')
    if isinstance(obj, type):
        return _from_function(obj.__init__, True)
    if hasattr(obj, '__func__'):
        return _from_function(obj.__func__, True)
    return _from_function(obj, False)
