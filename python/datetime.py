from time import time, localtime

class timedelta:
    def __init__(self, days=0, seconds=0):
        self.days = days
        self.seconds = seconds

    def __repr__(self) -> str:
        return f"datetime.timedelta(days={self.days}, seconds={self.seconds})"

    def __eq__(self, other: 'timedelta') -> bool:
        return self.days == other.days and self.seconds == other.seconds

    def __lt__(self, other: 'timedelta') -> bool:
        return (self.days, self.seconds) < (other.days, other.seconds)

    def __le__(self, other: 'timedelta') -> bool:
        return (self.days, self.seconds) <= (other.days, other.seconds)

    def __gt__(self, other: 'timedelta') -> bool:
        return (self.days, self.seconds) > (other.days, other.seconds)

    def __ge__(self, other: 'timedelta') -> bool:
        return (self.days, self.seconds) >= (other.days, other.seconds)


class timezone:
    def __init__(self, delta: timedelta):
        self.delta = delta

    def __repr__(self) -> str:
        return f"datetime.timezone({self.delta})"


class date:
    @staticmethod
    def today() -> 'date':
        t = localtime()
        return date(t.tm_year, t.tm_mon, t.tm_mday)

    def __init__(self, year, month=None, day=None):
        self.year = year
        self.month = month
        self.day = day

    @property
    def year(self) -> int:
        return self._year

    @year.setter
    def year(self, value: int):
        self._year = value

    @property
    def month(self) -> int:
        return self._month

    @month.setter
    def month(self, value: int):
        self._month = value

    @property
    def day(self) -> int:
        return self._day

    @day.setter
    def day(self, value: int):
        self._day = value

    def __repr__(self) -> str:
        return f"datetime.date({self.year}, {self.month}, {self.day})"

    def __eq__(self, other: 'date') -> bool:
        return (self.year, self.month, self.day) == (other.year, other.month, other.day)

    def __lt__(self, other: 'date') -> bool:
        return (self.year, self.month, self.day) < (other.year, other.month, other.day)

    def __le__(self, other: 'date') -> bool:
        return (self.year, self.month, self.day) <= (other.year, other.month, other.day)

    def __gt__(self, other: 'date') -> bool:
        return (self.year, self.month, self.day) > (other.year, other.month, other.day)

    def __ge__(self, other: 'date') -> bool:
        return (self.year, self.month, self.day) >= (other.year, other.month, other.day)

    def __add__(self, other: timedelta) -> 'date':
        # Add the timedelta to the current date and return a new date
        pass

    def __sub__(self, other: timedelta) -> 'date':
        # Subtract the timedelta from the current date and return a new date
        pass


class datetime(date):
    @staticmethod
    def now() -> 'datetime':
        t = localtime()
        return datetime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)

    def __init__(self, year, month=None, day=None, hour=None, minute=None, second=None, tzinfo=None):
        super().__init__(year, month, day)
        self.hour = hour
        self.minute = minute
        self.second = second
        self.tzinfo = tzinfo

    @property
    def hour(self) -> int:
        return self._hour

    @hour.setter
    def hour(self, value: int):
        self._hour = value

    @property
    def minute(self) -> int:
        return self._minute

    @minute.setter
    def minute(self, value: int):
        self._minute = value

    @property
    def second(self) -> int:
        return self._second

    @second.setter
    def second(self, value: int):
        self._second = value

    @property
    def tzinfo(self) -> timezone:
        return self._tzinfo

    @tzinfo.setter
    def tzinfo(self, value: timezone):
        self._tzinfo = value

    def __repr__(self) -> str:
        return f"datetime.datetime({self.year}, {self.month}, {self.day}, {self.hour}, {self.minute}, {self.second}, tzinfo={self.tzinfo})"

    def __eq__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second, self.tzinfo) == \
               (other.year, other.month, other.day, other.hour, other.minute, other.second, other.tzinfo)

    def __lt__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second, self.tzinfo) < \
               (other.year, other.month, other.day, other.hour, other.minute, other.second, other.tzinfo)

    def __le__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second, self.tzinfo) <= \
               (other.year, other.month, other.day, other.hour, other.minute, other.second, other.tzinfo)

    def __gt__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second, self.tzinfo) > \
               (other.year, other.month, other.day, other.hour, other.minute, other.second, other.tzinfo)

    def __ge__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second, self.tzinfo) >= \
               (other.year, other.month, other.day, other.hour, other.minute, other.second, other.tzinfo)

    def __add__(self, other: timedelta) -> 'datetime':
        # Add the timedelta to the current datetime and return a new datetime
        pass

    def __sub__(self, other: timedelta) -> 'datetime':
        # Subtract the timedelta from the current datetime and return a new datetime
        pass

    def timestamp(self) -> float:
        # Return the timestamp of the current datetime
        pass
