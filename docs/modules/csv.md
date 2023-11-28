---
icon: package
label: csv
---

### `csv.reader(csvfile: list[str]) -> list`

Parse a CSV file into a list of lists.


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
```