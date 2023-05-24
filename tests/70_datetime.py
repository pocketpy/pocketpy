from datetime import timedelta, timezone, datetime, time

delta = timedelta(
    days=50,
    seconds=27,
    microseconds=10,
    milliseconds=29000,
    minutes=5,
    hours=8,
    weeks=2
)

assert str(delta) == 'datetime.timedelta(days=64, seconds=29156, microseconds=10)'

delta1 = timedelta(seconds=57)
delta2 = timedelta(hours=25, seconds=2)
assert True == (delta2 != delta1)

timezone.utc = timezone(timedelta(0))
UTC = timezone.utc
_EPOCH = datetime(1970, 1, 1, tzinfo=timezone.utc)


_time_class = time  # so functions w/ args named "time" can get at the class

time.min = time(0, 0, 0)
time.max = time(23, 59, 59, 999999)
time.resolution = timedelta(microseconds=1)


assert '12:34' == str(time(hour=12, minute=34, second=56, microsecond=123456).isoformat(timespec='minutes'))

tm = time(hour=12, minute=34, second=56, microsecond=0)
assert '12:34:56.000000' == str(tm.isoformat(timespec='microseconds'))

assert '12:34:56' == str(tm.isoformat(timespec='auto'))

assert 'datetime.datetime(2011, 11, 4)' == str(datetime.fromisoformat('2011-11-04'))

assert 'datetime.datetime(2011, 11, 4)' == str(datetime.fromisoformat('20111104'))

assert 'datetime.datetime(2011, 11, 12)' == str(datetime.fromisoformat('2011-11-12'))
