import json

_2489KB = 'WorldMap_GridVania_layout.ldtk'
_1093KB = 'WorldMap_Free_layout.ldtk'
_339KB = 'Typical_2D_platformer_example.ldtk'

with open(f'res/{_2489KB}', 'r') as f:
    json_content = f.read()

data: dict = json.loads(json_content)
assert isinstance(data, dict)

# dumped: str = json.dumps(data)
# loaded: dict = json.loads(dumped)
# assert len(data) == len(loaded)
# assert data == loaded

# import pickle
##### very very slow!! DO NOT RUN IT
# data_pickled: bytes = pickle.dumps(data)
# assert isinstance(data_pickled, bytes)
# assert pickle.loads(data_pickled) == data