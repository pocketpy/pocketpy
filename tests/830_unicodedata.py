from unicodedata import east_asian_width

# full width
assert east_asian_width("Ａ") == "F"
# half width
assert east_asian_width("ｻ") == "H"
# narrow
assert east_asian_width("a") == "Na"
# wide
assert east_asian_width("测") == "W"
assert east_asian_width("🥕") == "W"
assert east_asian_width("。") == "W"
# ambiguous
assert east_asian_width("°") == "A"
# neutral
assert east_asian_width("\n") == "N"

