---
icon: package
label: datetime
---

# datetime

Basic local dates and times, backed by [time.localtime()](time.md).

| API | Purpose |
| --- | --- |
| `date(year, month, day)` | Store a calendar date. |
| `date.today()` | Current local date. |
| `datetime(year, month, day, hour, minute, second)` | Store date and time; all six arguments are required. |
| `datetime.now()` | Current local date and time. |
| `value.date()` | Extract a `date` from a `datetime`. |
| `timedelta(days=0, seconds=0)` | Store days and seconds. |

```python
from datetime import date, datetime

deadline = date(2026, 10, 2)
meeting = datetime(2026, 10, 2, 9, 30, 0)
assert meeting.date() == deadline
assert str(meeting) == '2026-10-02 09:30:00'
assert date(2026, 10, 1) < deadline
```

Dates and datetimes support field access, string representations, and
comparisons. This module does not implement timezone objects, parsing with
`strptime`, formatting with `strftime`, microseconds, or date arithmetic.
`timedelta` stores its two fields without CPython's normalization and
arithmetic behavior. Validate calendar inputs in your application; the date
constructor does not perform full calendar validation.

Implementation: [datetime.py](https://github.com/pocketpy/pocketpy/blob/main/python/datetime.py).
