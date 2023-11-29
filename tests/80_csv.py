import csv
def test(data: str, expected):
    ret = list(csv.reader(data.splitlines()))
    assert ret==expected, f"Expected {expected}, got {ret}"

test("""a,b,c
1,2,3
""",  [['a', 'b', 'c'], ['1', '2', '3']])

test("""a,b,c
1,2,"3"
""",  [['a', 'b', 'c'], ['1', '2', '3']])

test("""a,b,c
1,2,"3,,"
""",  [['a', 'b', 'c'], ['1', '2', '3,,']])

test("""a,b,c
1,2,'3'
""",  [['a', 'b', 'c'], ['1', '2', '\'3\'']])

test('''a,b,c
1,2,"123"""
''',  [['a', 'b', 'c'], ['1', '2', '123"']])

test("""a,b,c,
1,2,3,
""",  [['a', 'b', 'c', ''], ['1', '2', '3', '']])

test("""a,b ,c,
1,"22""33",3
""",  [['a', 'b ', 'c', ''], ['1', '22"33', '3']])

# newline
test('''a,b,c
1,2,"3,
  4"
5,"a,""
b",7
''',  [['a', 'b', 'c'], ['1', '2', '3,\n  4'], ['5', 'a,"\nb', '7']])

ret = csv.DictReader("""a,b,c
1,2,3
"4",5,6
""".splitlines())

assert list(ret)==[
    {'a': '1', 'b': '2', 'c': '3'},
    {'a': '4', 'b': '5', 'c': '6'},
]
