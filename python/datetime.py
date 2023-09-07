from time import localtime

class date:
    def __init__(self, year: int, month: int, day: int):
        self.year = year
        self.month = month
        self.day = day

    @staticmethod
    def today():
        t = localtime()
        return date(t.tm_year, t.tm_mon, t.tm_mday)
    
    def __str__(self):
        return f"{self.year}-{self.month:02}-{self.day:02}"
    
    def __repr__(self):
        return f"datetime.date({self.year}, {self.month}, {self.day})"

class datetime(date):
    def __init__(self, year: int, month: int, day: int, hour: int, minute: int, second: int):
        super(datetime, self).__init__(year, month, day)
        self.hour = hour
        self.minute = minute
        self.second = second

    @staticmethod
    def now():
        t = localtime()
        return datetime(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec)

    def __str__(self):
        return f"{self.year}-{self.month:02}-{self.day:02} {self.hour:02}:{self.minute:02}:{self.second:02}"

    def __repr__(self):
        return f"datetime.datetime({self.year}, {self.month}, {self.day}, {self.hour}, {self.minute}, {self.second})"