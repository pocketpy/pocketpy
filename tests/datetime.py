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
import time
import math
import sys

def _get_class_module(self):
    module_name = self.__name__
    if module_name == 'datetime':
        return 'datetime'
    else:
        return module_name

MINYEAR = 1

MAXYEAR = 9999

MONTH_DAYS = [-1, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
_DAYS_BEFORE_MONTH = [-1]  # -1 is a placeholder for indexing purposes.


def is_leap(year):
    return (year % 4 == 0 and year % 100 != 0) or year % 400 == 0

def _is_ascii_digit(c):
    return c in "0123456789"

def _days_before_year(year):
    "year -> number of days before January 1st of year."
    y = year - 1
    return y*365 + y//4 - y//100 + y//400

def _days_in_month(year, month):
    "year, month -> number of days in that month in that year."
    assert 1 <= month <= 12, month
    if month == 2 and is_leap(year):
        return 29
    return MONTH_DAYS[month]

def _days_before_month(year, month):
    "year, month -> number of days in year preceding first day of month."
    assert 1 <= month <= 12, 'month must be in 1..12'
    return _DAYS_BEFORE_MONTH[month] + (month > 2 and is_leap(year))

def _ymd2ord(year, month, day):
    "year, month, day -> ordinal, considering 01-Jan-0001 as day 1."
    assert 1 <= month <= 12, 'month must be in 1..12'
    dim = _days_in_month(year, month)
    assert 1 <= day <= dim, ('day must be in 1..%d' % dim)
    return (_days_before_year(year) + _days_before_month(year, month) + day)

_DI400Y = _days_before_year(401)    # number of days in 400 years
_DI100Y = _days_before_year(101)    #    "    "   "   " 100   "
_DI4Y   = _days_before_year(5)      #    "    "   "   "   4   "

def _ord2ymd(n):
    "ordinal -> (year, month, day), considering 01-Jan-0001 as day 1."

    # n is a 1-based index, starting at 1-Jan-1.  The pattern of leap years
    # repeats exactly every 400 years.  The basic strategy is to find the
    # closest 400-year boundary at or before n, then work with the offset
    # from that boundary to n.  Life is much clearer if we subtract 1 from
    # n first -- then the values of n at 400-year boundaries are exactly
    # those divisible by _DI400Y:
    #
    #     D  M   Y            n              n-1
    #     -- --- ----        ----------     ----------------
    #     31 Dec -400        -_DI400Y       -_DI400Y -1
    #      1 Jan -399         -_DI400Y +1   -_DI400Y      400-year boundary
    #     ...
    #     30 Dec  000        -1             -2
    #     31 Dec  000         0             -1
    #      1 Jan  001         1              0            400-year boundary
    #      2 Jan  001         2              1
    #      3 Jan  001         3              2
    #     ...
    #     31 Dec  400         _DI400Y        _DI400Y -1
    #      1 Jan  401         _DI400Y +1     _DI400Y      400-year boundary
    n -= 1
    n400, n = divmod(n, _DI400Y)
    year = n400 * 400 + 1   # ..., -399, 1, 401, ...

    # Now n is the (non-negative) offset, in days, from January 1 of year, to
    # the desired date.  Now compute how many 100-year cycles precede n.
    # Note that it's possible for n100 to equal 4!  In that case 4 full
    # 100-year cycles precede the desired day, which implies the desired
    # day is December 31 at the end of a 400-year cycle.
    n100, n = divmod(n, _DI100Y)

    # Now compute how many 4-year cycles precede it.
    n4, n = divmod(n, _DI4Y)

    # And now how many single years.  Again n1 can be 4, and again meaning
    # that the desired day is December 31 at the end of the 4-year cycle.
    n1, n = divmod(n, 365)

    year += n100 * 100 + n4 * 4 + n1
    if n1 == 4 or n100 == 4:
        assert n == 0
        return year-1, 12, 31

    # Now the year is correct, and n is the offset from January 1.  We find
    # the month via an estimate that's either exact or one too large.
    leapyear = n1 == 3 and (n4 != 24 or n100 == 3)
    assert leapyear == is_leap(year)
    month = (n + 50) >> 5
    preceding = _DAYS_BEFORE_MONTH[month] + (month > 2 and leapyear)
    if preceding > n:  # estimate is too large
        month -= 1
        preceding -= MONTH_DAYS[month] + (month == 2 and leapyear)
    n -= preceding
    assert 0 <= n < _days_in_month(year, month)

    # Now the year and month are correct, and n is the offset from the
    # start of that month:  we're done!
    return year, month, n+1

def _format_time(hh, mm, ss, us, timespec='auto'):
    specs = {
        'hours': '{:02d}',
        'minutes': '{:02d}:{:02d}',
        'seconds': '{:02d}:{:02d}:{:02d}',
        'milliseconds': '{:02d}:{:02d}:{:02d}.{:03d}',
        'microseconds': '{:02d}:{:02d}:{:02d}.{:06d}'
    }

    if timespec == 'auto':
        # Skip trailing microseconds when us==0.
        timespec = 'microseconds' if us else 'seconds'
    elif timespec == 'milliseconds':
        us //= 1000
    else:
        try:
            fmt = specs[timespec]
        except KeyError:
            raise ValueError('Unknown timespec value')
        return fmt.format(hh, mm, ss, us)


def _format_offset(off):
    s = ''
    if off is not None:
        if off.days < 0:
            sign = "-"
            off = -off
        else:
            sign = "+"
        hh, mm = divmod(off, timedelta(hours=1))
        mm, ss = divmod(mm, timedelta(minutes=1))
        s += "%s%02d:%02d" % (sign, hh, mm)
        if ss or ss.microseconds:
            s += ":%02d" % ss.seconds

            if ss.microseconds:
                s += '.%06d' % ss.microseconds
    return s

def _check_utc_offset(name, offset):
    assert name in ("utcoffset", "dst")
    if offset is None:
        return
    if not isinstance(offset, timedelta):
        raise TypeError("tzinfo must return None TypeError")
    if not -timedelta(1) < offset < timedelta(1):
        raise ValueError("timedelta valueError")


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

    def _getstate(self):
        return (self._days, self._seconds, self._microseconds)

    def _cmp(self, a, b):
        if a[0] > b[0]:
            return 1
        if a[0] < b[0]:
            return -1
        if a[1] > b[1]:
            return 1
        if a[1] < b[1]:
            return -1
        if a[2] > b[2]:
            return 1
        if a[2] < b[2]:
            return -1
        return 0

    def __cmp__(self, other):
        assert isinstance(other, timedelta)
        return self._cmp(self._getstate(), other._getstate())

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
    #返回当地时间相对于UTC的偏移量，以UTC以东分钟为单位
    def utcoffset(self, dt):
        raise NotImplementedError("tzinfo subclass must override utcoffset()")

    #返回夏令时（DST）调整，以UTC以东的分钟为单位；如果夏令时信息未知，则返回None
    def dst(self, dt):
        raise NotImplementedError("tzinfo subclass must override dst()")

    #将对应于 datetime 对象 dt 的时区名称作为字符串返回
    def tzname(self, dt):
        raise NotImplementedError("tzinfo subclass must override tzname()")

    def fromutc(self, dt):
        # raise ValueError error if dt.tzinfo is not self
        if not isinstance(dt, datetime):
            raise TypeError("fromutc() requires a datetime argument")
        if dt.tzinfo is not self:
            raise ValueError("dt.tzinfo is not self")
        dtoff = dt.utcoffset()
        # raise ValueError if dtoff is None or dtdst is None
        if dtoff is None:
            raise ValueError("fromutc() requires a non-None utcoffset() ")
        dtdst = dt.dst()
        delta = dtoff - dtdst  # this is self's standard offset
        if delta:
            dt += delta  # convert to standard local time
            dtdst = dt.dst()
            # raise ValueError if dtdst is None
            if dtdst is None:
                raise ValueError("fromutc(): dt.dst gave inconsistent ")
        if dtdst:
            return dt + dtdst
        else:
            return dt

def _check_tzinfo_arg(tz):
    if tz is not None and not isinstance(tz, tzinfo):
        raise TypeError("tzinfo argument must be None or of a tzinfo subclass")

class timezone(tzinfo):
    _Omitted = object()

    def __init__(self, offset, name):
        if not isinstance(offset, timedelta):
            raise TypeError("offset must be a timedelta")
        if name is self._Omitted:
            if not offset:
                return self.utc
            name = None
        elif name != None and not isinstance(name, str):
            raise TypeError("name must be a string")
        if not self._minoffset.second <= offset.second <= self._maxoffset.second:
            raise ValueError("offset must be a timedelta strictly between -timedelta(hours=24) and timedelta(hours=24).")
        self._offset = offset
        self._name = name
        #self.utc = timezone(timedelta(0))



    def __str__(self):
        return self.tzname(None)

    def utcoffset(self, dt):
        if isinstance(dt, datetime) or dt is None:
            return self._offset
        raise TypeError("utcoffset() argument must be a datetime instance or None")

    def tzname(self, dt):
        if isinstance(dt, datetime) or dt is None:
            if self._name is None:
                return self._name_from_offset(self._offset)
            return self._name
        raise TypeError("tzname() argument must be a datetime instance or None")

    def dst(self, dt):
        if isinstance(dt, datetime) or dt is None:
            return None
        raise TypeError("dst() argument must be a datetime instance or None")

    def fromutc(self, dt):
        if isinstance(dt, datetime):
            if dt.tzinfo is not self:
                raise ValueError("fromutc: dt.tzinfo is not self")
            return dt + self._offset
        raise TypeError("fromutc() argument must be a datetime instance or None")

    _maxoffset = timedelta(hours=24, microseconds=-1)
    _minoffset = -(timedelta(hours=24, microseconds=-1))
'''
TODO:
- 运算符重载
- hash()
- cmp()
'''


def check_time_arg(hour=0, minute=0,second=0, microsecond=0, fold=0):
    if hour < 0 or hour >= 24 or minute < 0 or minute >= 60 or second < 0 or second >= 60 or microsecond < 0 or microsecond >= 1000000 or fold not in [0, 1]:
        raise ValueError('time() argument error, eg: 0 <= hour < 24, please to check')

class time:
    def __init__(self, hour=0, minute=0, second=0, microsecond=0, tzinfo=None, fold=0):
        _check_tzinfo_arg(tzinfo)
        check_time_arg(hour, minute, second, microsecond, fold)

        self._hour = hour
        self._minute = minute
        self._second = second
        self._microsecond = microsecond
        self._tzinfo = tzinfo
        self._hashcode = -1
        self._fold = fold

    def replace(self, hour=None, minute=None, second=None, microsecond=None, tzinfo=True, fold=None):

        if hour is None:
            hour = self.hour
        if minute is None:
            minute = self.minute
        if second is None:
            second = self.second
        if microsecond is None:
            microsecond = self.microsecond
        if tzinfo is True:
            tzinfo = self.tzinfo
        if fold is None:
            fold = self._fold

        if hour < 0 or hour >= 24 or minute < 0 or minute >= 60 or second < 0 or second >= 60 or microsecond < 0 or microsecond >= 1000000 or fold not in [0, 1]:
            raise ValueError('time() argument error, eg: 0 <= hour < 24, please to check')


        return type(self)(hour, minute, second, microsecond, tzinfo, fold=fold)

    def utcoffset(self):
        """Return the timezone offset as timedelta, positive east of UTC
         (negative west of UTC)."""
        if self._tzinfo is None:
            return None
        offset = self._tzinfo.utcoffset(None)
        _check_utc_offset("utcoffset", offset)
        return offset

    def _tzstr(self):
        """Return formatted timezone offset (+xx:xx) or an empty string."""
        off = self.utcoffset()
        return _format_offset(off)

    def isoformat(self, timespec='auto'):
        """Return the time formatted according to ISO.
        The full format is 'HH:MM:SS.mmmmmm+zz:zz'. By default, the fractional
        part is omitted if self.microsecond == 0.
        The optional argument timespec specifies the number of additional
        terms of the time to include. Valid options are 'auto', 'hours',
        'minutes', 'seconds', 'milliseconds' and 'microseconds'.
        """
        s = _format_time(self._hour, self._minute, self._second,
                         self._microsecond, timespec)
        tz = self._tzstr()
        if tz:
            s += tz
        return s

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
            raise ValueError('day must be in [1, ' +MONTH_DAYS[month] + ']')

        self.year = year
        self.month = month
        self.day = day

    def fromtimestamp(self, t):
        y, m, d, hh, mm, ss, weekday, jday, dst = time.localtime(t)
        return date(y, m, d)

    def today(self):
        t = time.time()
        return self.fromtimestamp(t)

    def fromisoformat(date_string):
        """Construct a date from a string in ISO 8601 format."""
        if not isinstance(date_string, str):
            raise TypeError('fromisoformat: argument must be str')

        if len(date_string) not in (7, 8, 10):
            raise ValueError(f'Invalid isoformat string: {date_string}')

        try:
            return datetime(*_parse_isoformat_date(date_string))
        except Exception:
            raise ValueError(f'Invalid isoformat string: {date_string}')

    '''
    def __repr__(self):
        """Convert to formal string, for repr().

        >>> dt = datetime(2010, 1, 1)
        >>> repr(dt)
        'datetime.datetime(2010, 1, 1, 0, 0)'

        >>> dt = datetime(2010, 1, 1, tzinfo=timezone.utc)
        >>> repr(dt)
        'datetime.datetime(2010, 1, 1, 0, 0, tzinfo=datetime.timezone.utc)'
        """
        return "%s.%s(%d, %d, %d)" % (_get_class_module(self),
                                      self.__class__.__qualname__,
                                      self._year,
                                      self._month,
                                      self._day)
    '''
def _find_isoformat_datetime_separator(dtstr):
    # See the comment in _datetimemodule.c:_find_isoformat_datetime_separator
    len_dtstr = len(dtstr)
    if len_dtstr == 7:
        return 7

    assert len_dtstr > 7
    date_separator = "-"
    week_indicator = "W"

    if dtstr[4] == date_separator:
        if dtstr[5] == week_indicator:
            if len_dtstr < 8:
                raise ValueError("Invalid ISO string")
            if len_dtstr > 8 and dtstr[8] == date_separator:
                if len_dtstr == 9:
                    raise ValueError("Invalid ISO string")
                if len_dtstr > 10 and _is_ascii_digit(dtstr[10]):
                    # This is as far as we need to resolve the ambiguity for
                    # the moment - if we have YYYY-Www-##, the separator is
                    # either a hyphen at 8 or a number at 10.
                    #
                    # We'll assume it's a hyphen at 8 because it's way more
                    # likely that someone will use a hyphen as a separator than
                    # a number, but at this point it's really best effort
                    # because this is an extension of the spec anyway.
                    # TODO(pganssle): Document this
                    return 8
                return 10
            else:
                # YYYY-Www (8)
                return 8
        else:
            # YYYY-MM-DD (10)
            return 10
    else:
        if dtstr[4] == week_indicator:
            # YYYYWww (7) or YYYYWwwd (8)
            idx = 7
            while idx < len_dtstr:
                if not _is_ascii_digit(dtstr[idx]):
                    break
                idx += 1

            if idx < 9:
                return idx

            if idx % 2 == 0:
                # If the index of the last number is even, it's YYYYWwwd
                return 7
            else:
                return 8
        else:
            # YYYYMMDD (8)
            return 8

def _isoweek1monday(year):
    # Helper to calculate the day number of the Monday starting week 1
    # XXX This could be done more efficiently
    THURSDAY = 3
    firstday = _ymd2ord(year, 1, 1)
    firstweekday = (firstday + 6) % 7  # See weekday() above
    week1monday = firstday - firstweekday
    if firstweekday > THURSDAY:
        week1monday += 7
    return week1monday

def _isoweek_to_gregorian(year, week, day):
    # Year is bounded this way because 9999-12-31 is (9999, 52, 5)
    if not MINYEAR <= year <= MAXYEAR:
        raise ValueError(f"Year is out of range: {year}")

    if not 0 < week < 53:
        out_of_range = True

        if week == 53:
            # ISO years have 53 weeks in them on years starting with a
            # Thursday and leap years starting on a Wednesday
            first_weekday = _ymd2ord(year, 1, 1) % 7
            if (first_weekday == 4 or (first_weekday == 3 and is_leap(year))):
                out_of_range = False

        if out_of_range:
            raise ValueError(f"Invalid week: {week}")

    if not 0 < day < 8:
        raise ValueError(f"Invalid weekday: {day} (range is [1, 7])")

    # Now compute the offset from (Y, 1, 1) in days:
    day_offset = (week - 1) * 7 + (day - 1)

    # Calculate the ordinal day for monday, week 1
    day_1 = _isoweek1monday(year)
    ord_day = day_1 + day_offset

    return _ord2ymd(ord_day)

def _parse_isoformat_date(dtstr):
    # It is assumed that this is an ASCII-only string of lengths 7, 8 or 10,
    # see the comment on Modules/_datetimemodule.c:_find_isoformat_datetime_separator
    assert len(dtstr) in (7, 8, 10)
    year = int(dtstr[0:4])
    has_sep = dtstr[4] == '-'

    pos = 4 + has_sep
    if dtstr[pos:pos + 1] == "W":
        # YYYY-?Www-?D?
        pos += 1
        weekno = int(dtstr[pos:pos + 2])
        pos += 2

        dayno = 1
        if len(dtstr) > pos:
            if (dtstr[pos:pos + 1] == '-') != has_sep:
                raise ValueError("Inconsistent use of dash separator")

            pos += has_sep

            dayno = int(dtstr[pos:pos + 1])

        return list(_isoweek_to_gregorian(year, weekno, dayno))
    else:
        month = int(dtstr[pos:pos + 2])
        pos += 2
        if (dtstr[pos:pos + 1] == "-") != has_sep:
            raise ValueError("Inconsistent use of dash separator")

        pos += has_sep
        day = int(dtstr[pos:pos + 2])

        return [year, month, day]

_FRACTION_CORRECTION = [100000, 10000, 1000, 100, 10]

def _parse_hh_mm_ss_ff(tstr):
    # Parses things of the form HH[:?MM[:?SS[{.,}fff[fff]]]]
    len_str = len(tstr)

    time_comps = [0, 0, 0, 0]
    pos = 0
    for comp in range(0, 3):
        if (len_str - pos) < 2:
            raise ValueError("Incomplete time component")

        time_comps[comp] = int(tstr[pos:pos+2])

        pos += 2
        next_char = tstr[pos:pos+1]

        if comp == 0:
            has_sep = next_char == ':'

        if not next_char or comp >= 2:
            break

        if has_sep and next_char != ':':
            raise ValueError("Invalid time separator: %c" % next_char)

        pos += has_sep

    if pos < len_str:
        if tstr[pos] not in '.,':
            raise ValueError("Invalid microsecond component")
        else:
            pos += 1

            len_remainder = len_str - pos

            if len_remainder >= 6:
                to_parse = 6
            else:
                to_parse = len_remainder

            time_comps[3] = int(tstr[pos:(pos+to_parse)])
            if to_parse < 6:
                time_comps[3] *= _FRACTION_CORRECTION[to_parse-1]
            if (len_remainder > to_parse and not all(map(_is_ascii_digit, tstr[(pos+to_parse):]))):
                raise ValueError("Non-digit values in unparsed fraction")

    return time_comps

def _parse_isoformat_time(tstr):
    # Format supported is HH[:MM[:SS[.fff[fff]]]][+HH:MM[:SS[.ffffff]]]
    len_str = len(tstr)
    if len_str < 2:
        raise ValueError("Isoformat time too short")

    # This is equivalent to re.search('[+-Z]', tstr), but faster
    tz_pos = (tstr.find('-') + 1 or tstr.find('+') + 1 or tstr.find('Z') + 1)
    timestr = tstr[:tz_pos-1] if tz_pos > 0 else tstr

    time_comps = _parse_hh_mm_ss_ff(timestr)

    tzi = None
    if tz_pos == len_str and tstr[-1] == 'Z':
        tzi = timezone.utc
    elif tz_pos > 0:
        tzstr = tstr[tz_pos:]

        # Valid time zone strings are:
        # HH                  len: 2
        # HHMM                len: 4
        # HH:MM               len: 5
        # HHMMSS              len: 6
        # HHMMSS.f+           len: 7+
        # HH:MM:SS            len: 8
        # HH:MM:SS.f+         len: 10+

        if len(tzstr) in (0, 1, 3):
            raise ValueError("Malformed time zone string")

        tz_comps = _parse_hh_mm_ss_ff(tzstr)

        if all([x == 0 for x in tz_comps]):
            tzi = timezone.utc
        else:
            tzsign = -1 if tstr[tz_pos - 1] == '-' else 1

            td = timedelta(hours=tz_comps[0], minutes=tz_comps[1],
                           seconds=tz_comps[2], microseconds=tz_comps[3])

            tzi = timezone(tzsign * td)

    time_comps.append(tzi)

    return time_comps


class datetime(date):
    def __init__(self, year, month, day, hour=0, minute=0, second=0, microsecond=0, tzinfo=None, fold=0):
        check_time_arg(hour, minute, second, microsecond, fold)
        self._year = year
        self._month = month
        self._day = day
        self._hour = hour
        self._minute = minute
        self._second = second
        self._microsecond = microsecond
        self._tzinfo = tzinfo
        self._hashcode = -1
        self._fold = fold

    def hour(self):
        """hour (0-23)"""
        return self._hour

    def minute(self):
        """minute (0-59)"""
        return self._minute

    def second(self):
        """second (0-59)"""
        return self._second

    def microsecond(self):
        """microsecond (0-999999)"""
        return self._microsecond

    def tzinfo(self):
        """timezone info object"""
        return self._tzinfo

    def fold(self):
        return self._fold

    def _fromtimestamp(self, t, utc, tz):
        """Construct a datetime from a POSIX timestamp (like time.time()).

        A timezone info object may be passed in as well.
        """
        frac, t = math.modf(t)
        us = round(frac * 1000000)
        if us >= 1000000:
            t += 1
            us -= 1000000
        elif us < 0:
            t -= 1
            us += 1000000

        converter = time.gmtime if utc else time.localtime
        y, m, d, hh, mm, ss, weekday, jday, dst = converter(t)
        ss = min(ss, 59)  # clamp out leap seconds if the platform has them
        result = self(y, m, d, hh, mm, ss, us, tz)
        if tz is None and not utc:
            # As of version 2015f max fold in IANA database is
            # 23 hours at 1969-09-30 13:00:00 in Kwajalein.
            # Let's probe 24 hours in the past to detect a transition:
            max_fold_seconds = 24 * 3600

            # On Windows localtime_s throws an OSError for negative values,
            # thus we can't perform fold detection for values of time less
            # than the max time fold. See comments in _datetimemodule's
            # version of this method for more details.
            if t < max_fold_seconds and sys.platform.startswith("win"):
                return result

            y, m, d, hh, mm, ss = converter(t - max_fold_seconds)[:6]
            probe1 = self(y, m, d, hh, mm, ss, us, tz)
            trans = result - probe1 - timedelta(0, max_fold_seconds)
            if trans.days < 0:
                y, m, d, hh, mm, ss = converter(t + trans // timedelta(0, 1))[:6]
                probe2 = self(y, m, d, hh, mm, ss, us, tz)
                if probe2 == result:
                    result._fold = 1
        elif tz is not None:
            result = tz.fromutc(result)
        return result

    def fromtimestamp(self, timestamp, tz=None):
        _check_tzinfo_arg(tz)
        return self._fromtimestamp(timestamp, tz is not None, tz)

    def now(self, tz=None):
        t = time.time()
        return self.fromtimestamp(t, tz)

    def date(self):
        "Return the date part."
        return date(self._year, self._month, self._day)

    def time(self):
        "Return the time part, with tzinfo None."
        return time(self.hour, self.minute, self.second, self.microsecond, fold=self.fold)

    def timetz(self):
        "Return the time part, with same tzinfo."
        return time(self.hour, self.minute, self.second, self.microsecond,
                    self._tzinfo, fold=self.fold)

    def utcoffset(self):
        """Return the timezone offset as timedelta, positive east of UTC
         (negative west of UTC)."""
        if self._tzinfo is None:
            return None
        offset = self._tzinfo.utcoffset(None)
        _check_utc_offset("utcoffset", offset)
        return offset

    def isoformat(self, sep='T', timespec='auto'):
        """Return the time formatted according to ISO.

        The full format looks like 'YYYY-MM-DD HH:MM:SS.mmmmmm'.
        By default, the fractional part is omitted if self.microsecond == 0.

        If self.tzinfo is not None, the UTC offset is also attached, giving
        giving a full format of 'YYYY-MM-DD HH:MM:SS.mmmmmm+HH:MM'.

        Optional argument sep specifies the separator between date and
        time, default 'T'.

        The optional argument timespec specifies the number of additional
        terms of the time to include. Valid options are 'auto', 'hours',
        'minutes', 'seconds', 'milliseconds' and 'microseconds'.
        """
        s = ("%04d-%02d-%02d%c" % (self._year, self._month, self._day, sep) + _format_time(self._hour, self._minute, self._second, self._microsecond, timespec))

        off = self.utcoffset()
        tz = _format_offset(off)
        if tz:
            s += tz

        return s
    def __repr__(self):
        """Convert to formal string, for repr().

        >>> dt = datetime(2010, 1, 1)
        >>> repr(dt)
        'datetime.datetime(2010, 1, 1, 0, 0)'

        >>> dt = datetime(2010, 1, 1, tzinfo=timezone.utc)
        >>> repr(dt)
        'datetime.datetime(2010, 1, 1, 0, 0, tzinfo=datetime.timezone.utc)'
        """
        return "datetime.datetime(%d, %d, %d)" % (self._year,self._month, self._day)
    '''
    def __str__(self):
        "Convert to string, for str()."
        return self.isoformat(sep=' ')
    '''
