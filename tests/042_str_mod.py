# Test str.__mod__ (old-style % formatting)

# Test %s - string formatting
assert "hello %s" % "world" == "hello world"
assert "%s" % 123 == "123"
assert "%s %s" % ("a", "b") == "a b"
assert "name: %s, age: %s" % ("Alice", 30) == "name: Alice, age: 30"

# Test %d - integer formatting
assert "%d" % 42 == "42"
assert "%d" % -123 == "-123"
assert "%d" % 0 == "0"
assert "count: %d" % 100 == "count: 100"

# Test %i - integer formatting (same as %d)
assert "%i" % 42 == "42"
assert "%i" % -123 == "-123"
assert "value: %i" % 999 == "value: 999"

# Test %f - float formatting
assert "%f" % 3.14 == "3.140000"
assert "%f" % -2.5 == "-2.500000"
assert "%f" % 0.0 == "0.000000"
assert "%f" % 42 == "42.000000"  # int to float

# Test %r - repr formatting
assert "%r" % "hello" == "'hello'"
assert "%r" % 123 == "123"
assert "%r" % [1, 2, 3] == "[1, 2, 3]"

# Test %% - literal percent
assert "%%" % () == "%"
assert "100%%" % () == "100%"
assert "%%s" % () == "%s"
assert "%d%%" % 50 == "50%"

# Test combined format specifiers
assert "%s is %d years old" % ("Bob", 25) == "Bob is 25 years old"
assert "%s: %f" % ("pi", 3.14159) == "pi: 3.141590"
assert "%d + %d = %d" % (1, 2, 3) == "1 + 2 = 3"
assert "Hello %s! You have %d messages." % ("User", 5) == "Hello User! You have 5 messages."

# Test single value (not tuple)
assert "value: %s" % "test" == "value: test"
assert "number: %d" % 42 == "number: 42"

# Test empty string
assert "" % () == ""

# Test no format specifiers
assert "hello world" % () == "hello world"
