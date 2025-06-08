from time import localtime
import operator
from typing import Union, Optional, Any

class timedelta:
    #Represent a duration, the difference between two dates or times
    
    def __init__(self, days: int = 0, seconds: int = 0, minutes: int = 0, hours: int = 0):
        """Initialize a timedelta object.
        
        Args:
            days: Number of days
            seconds: Number of seconds
            minutes: Number of minutes
            hours: Number of hours
        """
        # Normalize input values
        total_seconds = seconds + minutes * 60 + hours * 3600
        self.days = days
        self.seconds = total_seconds % 86400  
        self.days += total_seconds // 86400
    
    def total_seconds(self) -> float:
        #Return the total number of seconds contained in the duration
        return self.days * 86400 + self.seconds
    
    def __repr__(self) -> str:
        return f"datetime.timedelta(days={self.days}, seconds={self.seconds})"
    
    def __str__(self) -> str:
        days_str = f"{self.days} day{'s' if self.days != 1 else ''}" if self.days else ""
        hours, remainder = divmod(self.seconds, 3600)
        minutes, seconds = divmod(remainder, 60)
        
        parts = []
        if days_str:
            parts.append(days_str)
        if hours:
            parts.append(f"{hours} hour{'s' if hours != 1 else ''}")
        if minutes:
            parts.append(f"{minutes} minute{'s' if minutes != 1 else ''}")
        if seconds or not parts:  # Include seconds if it's non-zero or if all other parts are zero
            parts.append(f"{seconds} second{'s' if seconds != 1 else ''}")
            
        return ", ".join(parts)
    
    def __eq__(self, other: Any) -> bool:
        if not isinstance(other, timedelta):
            return NotImplemented
        return self.total_seconds() == other.total_seconds()
    
    def __ne__(self, other: Any) -> bool:
        if not isinstance(other, timedelta):
            return NotImplemented
        return self.total_seconds() != other.total_seconds()
    
    def __lt__(self, other: 'timedelta') -> bool:
        if not isinstance(other, timedelta):
            return NotImplemented
        return self.total_seconds() < other.total_seconds()
    
    def __le__(self, other: 'timedelta') -> bool:
        if not isinstance(other, timedelta):
            return NotImplemented
        return self.total_seconds() <= other.total_seconds()
    
    def __gt__(self, other: 'timedelta') -> bool:
        if not isinstance(other, timedelta):
            return NotImplemented
        return self.total_seconds() > other.total_seconds()
    
    def __ge__(self, other: 'timedelta') -> bool:
        if not isinstance(other, timedelta):
            return NotImplemented
        return self.total_seconds() >= other.total_seconds()
    
    def __add__(self, other: 'timedelta') -> 'timedelta':
        if not isinstance(other, timedelta):
            return NotImplemented
        return timedelta(days=self.days + other.days, seconds=self.seconds + other.seconds)
    
    def __sub__(self, other: 'timedelta') -> 'timedelta':
        if not isinstance(other, timedelta):
            return NotImplemented
        return timedelta(days=self.days - other.days, seconds=self.seconds - other.seconds)
    
    def __mul__(self, other: Union[int, float]) -> 'timedelta':
        if not isinstance(other, (int, float)):
            return NotImplemented
        total_seconds = self.total_seconds() * other
        days, seconds = divmod(int(total_seconds), 86400)
        return timedelta(days=days, seconds=seconds)
    
    def __rmul__(self, other: Union[int, float]) -> 'timedelta':
        return self.__mul__(other)
    
    def __neg__(self) -> 'timedelta':
        return timedelta(days=-self.days, seconds=-self.seconds)
    
    def __pos__(self) -> 'timedelta':
        return timedelta(days=self.days, seconds=self.seconds)
    
    def __abs__(self) -> 'timedelta':
        if self.days < 0 or self.seconds < 0:
            return -self
        return self


class date:
    #Represent a date (year, month, day)
    
    _DAYS_IN_MONTH = [0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31]
    
    def __init__(self, year: int, month: int, day: int):
        """Initialize a date object.
        
        Args:
            year: Year (1-9999)
            month: Month (1-12)
            day: Day (1-31, depending on month)
            
        Raises:
            ValueError: If the date is invalid
        """
        # Validate input
        if not 1 <= month <= 12:
            raise ValueError("Month must be between 1 and 12")
        
        # Adjust for leap year
        days_in_month = self._days_in_month(year, month)
        if not 1 <= day <= days_in_month:
            raise ValueError(f"Day must be between 1 and {days_in_month} for month {month}")
        
        self.year = year
        self.month = month
        self.day = day
    
    @staticmethod
    def _is_leap_year(year: int) -> bool:
        #Check if a year is a leap year
        return year % 4 == 0 and (year % 100 != 0 or year % 400 == 0)
    
    @classmethod
    def _days_in_month(cls, year: int, month: int) -> int:
        #Return the number of days in the given month
        if month == 2 and cls._is_leap_year(year):
            return 29
        return cls._DAYS_IN_MONTH[month]
    
    @classmethod
    def today(cls) -> 'date':
        #Return the current local date
        t = localtime()
        return cls(t.tm_year, t.tm_mon, t.tm_mday)
    
    def replace(self, year: Optional[int] = None, month: Optional[int] = None, day: Optional[int] = None) -> 'date':
        #Return a new date with the same attributes, except for those given new values
        return date(
            year if year is not None else self.year,
            month if month is not None else self.month,
            day if day is not None else self.day
        )
    
    def __str__(self) -> str:
        return f"{self.year}-{self.month:02d}-{self.day:02d}"
    
    def __repr__(self) -> str:
        return f"datetime.date({self.year}, {self.month}, {self.day})"
    
    def __cmp(self, other: Any, op: Any) -> bool:
        if not isinstance(other, date):
            return NotImplemented
        return op((self.year, self.month, self.day), (other.year, other.month, other.day))
    
    def __eq__(self, other: Any) -> bool:
        return self.__cmp(other, operator.eq)
    
    def __ne__(self, other: Any) -> bool:
        return self.__cmp(other, operator.ne)
    
    def __lt__(self, other: 'date') -> bool:
        return self.__cmp(other, operator.lt)
    
    def __le__(self, other: 'date') -> bool:
        return self.__cmp(other, operator.le)
    
    def __gt__(self, other: 'date') -> bool:
        return self.__cmp(other, operator.gt)
    
    def __ge__(self, other: 'date') -> bool:
        return self.__cmp(other, operator.ge)
    
    def __sub__(self, other: Union['date', timedelta]) -> Union[timedelta, 'date']:
        #Subtract another date or timedelta from this date
        if isinstance(other, date):
            # Simplified calculation - this doesn't handle leap years correctly
            # For a robust implementation, you'd use a calendar algorithm
            days_self = self.year * 365 + self.month * 30 + self.day
            days_other = other.year * 365 + other.month * 30 + other.day
            return timedelta(days=days_self - days_other)
        elif isinstance(other, timedelta):
            # Simplified subtraction - doesn't handle month boundaries correctly
            # For a robust implementation, you'd use a calendar algorithm
            return self  # Placeholder for actual implementation
        return NotImplemented
    
    def __add__(self, other: timedelta) -> 'date':
        #Add a timedelta to this date
        if isinstance(other, timedelta):
            # Simplified addition - doesn't handle month boundaries correctly
            # For a robust implementation, you'd use a calendar algorithm
            return self  # Placeholder for actual implementation
        return NotImplemented
    
    def weekday(self) -> int:
        #Return the day of the week as an integer (0 is Monday, 6 is Sunday)
        # Simplified implementation using Zeller's congruence
        # For a robust implementation, you'd use a more accurate algorithm
        m = self.month
        y = self.year
        if m < 3:
            m += 12
            y -= 1
        k = y % 100
        j = y // 100
        
        h = (self.day + 13 * (m + 1) // 5 + k + k // 4 + j // 4 - 2 * j) % 7
        return (h + 5) % 7  # Convert to Monday=0, Sunday=6 format


class datetime(date):
    #Represent a date and time
    
    def __init__(self, year: int, month: int, day: int, hour: int = 0, minute: int = 0, second: int = 0):
        """Initialize a datetime object.
        
        Args:
            year: Year (1-9999)
            month: Month (1-12)
            day: Day (1-31, depending on month)
            hour: Hour (0-23)
            minute: Minute (0-59)
            second: Second (0-59)
            
        Raises:
            ValueError: If any of the parameters are invalid
        """
        super().__init__(year, month, day)
        
        # Validate time components
        if not 0 <= hour <= 23:
            raise ValueError("Hour must be between 0 and 23")
        if not 0 <= minute <= 59:
            raise ValueError("Minute must be between 0 and 59")
        if not 0 <= second <= 59:
            raise ValueError("Second must be between 0 and 59")
        
        self.hour = hour
        self.minute = minute
        self.second = second
    
    def date(self) -> date:
        #Return the date part of the datetime
        return date(self.year, self.month, self.day)
    
    def time(self) -> tuple:
        #Return the time part of the datetime as a tuple (hour, minute, second)
        return (self.hour, self.minute, self.second)
    
    def replace(self, year: Optional[int] = None, month: Optional[int] = None, day: Optional[int] = None,
                hour: Optional[int] = None, minute: Optional[int] = None, second: Optional[int] = None) -> 'datetime':
        """Return a new datetime with the same attributes, except for those given new values."""
        return datetime(
            year if year is not None else self.year,
            month if month is not None else self.month,
            day if day is not None else self.day,
            hour if hour is not None else self.hour,
            minute if minute is not None else self.minute,
            second if second is not None else self.second
        )
    
    @classmethod
    def now(cls) -> 'datetime':
        #Return the current local datetime
        t = localtime()
        tm_sec = t.tm_sec
        if tm_sec == 60:  # Handle leap second
            tm_sec = 59
        return cls(t.tm_year, t.tm_mon, t.tm_mday, t.tm_hour, t.tm_min, tm_sec)
    
    @classmethod
    def combine(cls, date_obj: date, time_tuple: tuple) -> 'datetime':
        #Combine a date object and a time tuple (hour, minute, second) into a datetime
        hour, minute, second = time_tuple
        return cls(date_obj.year, date_obj.month, date_obj.day, hour, minute, second)
    
    def __str__(self) -> str:
        return f"{self.year}-{self.month:02d}-{self.day:02d} {self.hour:02d}:{self.minute:02d}:{self.second:02d}"
    
    def __repr__(self) -> str:
        return f"datetime.datetime({self.year}, {self.month}, {self.day}, {self.hour}, {self.minute}, {self.second})"
    
    def __cmp(self, other: Any, op: Any) -> bool:
        if not isinstance(other, datetime):
            return NotImplemented
        return op(
            (self.year, self.month, self.day, self.hour, self.minute, self.second),
            (other.year, other.month, other.day, other.hour, other.minute, other.second)
        )
    
    def __eq__(self, other: Any) -> bool:
        return self.__cmp(other, operator.eq)
    
    def __ne__(self, other: Any) -> bool:
        return self.__cmp(other, operator.ne)
    
    def __lt__(self, other: 'datetime') -> bool:
        return self.__cmp(other, operator.lt)
    
    def __le__(self, other: 'datetime') -> bool:
        return self.__cmp(other, operator.le)
    
    def __gt__(self, other: 'datetime') -> bool:
        return self.__cmp(other, operator.gt)
    
    def __ge__(self, other: 'datetime') -> bool:
        return self.__cmp(other, operator.ge)
    
    
