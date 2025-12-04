from array2d import array2d, chunked_array2d
from vmath import color32, vec2i

'''
src/modules/array2d.c
'''

# =====array2d_like_render
data = [
    [1,2,3],
    [4,5,6],
]
assert array2d.fromlist(data).render() == "123\n456"

# ====array2d_like_render_with_color
text_data = []
color_fg = []
color_bg = []
for i in range(51):
    r = i * 5  # r in [0,5,10,15...250,255]
    g = i * 5  # g in [0,5,10,15...250,255]
    
    row_fg = []
    row_bg = []
    row_text_data = []
    for j in range(51):
        b = j * 5  # j in [0,5,10,15...250,255]
        row_fg.append(color32(255-r, 255-g, 255-b, 255))
        row_bg.append(color32(r, g, b, 255))
        row_text_data.append("A")
    
    color_fg.append(row_fg)
    color_bg.append(row_bg)
    text_data.append(row_text_data)


array2d.fromlist(text_data).render_with_color(
    array2d.fromlist(color_fg),
    array2d.fromlist(color_bg)
    )

array2d.fromlist(text_data).render_with_color(
    array2d.fromlist(color_fg),
    array2d(51, 51, None)
    )

array2d.fromlist(text_data).render_with_color(
    array2d(51, 51, None),
    array2d.fromlist(color_bg)
    )

# (curr_fg.u32 != 0 || curr_bg.u32 != 0) == false
array2d.fromlist(text_data).render_with_color(
    array2d(51, 51, None),
    array2d(51, 51, None)
    )

try:
    array2d.fromlist(text_data).render_with_color(
        array2d.fromlist(color_fg),
        array2d(51, 51, 1)
        )
except TypeError as e:
    pass

try:
    array2d.fromlist(text_data).render_with_color(
        array2d(51, 51, 1),
        array2d.fromlist(color_bg)
        )
except TypeError as e:
    pass


# ====array2d_like_any
data = [[False, False], [False, False]]
assert array2d.fromlist(data).any() == False

# ====c11_array2d_view__set
data = chunked_array2d(4)
data[vec2i(3,3)] = 0
data.view()[1,1] = 10
assert data.view() == array2d.fromlist([
    [None, None, None, None,],
    [None, 10, None, None,],
    [None, None, 0, None,],
    [None, None, None, None,],
    ])

# ====array2d_view_origin
assert data.view()[vec2i(1,1)-data.view().origin] == data[vec2i(1,1)]
assert data.view()[vec2i(3,3)-data.view().origin] == data[vec2i(3,3)]

# ====chunked_array2d__delitem__
data = chunked_array2d(4)
for i in range(10):
    for j in range(10):
        data[vec2i(i,j)] = 10

del data[vec2i(0,0)]


# ====chunked_array2d__len__
data = chunked_array2d(4)
for i in range(10):
    for j in range(10):
        data[vec2i(i,j)] = 10

assert len(data) == 9

# ====c11_chunked_array2d__mark
import gc

def gc_collect_callback(statue, msg):
    if statue == 'stop':
        for line in msg.split('\n'):
            print(line)
            if "5290" in line and "str" in line:
                return
        print(msg)
        assert False

gc.collect()
gc.setup_debug_callback(gc_collect_callback)
def create_garbage():
    data = chunked_array2d(4)
    # [str  ] small:      0  large:   5290
    for x in range(23):
        for y in range(230):
            data[vec2i(x, y)] = "garbage"*100
            
create_garbage()
gc.collect()


# ====chunked_array2d_view_chunks
data = chunked_array2d(4, default = 0)
data[vec2i(-10,-10)] = -1
assert data.view_chunks(vec2i(-3, -3), 6, 6)[vec2i(2, 2)] == -1
data.view_chunks(vec2i(-3, -3), 6, 6)[vec2i(22, 22)] = 1
assert data[vec2i(10,10)] == 1


