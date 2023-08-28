D = 10

try:
    import abc  # does not exist
    exit(1)
except ImportError:
    pass