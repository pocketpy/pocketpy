---
icon: package
label: csv
---

### `csv.reader(csvfile: list[str]) -> list[list]`

Parse a CSV file into a list of lists.

### `csv.DictReader(csvfile: list[str]) -> list[dict]`

Parse a CSV file into a list of dictionaries.

## Example

```python
import csv

data = """a,b,c
1,2,3
"""

print(csv.reader(data.splitlines()))
# [
#    ['a', 'b', 'c'],
#    ['1', '2', '3']
# ]

print(csv.DictReader(data.splitlines()))
# [
#    {'a': '1', 'b': '2', 'c': '3'}
# ]
```