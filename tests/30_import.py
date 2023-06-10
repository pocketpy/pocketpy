try:
    import os
except ImportError:
    exit(0)

import test1

assert test1.add(1, 2) == 13