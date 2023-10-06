from time import localtime, time

class Date:
    def __init__(self, year: int, month: int, day: int):
        self.year, self.month, self.day = year, month, day

    @classmethod
    def today(cls):
        t = localtime()
        return cls(t.tm_year, t.tm_mon, t.tm_mday)

    def __str__(self):
        return f"{self.year}-{self.month:02}-{self.day:02}"

    def __repr__(self):
        return f"Date({self.year}, {self.month}, {self.day})"

    def __eq__(self, other: 'Date') -> bool:
        return (self.year, self.month, self.day) == (other.year, other.month, other.day)

    def __lt__(self, other: 'Date') -> bool:
        return (self.year, self.month, self.day) < (other.year, other.month, other.day)

    def __le__(self, other: 'Date') -> bool:
        return self < other or self == other

    def __gt__(self, other: 'Date') -> bool:
        return not (self < other or self == other)

    def __ge__(self, other: 'Date') -> bool:
        return not (self < other)

    def __add__(self, timedelta) -> 'Date':
        # Implement the addition logic here
        pass

    def __sub__(self, timedelta) -> 'Date':
        # Implement the subtraction logic here
        pass

class DateTime(Date):
    def __init__(self, year: int, month: int, day: int, hour: int, minute: int, second: int):
        super().__init__(year, month, day)
        self.hour, self.minute, self.second = hour, minute, second

    @classmethod
    def now(cls):
        t = localtime()
        return cls(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)

    def __str__(self):
        date_str = super().__str__()
        return f"{date_str} {self.hour:02}:{self.minute:02}:{self.second:02}"

    def __repr__(self):
        return f"DateTime({self.year}, {self.month}, {self.day}, {self.hour}, {self.minute}, {self.second})"

    def __eq__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) == \
               (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __lt__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) < \
               (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __le__(self, other) -> bool:
        return self < other or self == other

    def __gt__(self, other) -> bool:
        return not (self < other or self == other)

    def __ge__(self, other) -> bool:
        return not (self < other)

    def __add__(self, timedelta) -> 'DateTime':
        # Implement the addition logic here
        pass

    def __sub__(self, timedelta) -> 'DateTime':
        # Implement the subtraction logic here
        pass

    def timestamp(self) -> float:
        return time()

class TimeDelta:
    def __init__(self, days, seconds):
        self.days, self.seconds = days, seconds

    def __repr__(self) -> str:
        return f"TimeDelta({self.days}, {self.seconds})"

    def __eq__(self, other: 'TimeDelta') -> bool:
        return (self.days, self.seconds) == (other.days, other.seconds)

    def __lt__(self, other: 'TimeDelta') -> bool:
        return (self.days, self.seconds) < (other.days, other.seconds)

    def __le__(self, other: 'TimeDelta') -> bool:
        return self < other or self == other

    def __gt__(self, other: 'TimeDelta') -> bool:
        return not (self < other or self == other)

    def __ge__(self, other: 'TimeDelta') -> bool:
        return not (self < other)

class TimeZone:
    def __init__(self, delta: TimeDelta):
        self.delta = delta
