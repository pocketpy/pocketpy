try:
    import os
except ImportError:
    exit(0)

import sys
is_pkpy = not hasattr(sys, 'getrefcount')

os.chdir('benchmarks')

if is_pkpy:
    try:
        import cjson as json
    except ImportError:
        print('[cJSON not Enabled]')
        exit(0)
else:
    import json

_2489KB = 'WorldMap_GridVania_layout.ldtk'
_1093KB = 'WorldMap_Free_layout.ldtk'
_339KB = 'Typical_2D_platformer_example.ldtk'

with open(f'res/{_2489KB}', 'r') as f:
    json_content = f.read()

data: dict = json.loads(json_content)
assert isinstance(data, dict)

# serialize and deserialize
dumped: str = json.dumps(data)
for _ in range(10):
    loaded: dict = json.loads(dumped)
assert len(data) == len(loaded)
assert data == loaded

#### very very slow!!
import pickle

with open(f'res/{_339KB}', 'r') as f:
    json_content = f.read()
data: dict = json.loads(json_content)

data_pickled: bytes = pickle.dumps(data)
assert isinstance(data_pickled, bytes)
assert pickle.loads(data_pickled) == data