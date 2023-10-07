from time import localtime, time as current_time
from datetime import datetime as dt, timedelta as td #internally using datetime , timedelta for __add__,__sub__

class timedelta:
    def __init__(self, days=0, seconds=0):
        self.days = days
        self.seconds = seconds

    def __repr__(self):
        return f"datetime.timedelta({self.days}, {self.seconds})"

    def __eq__(self, other: 'timedelta') -> bool:
        return (self.days, self.seconds) == (other.days, other.seconds)

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
        self._delta = delta

    def __repr__(self):
        return f"datetime.timezone({self._delta})"

class date:
    def __init__(self, year: int, month: int=None, day: int=None):
        self.year = year
        self.month = month
        self.day = day

    @staticmethod
    def today():
        t = localtime()
        return date(t.tm_year, t.tm_mon, t.tm_mday)
    
    def __eq__(self, other: 'date') -> bool:
        return self.year == other.year and self.month == other.month and self.day == other.day
    
    def __lt__(self, other: 'date') -> bool:
        return (self.year, self.month, self.day) < (other.year, other.month, other.day)
    
    def __le__(self, other: 'date') -> bool:
        return (self.year, self.month, self.day) <= (other.year, other.month, other.day)
    
    def __gt__(self, other: 'date') -> bool:
        return (self.year, self.month, self.day) > (other.year, other.month, other.day)

    def __ge__(self, other: 'date') -> bool:
        return (self.year, self.month, self.day) >= (other.year, other.month, other.day)
    
    def __add__(self, other: 'timedelta') -> 'date':
        new_date=dt(self.year,self.month,self.day)+td(other.days,other.seconds)
        return date(new_date.year,new_date.month,new_date.day)

    def __sub__(self, other: 'timedelta') -> 'date':
        new_date=dt(self.year,self.month,self.day)-td(other.days,other.seconds)
        return date(new_date.year,new_date.month,new_date.day)

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

    def __str__(self):
        return f"{self.year}-{self.month:02}-{self.day:02} {self.hour:02}:{self.minute:02}:{self.second:02}"

    def __repr__(self):
        return f"datetime.datetime({self.year}, {self.month}, {self.day}, {self.hour}, {self.minute}, {self.second})"
    
    def __eq__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) == \
               (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __lt__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) < \
               (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __le__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) <= \
               (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __gt__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) > \
               (other.year, other.month, other.day, other.hour, other.minute, other.second)

    def __ge__(self, other) -> bool:
        return (self.year, self.month, self.day, self.hour, self.minute, self.second) >= \
               (other.year, other.month, other.day, other.hour, other.minute, other.second)
    
    def __add__(self, other: 'timedelta') -> 'datetime':
        new_datetime = dt(self.year, self.month, self.day, self.hour, self.minute, self.second) + td(days=other.days, seconds=other.seconds)
        return datetime(new_datetime.year, new_datetime.month, new_datetime.day, new_datetime.hour, new_datetime.minute, new_datetime.second)
    
    def __sub__(self, other: 'timedelta') -> 'datetime':
        total_seconds = self.second + other.seconds + other.days * 86400
        new_datetime = self - timedelta(seconds=total_seconds)
        return new_datetime
    
    def timestamp(self) -> float:
        delta = self - datetime(1970, 1, 1)
        return delta.total_seconds()


