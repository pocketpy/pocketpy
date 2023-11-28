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
