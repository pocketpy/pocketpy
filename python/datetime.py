from time import localtime

class timedelta:
    def __init__(self, days: int = 0, seconds: int = 0):
        self.days: int = days
        self.seconds: int = seconds

    def __repr__(self) -> str:
        return f"datetime.timedelta(days={self.days}, seconds={self.seconds})"

    def __eq__(self, other: 'timedelta') -> bool:
        if type(other) is not timedelta:
            return NotImplemented
        return (self.days, self.seconds) == (other.days, other.seconds)

    def __lt__(self, other: 'timedelta') -> bool:
        if type(other) is not timedelta:
            return NotImplemented
        return (self.days, self.seconds) < (other.days, other.seconds)

    def __le__(self, other: 'timedelta') -> bool:
        if type(other) is not timedelta:
            return NotImplemented
        return (self.days, self.seconds) <= (other.days, other.seconds)

    def __gt__(self, other: 'timedelta') -> bool:
        if type(other) is not timedelta:
            return NotImplemented
        return (self.days, self.seconds) > (other.days, other.seconds)

    def __ge__(self, other: 'timedelta') -> bool:
        if type(other) is not timedelta:
            return NotImplemented
        return (self.days, self.seconds) >= (other.days, other.seconds)


class date:
    def __init__(self, year: int, month: int, day: int):
        self.year: int = year
        self.month: int = month
        self.day: int = day

    @staticmethod
    def today() -> 'date':
        t = localtime()
        return date(t.tm_year, t.tm_mon, t.tm_mday)

    def __eq__(self, other: 'date') -> bool:
        if type(other) is not date:
            return NotImplemented
        return (self.year, self.month, self.day) == (other.year, other.month, other.day)

    def __lt__(self, other: 'date') -> bool:
        if type(other) is not date:
            return NotImplemented
        return (self.year, self.month, self.day) < (other.year, other.month, other.day)

    def __le__(self, other: 'date') -> bool:
        if type(other) is not date:
            return NotImplemented
        return (self.year, self.month, self.day) <= (other.year, other.month, other.day)

    def __gt__(self, other: 'date') -> bool:
        if type(other) is not date:
            return NotImplemented
        return (self.year, self.month, self.day) > (other.year, other.month, other.day)

    def __ge__(self, other: 'date') -> bool:
        if type(other) is not date:
            return NotImplemented
        return (self.year, self.month, self.day) >= (other.year, other.month, other.day)

    def __str__(self) -> str:
        return f"{self.year}-{self.month:02}-{self.day:02}"

    def __repr__(self) -> str:
        return f"datetime.date({self.year}, {self.month}, {self.day})"


class datetime(date):
    def __init__(self, year: int, month: int, day: int, hour: int, minute: int, second: int):
        super().__init__(year, month, day)
        if not 0 <= hour <= 23:
            raise ValueError("Hour must be between 0 and 23")
        self.hour: int = hour
        if not 0 <= minute <= 59:
            raise ValueError("Minute must be between 0 and 59")
        self.minute: int = minute
        if not 0 <= second <= 59:
            raise ValueError("Second must be between 0 and 59")
        self.second: int = second

    def date(self) -> date:
        return date(self.year, self.month, self.day)

    @staticmethod
    def now() -> 'datetime':
        t = localtime()
        tm_sec = t.tm_sec
        if tm_sec == 60:
            tm_sec = 59
        return datetime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, tm_sec)

    def __str__(self) -> str:
        return f"{self.year}-{self.month:02}-{self.day:02} {self.hour:02}:{self.minute:02}:{self.second:02}"

    def __repr__(self) -> str:
        return f"datetime.datetime({self.year}, {self.month}, {self.day}, {self.hour}, {self.minute}, {self.second})"

    def __eq__(self, other) -> bool:
        if type(other) is not datetime:
            return NotImplemented
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) ==\
            (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __lt__(self, other) -> bool:
        if type(other) is not datetime:
            return NotImplemented
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) <\
            (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __le__(self, other) -> bool:
        if type(other) is not datetime:
            return NotImplemented
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) <=\
            (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __gt__(self, other) -> bool:
        if type(other) is not datetime:
            return NotImplemented
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) >\
            (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __ge__(self, other) -> bool:
        if type(other) is not datetime:
            return NotImplemented
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) >=\
            (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def timestamp(self) -> float:
        raise NotImplementedError("Method 'timestamp' needs to be implemented in subclasses")
