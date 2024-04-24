from operator import lt as __operator_lt
from operator import gt as __operator_gt
from __builtins import next as __builtins_next

def __minmax_reduce(op, args, key):
    if key is None:
        if len(args) == 2:
            return args[0] if op(args[0], args[1]) else args[1]
    if len(args) == 0:
        raise TypeError('expected 1 arguments, got 0')
    if len(args) == 1:
        args = args[0]
    args = iter(args)
    res = __builtins_next(args)
    if res is StopIteration:
        raise ValueError('args is an empty sequence')
    while True:
        i = __builtins_next(args)
        if i is StopIteration:
            break
        if key is None:
            if op(i, res):
                res = i
        else:
            if op(key(i), key(res)):
                res = i
    return res

def min(*args, key=None):
    return __minmax_reduce(__operator_lt, args, key)

def max(*args, key=None):
    return __minmax_reduce(__operator_gt, args, key)

def all(iterable):
    for i in iterable:
        if not i:
            return False
    return True

def any(iterable):
    for i in iterable:
        if i:
            return True
    return False

def enumerate(iterable, start=0):
    n = start
    for elem in iterable:
        yield n, elem
        ++n

def sum(iterable):
    res = 0
    for i in iterable:
        res += i
    return res

def map(f, iterable):
    for i in iterable:
        yield f(i)

def filter(f, iterable):
    for i in iterable:
        if f(i):
            yield i

def zip(a, b):
    a = iter(a)
    b = iter(b)
    while True:
        ai = __builtins_next(a)
        bi = __builtins_next(b)
        if ai is StopIteration or bi is StopIteration:
            break
        yield ai, bi

def reversed(iterable):
    a = list(iterable)
    a.reverse()
    return a

def sorted(iterable, key=None, reverse=False):
    a = list(iterable)
    a.sort(key=key, reverse=reverse)
    return a

##### str #####
def __format_string(self: str, *args, **kwargs) -> str:
    def tokenizeString(s: str):
        tokens = []
        L, R = 0,0
        
        mode = None
        curArg = 0
        # lookingForKword = False
        
        while(R<len(s)):
            curChar = s[R]
            nextChar = s[R+1] if R+1<len(s) else ''
            
            # Invalid case 1: stray '}' encountered, example: "ABCD EFGH {name} IJKL}", "Hello {vv}}", "HELLO {0} WORLD}"
            if curChar == '}' and nextChar != '}':
                raise ValueError("Single '}' encountered in format string")        
            
            # Valid Case 1: Escaping case, we escape "{{ or "}}" to be "{" or "}", example: "{{}}", "{{My Name is {0}}}"
            if (curChar == '{' and nextChar == '{') or (curChar == '}' and nextChar == '}'):
                
                if (L<R): # Valid Case 1.1: make sure we are not adding empty string
                    tokens.append(s[L:R]) # add the string before the escape
                
                
                tokens.append(curChar) # Valid Case 1.2: add the escape char
                L = R+2 # move the left pointer to the next char
                R = R+2 # move the right pointer to the next char
                continue
            
            # Valid Case 2: Regular command line arg case: example:  "ABCD EFGH {} IJKL", "{}", "HELLO {} WORLD"
            elif curChar == '{' and nextChar == '}':
                if mode is not None and mode != 'auto':
                    # Invalid case 2: mixing automatic and manual field specifications -- example: "ABCD EFGH {name} IJKL {}", "Hello {vv} {}", "HELLO {0} WORLD {}" 
                    raise ValueError("Cannot switch from manual field numbering to automatic field specification")
                
                mode = 'auto'
                if(L<R): # Valid Case 2.1: make sure we are not adding empty string
                    tokens.append(s[L:R]) # add the string before the special marker for the arg
                
                tokens.append("{"+str(curArg)+"}") # Valid Case 2.2: add the special marker for the arg
                curArg+=1 # increment the arg position, this will be used for referencing the arg later
                
                L = R+2 # move the left pointer to the next char
                R = R+2 # move the right pointer to the next char
                continue
            
            # Valid Case 3: Key-word arg case: example: "ABCD EFGH {name} IJKL", "Hello {vv}", "HELLO {name} WORLD"
            elif (curChar == '{'):
                
                if mode is not None and mode != 'manual':
                    # # Invalid case 2: mixing automatic and manual field specifications -- example: "ABCD EFGH {} IJKL {name}", "Hello {} {1}", "HELLO {} WORLD {name}"
                    raise ValueError("Cannot switch from automatic field specification to manual field numbering")
                
                mode = 'manual'
                
                if(L<R): # Valid case 3.1: make sure we are not adding empty string
                    tokens.append(s[L:R]) # add the string before the special marker for the arg
                
                # We look for the end of the keyword          
                kwL = R # Keyword left pointer
                kwR = R+1 # Keyword right pointer
                while(kwR<len(s) and s[kwR]!='}'):
                    if s[kwR] == '{': # Invalid case 3: stray '{' encountered, example: "ABCD EFGH {n{ame} IJKL {", "Hello {vv{}}", "HELLO {0} WOR{LD}"
                        raise ValueError("Unexpected '{' in field name")
                    kwR += 1
                
                # Valid case 3.2: We have successfully found the end of the keyword
                if kwR<len(s) and s[kwR] == '}':
                    tokens.append(s[kwL:kwR+1]) # add the special marker for the arg
                    L = kwR+1
                    R = kwR+1
                    
                # Invalid case 4: We didn't find the end of the keyword, throw error
                else:
                    raise ValueError("Expected '}' before end of string")
                continue
            
            R = R+1
        
        
        # Valid case 4: We have reached the end of the string, add the remaining string to the tokens 
        if L<R:
            tokens.append(s[L:R])
                
        # print(tokens)
        return tokens

    tokens = tokenizeString(self)
    argMap = {}
    for i, a in enumerate(args):
        argMap[str(i)] = a
    final_tokens = []
    for t in tokens:
        if t[0] == '{' and t[-1] == '}':
            key = t[1:-1]
            argMapVal = argMap.get(key, None)
            kwargsVal = kwargs.get(key, None)
                                    
            if argMapVal is None and kwargsVal is None:
                raise ValueError("No arg found for token: "+t)
            elif argMapVal is not None:
                final_tokens.append(str(argMapVal))
            else:
                final_tokens.append(str(kwargsVal))
        else:
            final_tokens.append(t)
    
    return ''.join(final_tokens)

str.format = __format_string
del __format_string


def help(obj):
    if hasattr(obj, '__func__'):
        obj = obj.__func__
    print(obj.__signature__)
    print(obj.__doc__)

def complex(*args, **kwargs):
    import cmath
    return cmath.complex(*args, **kwargs)

def long(*args, **kwargs):
    import _long
    return _long.long(*args, **kwargs)


# builtin exceptions
class StackOverflowError(Exception): pass
class IOError(Exception): pass
class NotImplementedError(Exception): pass
class TypeError(Exception): pass
class IndexError(Exception): pass
class ValueError(Exception): pass
class RuntimeError(Exception): pass
class ZeroDivisionError(Exception): pass
class NameError(Exception): pass
class UnboundLocalError(Exception): pass
class AttributeError(Exception): pass
class ImportError(Exception): pass
class AssertionError(Exception): pass

class KeyError(Exception):
    def __init__(self, key=...):
        self.key = key
        if key is ...:
            super().__init__()
        else:
            super().__init__(repr(key))

    def __str__(self):
        if self.key is ...:
            return ''
        return str(self.key)
    
    def __repr__(self):
        if self.key is ...:
            return 'KeyError()'
        return f'KeyError({self.key!r})'
