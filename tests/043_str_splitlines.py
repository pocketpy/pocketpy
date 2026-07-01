try:
    ''.splitlines(5)
    exit(1)
except TypeError:
    pass

try:
    ''.splitlines(5, 5)
    exit(1)
except TypeError:
    pass

assert ''.splitlines() == []
assert ''.splitlines(False) == []
assert ''.splitlines(True) == []

assert '\n'.splitlines() == ['']                        # Line Feed
assert '\r'.splitlines() == ['']                        # Carriage return
assert '\r\n'.splitlines() == ['']                      # Line feed + carriage return
assert '\v'.splitlines() == ['']                        # Line tabulation
assert '\f'.splitlines() == ['']                        # Form field
assert '\x1c'.splitlines() == ['']                      # File separator
assert '\x1d'.splitlines() == ['']                      # Group separator
assert '\x1e'.splitlines() == ['']                      # Record separator
assert b'\xc2\x85'.decode().splitlines() == ['']        # \u85   Next line (C1 control code)
assert b'\xe2\x80\xa8'.decode().splitlines() == ['']    # \u2028 Unicode line separator
assert b'\xe2\x80\xa9'.decode().splitlines() == ['']    # \u2029 Unicode paragraph separator
assert '🥕'.splitlines() == ['🥕']

all_ends = ['\n', '\r', '\r\n', '\v', '\f', '\x1c', '\x1d', '\x1e', b'\xc2\x85'.decode(), b'\xe2\x80\xa8'.decode(), b'\xe2\x80\xa9'.decode()]
for eol in all_ends:
    assert (eol).splitlines() == ['']
    assert (eol).splitlines(False) == ['']
    assert (eol).splitlines(True) == [eol]
    for text in ['a', 'a b', 'abc\tdef', '🥕 and 🍋', '测试123测试']:
        assert (text).splitlines(False) == [text]
        assert (text).splitlines(True) == [text]
        assert (text + eol).splitlines(False) == [text]
        assert (text + eol).splitlines(True) == [text + eol]
        assert (eol + eol).splitlines(False) == ['', '']
        assert (eol + eol).splitlines(True) == [eol, eol]
        assert (eol + text).splitlines(False) == ['', text]
        assert (eol + text).splitlines(True) == [eol, text]
        assert (text + eol + eol + eol + eol).splitlines(False) == [text, '', '', '']
        assert (text + eol + eol + eol + eol).splitlines(True) == [text + eol, eol, eol, eol]
        assert (eol + eol + text + eol + eol + eol + eol).splitlines(False) == ['', '', text, '', '', '']
        assert (eol + eol + text + eol + eol + eol + eol).splitlines(True) == [eol, eol, text + eol, eol, eol, eol]
        assert (text + eol + text).splitlines(False) == [text, text]
        assert (text + eol + text).splitlines(True) == [text + eol, text]
        assert (text + eol + text + eol).splitlines(False) == [text, text]
        assert (text + eol + text + eol).splitlines(True) == [text + eol, text + eol]

assert '\r\r\n'.splitlines() == ['', '']
assert '\r\r\n'.splitlines(True) == ['\r', '\r\n']
assert '\n\r\r\n'.splitlines() == ['', '', '']
assert '\n\r\r\n'.splitlines(True) == ['\n', '\r', '\r\n']
assert '\n\r\r\n\n'.splitlines() == ['', '', '', '']
assert '\n\r\r\n\n'.splitlines(True) == ['\n', '\r', '\r\n', '\n']
assert ''.join(all_ends).splitlines() == [''] * len(all_ends)
assert ''.join(all_ends).splitlines(True) == all_ends
