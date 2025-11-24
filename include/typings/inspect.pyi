def isgeneratorfunction(obj) -> bool: ...

def is_user_defined_type(t: type) -> bool:
    """Check if a type is user-defined.
    
    This means the type was created by executing python `class` statement."""