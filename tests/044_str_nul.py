# strings with embedded NUL bytes must compare by their full length

# literal dedup must keep same-length strings that differ after the NUL
assert "a\0b" != "a\0c"
assert not ("a\0b" == "a\0c")
assert "a\0b" == "a\0b"

# runtime-built strings
x = "a" + chr(0) + "c"
y = "a" + chr(0) + "b"
assert x != y
assert x > y
assert y < x

# ordering
assert "a" < "a\0b"
assert "a\0b" < "a\0c"
assert sorted(["a\0c", "a\0b", "a"]) == ["a", "a\0b", "a\0c"]

# membership
assert "a\0c" not in "a\0b"
assert "b" in "a\0b"
assert "\0" in "a\0b"
assert "a\0b" in "a\0b"
assert "" in "a\0b"
assert "a" in "a\0b"
assert x not in y
assert "b" in y
assert "a\0" in "a\0b"

# dict keys
d = {"a\0b": 1, "a\0c": 2}
assert len(d) == 2
assert d["a\0b"] == 1
assert d["a\0c"] == 2

# find / startswith keep working on the full length
assert "a\0b".find("b") == 2
assert "a\0b".find("c") == -1
assert "a\0b".startswith("a\0")
assert not "a\0b".startswith("a\0c")

# match/case literal patterns
def match_nul(v):
    match v:
        case "a\0b":
            return "b"
        case "a\0c":
            return "c"
        case _:
            return "other"

assert match_nul("a\0b") == "b"
assert match_nul("a\0c") == "c"
assert match_nul("a\0d") == "other"
assert match_nul("a") == "other"

# identity: short strings compare by content, long strings by object
s = "a\0b"
t = "a\0c"
assert not (s is t)
assert not (x is y)
plain = "a"
assert not (plain is s)
long_a = "a" * 20
assert long_a == "a" * 20
assert not (long_a is plain)
assert not (plain is long_a)

# Length 15 uses inline storage; lengths 16 and above use heap storage.
for size in (15, 16, 17, 64):
    prefix = "a" * (size - 2)
    first = prefix + chr(0) + "b"
    second = prefix + chr(0) + "c"
    equal_first = prefix + chr(0) + "b"
    assert len(first) == size
    assert first == equal_first
    assert first != second
    assert not (first == second)
    assert first < second
    assert first <= equal_first
    assert second > first
    assert second >= first
    assert sorted([second, first, prefix]) == [prefix, first, second]

    assert "b" in first
    assert chr(0) in first
    assert chr(0) + "b" in first
    assert "" in first
    assert equal_first in first
    assert second not in first
    assert first not in prefix
    assert first.find("b") == size - 1
    assert first.count(chr(0)) == 1
    assert first.endswith(chr(0) + "b")

    keys = {first: 1, second: 2}
    assert len(keys) == 2
    assert keys[equal_first] == 1
    assert keys[second] == 2

    same_reference = first
    assert same_reference is first
    assert not (first is second)
    if size >= 16:
        assert not (first is equal_first)

# Mixed inline/heap comparisons must examine the string data in both orders.
short_nul = "a" + chr(0)
long_nul = short_nul + "b" * 20
assert short_nul != long_nul
assert long_nul != short_nul
assert not (short_nul is long_nul)
assert not (long_nul is short_nul)

# A leading or trailing NUL is part of the value, including in heap strings.
for size in (15, 16, 17):
    body = "z" * (size - 1)
    leading = chr(0) + body
    trailing = body + chr(0)
    assert len(leading) == size
    assert len(trailing) == size
    assert body in leading
    assert leading not in body
    assert trailing not in body
    assert leading.startswith(chr(0))
    assert trailing.endswith(chr(0))
    assert leading.find("z") == 1
    assert trailing.find(chr(0)) == size - 1

# Same-length heap literals must not be merged during constant deduplication.
literal_b = "abcdefghijklmnopqrstuvwxyz\0b"
literal_c = "abcdefghijklmnopqrstuvwxyz\0c"
assert literal_b != literal_c
assert literal_b < literal_c

def match_heap_nul(value):
    match value:
        case "abcdefghijklmnopqrstuvwxyz\0b":
            return "b"
        case "abcdefghijklmnopqrstuvwxyz\0c":
            return "c"
        case _:
            return "other"

assert match_heap_nul(literal_b) == "b"
assert match_heap_nul(literal_c) == "c"
assert match_heap_nul("abcdefghijklmnopqrstuvwxyz\0d") == "other"
assert match_heap_nul("abcdefghijklmnopqrstuvwxyz") == "other"
