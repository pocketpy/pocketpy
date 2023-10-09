
from time import localtime


class timedelta:
    def __init__(self, days=0, seconds=0):
        self.days = days
        self.seconds = seconds

    def __repr__(self):
        return f"datetime.timedelta({self.days}, {self.seconds})"

    def check(self, other, class_type):
        if not isinstance(other, class_type):
            raise NotImplementedError("incompatible types / not implemented")

    def __eq__(self, other: 'timedelta') -> bool:
        self.check(other, timedelta)
        return (self.days, self.seconds) == (other.days, other.seconds)

    def __lt__(self, other: 'timedelta') -> bool:
        self.check(other, timedelta)
        return (self.days, self.seconds) < (other.days, other.seconds)

    def __le__(self, other: 'timedelta') -> bool:
        self.check(other, timedelta)
        return (self.days, self.seconds) <= (other.days, other.seconds)

    def __gt__(self, other: 'timedelta') -> bool:
        self.check(other, timedelta)
        return (self.days, self.seconds) > (other.days, other.seconds)

    def __ge__(self, other: 'timedelta') -> bool:
        self.check(other, timedelta)
        return (self.days, self.seconds) >= (other.days, other.seconds)


class timezone:
    def __init__(self, delta: timedelta):
        self.delta = delta

    def __repr__(self):
        return f"datetime.timezone({self.delta})"


class date:
    def __init__(self, year: int, month: int = None, day: int = None):
        self.year = year
        self.month = month
        self.day = day

    @staticmethod
    def today():
        t = localtime()
        return date(t.tm_year, t.tm_mon, t.tm_mday)

    def check(self, other, class_type):
        if not isinstance(other, class_type):
            raise NotImplementedError("incompatible types / not implemented")

    def __eq__(self, other: 'date') -> bool:
        self.check(other, date)
        return (self.year, self.month, self.day) == (other.year, other.month, other.day)

    def __lt__(self, other: 'date') -> bool:
        self.check(other, date)
        return (self.year, self.month, self.day) < (other.year, other.month, other.day)

    def __le__(self, other: 'date') -> bool:
        self.check(other, date)
        return (self.year, self.month, self.day) <= (other.year, other.month, other.day)

    def __gt__(self, other: 'date') -> bool:
        self.check(other, date)
        return (self.year, self.month, self.day) > (other.year, other.month, other.day)

    def __ge__(self, other: 'date') -> bool:
        self.check(other, date)
        return (self.year, self.month, self.day) >= (other.year, other.month, other.day)

    def __str__(self):
        return f"{self.year}-{self.month:02}-{self.day:02}"

    def __repr__(self):
        return f"datetime.date({self.year}, {self.month}, {self.day})"


class datetime(date):
    def __init__(self, year: int, month: int = None, day: int = None, hour: int = None, minute: int = None, second: int = None, tzinfo: timezone = None):
        super().__init__(year, month, day)
        # Validate and set hour, minute, and second
        if hour is not None and not 0 <= hour <= 23:
            raise ValueError("Hour must be between 0 and 23")
        self.hour = hour
        if minute is not None and not 0 <= minute <= 59:
            raise ValueError("Minute must be between 0 and 59")
        self.minute = minute
        if second is not None and not 0 <= second <= 59:
            raise ValueError("Second must be between 0 and 59")
        self.second = second
        self.tzinfo = tzinfo

    @staticmethod
    def now():
        t = localtime()
        return datetime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)

    def check(self, other, class_type):
        if not isinstance(other, class_type):
            raise NotImplementedError("incompatible types / not implemented")

    def __str__(self):
        return f"{self.year}-{self.month:02}-{self.day:02} {self.hour:02}:{self.minute:02}:{self.second:02}"

    def __repr__(self):
        return f"datetime.datetime({self.year}, {self.month}, {self.day}, {self.hour}, {self.minute}, {self.second})"

    def __eq__(self, other) -> bool:
        self.check(other, datetime)
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) ==\
            (other.year, other.month, other.day,
             other.hour, other.minute, other.second)

    def __lt__(self, other) -> bool:
        self.check(other, datetime)
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) <\
            (other.year, other.month, other.day,
             other.hour, other.minute, other.second)

    def __le__(self, other) -> bool:
        self.check(other, datetime)
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) <=\
            (other.year, other.month, other.day,
             other.hour, other.minute, other.second)

    def __gt__(self, other) -> bool:
        self.check(other, datetime)
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) >\
            (other.year, other.month, other.day,
             other.hour, other.minute, other.second)

    def __ge__(self, other) -> bool:
        self.check(other, datetime)
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) >=\
            (other.year, other.month, other.day,
             other.hour, other.minute, other.second)

    def timestamp(self) -> float:
        return self.now()
