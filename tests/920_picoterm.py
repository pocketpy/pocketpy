import picoterm
from vmath import rgb

picoterm.enable_full_buffering_mode()

bg = rgb(78, 118, 164)
fg = rgb(200, 200, 0)
text = "hello, \nworld"
text = bg.ansi_bg(text)
text = fg.ansi_fg(text)

def ansi_italic(text: str):
    return f'\x1b[3m{text}\x1b[0m'

text = ansi_italic(text) + '123'
print(text)

cpnts = picoterm.split_ansi_escaped_string(text)

assert cpnts == ['\x1b[3m', '\x1b[38;2;200;200;0m', '\x1b[48;2;78;118;164m', 'hello, ', '\n', 'world', '\x1b[0m', '\x1b[0m', '\x1b[0m', '123']

cpnts_join = ''.join(cpnts)
assert cpnts_join == text

assert picoterm.wcwidth(ord('\n')) == 0
assert picoterm.wcwidth(ord('a')) == 1
assert picoterm.wcwidth(ord('测')) == 2
assert picoterm.wcwidth(ord('👀')) == 2

assert picoterm.wcswidth("hello, 测试a测试👀测\n") == 7 + 1 + 12

text = rgb(12, 34, 56).ansi_fg("hello")
out_list = []
assert picoterm.sscanf(text, "\x1b[38;2;%d;%d;%dm", out_list)
assert out_list == [12, 34, 56]

assert picoterm.sscanf(text, "\x1b[38;2;%d;%d;%dmhello", out_list)
assert out_list == [12, 34, 56]

assert picoterm.sscanf(text, "\x1b[38;2;%d;%d;%d", out_list)
assert not picoterm.sscanf(text, "\x1b[38;2;%d;%d;%dm???", out_list)
assert not picoterm.sscanf(text, "\x1b[77;2;%d;%d;%dm", out_list)