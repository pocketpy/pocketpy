import datetime


def test_timedelta():
    assert datetime.timedelta(days=1) == datetime.timedelta(days=1)
    assert datetime.timedelta(days=1) != datetime.timedelta(days=2)
    assert datetime.timedelta(days=1, seconds=1) >= datetime.timedelta(days=1)
    assert datetime.timedelta(days=0, seconds=1) <= datetime.timedelta(days=1)
    assert datetime.timedelta(days=1, seconds=1) < datetime.timedelta(days=2)
    assert datetime.timedelta(days=1, seconds=1) > datetime.timedelta(days=0)


def test_date():
    assert datetime.date(2023, 8, 5) == datetime.date(2023, 8, 5)
    assert datetime.date(2023, 8, 5) != datetime.date(2023, 8, 6)
    assert datetime.date(2024, 8, 5) >= datetime.date(2023, 8, 6)
    assert datetime.date(2023, 8, 5) <= datetime.date(2023, 8, 6)
    assert datetime.date(2024, 8, 5) > datetime.date(2023, 8, 6)
    assert datetime.date(2023, 8, 5) < datetime.date(2024, 8, 6)


def test_datetime():
    assert datetime.datetime(
        2023, 8, 5, 12, 0, 0) == datetime.datetime(2023, 8, 5, 12, 0, 0)
    assert datetime.datetime(
        2023, 8, 5, 12, 0, 0) != datetime.datetime(2023, 8, 5, 12, 1, 0)
    assert datetime.datetime(
        2023, 8, 5, 12, 0, 0) >= datetime.datetime(2023, 8, 5, 12, 0, 0)
    assert datetime.datetime(
        2023, 8, 5, 12, 30, 0) > datetime.datetime(2023, 8, 5, 12, 1, 0)
    assert datetime.datetime(
        2023, 8, 5, 12, 0, 0) < datetime.datetime(2023, 8, 5, 12, 1, 0)
    assert datetime.datetime(
        2023, 8, 5, 12, 0, 0) <= datetime.datetime(2023, 8, 5, 12, 1, 0)


test_timedelta()
test_date()
test_datetime()
