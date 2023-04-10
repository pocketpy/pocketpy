'''
object
    timedelta
    tzinfo
        timezone
    time
    date
        datetime
'''
import builtins

MINYEAR = 1

MAXYEAR = 9999

MONTH_DAYS = [-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]

def is_leap(year):
    return (year % 4 == 0 and year % 100 != 0) or year % 400 == 0

class timedelta:
    def __init__(self, days=0, seconds=0, microseconds=0,
                 milliseconds=0, minutes=0, hours=0, weeks=0):
        days += 7 * weeks
        seconds += 60 * minutes + 3600 * hours
        microseconds += 1000 * milliseconds #微秒 = 1000*毫秒

        if microseconds >= 1000000:
            seconds += microseconds // 1000000
            microseconds %= 1000000
        if seconds >= 3600 * 24:
            days += seconds // (3600 * 24)
            seconds %= (3600 * 24)
        if builtins.abs(days) > 999999999:
            raise OverflowError('timedelta # of days is Overflow')

        self.days = days
        self.second = seconds
        self.microseconds = microseconds

    def __str__(self):
        s = 'datetime.timedelta(days=' + str(self.days) + ', '
        s += 'seconds=' + str(self.second) + ', '
        s += 'microseconds=' + str(self.microseconds) + ')'
        return s

    def __add__(self, other):
        if isinstance(other, timedelta):
            return timedelta(days = self.days + other.days,
                             seconds = self.second + other.second,
                             microseconds = self.microseconds + other.microseconds)
        return NotImplemented

    def __sub__(self, other):
        if isinstance(other, timedelta):
            return timedelta(days = self.days - other.days,
                             seconds = self.second - other.second,
                             microseconds = self.microseconds - other.microseconds)
        return NotImplemented
    def __neg__(self):
        return timedelta(days = -self.days,
                         seconds = -self.second,
                         microseconds= -self.microseconds)
    def __pos__(self):
        return self

    def __abs__(self):
        if self._days < 0:
            return -self
        else:
            return self

class tzinfo:
    pass

class timezone(tzinfo):
    pass

class time:
    pass

class date:
    def __init__(self, year=None, month=None, day=None):
        if year < MINYEAR or year > MAXYEAR:
            raise ValueError('year must be in [1, 9999]')
        if month < 1 or month > 12:
            raise ValueError('month must be in [1, 12]')
        _max_day = MONTH_DAYS[month]
        if month == 2 and is_leap(year):
            _max_day += 1
        if day < 1 or day > _max_day:
            raise ValueError('day must be in [1, ', MONTH_DAYS[month], ']')

        self.year = year
        self.month = month
        self.day = day

    def today(self):
        pass

class datetime(date):
    pass
