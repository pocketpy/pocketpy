"""
Optimized bisection algorithms for sorted list insertions.
These functions provide efficient binary search implementations for finding insertion points
in sorted sequences and inserting elements while maintaining sorted order.
"""
def bisect_right(a, x, lo=0, hi=None):
    """Return the index where to insert item x in list a, assuming a is sorted.
    
    The return value i is such that all e in a[:i] have e <= x, and all e in
    a[i:] have e > x. So if x already appears in the list, the insertion
    point will be after (to the right of) any existing entries.
    
    Args:
        a: A sorted list-like object supporting comparison and __len__
        x: The item to be inserted
        lo: Lower bound of the slice to be searched (inclusive)
        hi: Upper bound of the slice to be searched (exclusive)
    
    Returns:
        The index where x should be inserted to maintain sorted order
    
    Raises:
        ValueError: If lo is negative
    """
    if lo < 0:
        raise ValueError('lo must be non-negative')
    
    if hi is None:
        hi = len(a)
    
    # Fast path for common case: appending to the end
    if hi > 0 and hi == len(a) and x > a[-1]:
        return hi
    
    # Normal binary search
    while lo < hi:
        mid = (lo + hi) // 2
        if x < a[mid]:
            hi = mid
        else:
            lo = mid + 1
    
    return lo


def bisect_left(a, x, lo=0, hi=None):
    """Return the index where to insert item x in list a, assuming a is sorted.
    
    The return value i is such that all e in a[:i] have e < x, and all e in
    a[i:] have e >= x. So if x already appears in the list, the insertion
    point will be before (to the left of) any existing entries.
    
    Args:
        a: A sorted list-like object supporting comparison and __len__
        x: The item to be inserted
        lo: Lower bound of the slice to be searched (inclusive)
        hi: Upper bound of the slice to be searched (exclusive)
    
    Returns:
        The index where x should be inserted to maintain sorted order
    
    Raises:
        ValueError: If lo is negative
    """
    if lo < 0:
        raise ValueError('lo must be non-negative')
    
    if hi is None:
        hi = len(a)
    
    # Fast path for common case: appending to the end
    if hi > 0 and hi == len(a) and x > a[-1]:
        return hi
        
    # Fast path for common case: inserting at the beginning
    if lo == 0 and hi > 0 and x < a[0]:
        return 0
    
    # Normal binary search
    while lo < hi:
        mid = (lo + hi) // 2
        if a[mid] < x:
            lo = mid + 1
        else:
            hi = mid
    
    return lo


def insort_right(a, x, lo=0, hi=None):
    """Insert item x in list a, and keep it sorted assuming a is sorted.
    
    If x is already in a, insert it to the right of the rightmost x.
    
    Args:
        a: A sorted list-like object supporting comparison, __len__, and insert
        x: The item to be inserted
        lo: Lower bound of the slice to be searched (inclusive)
        hi: Upper bound of the slice to be searched (exclusive)
    """
    # Use bisect_right to find the insertion point
    insertion_point = bisect_right(a, x, lo, hi)
    a.insert(insertion_point, x)


def insort_left(a, x, lo=0, hi=None):
    """Insert item x in list a, and keep it sorted assuming a is sorted.
    
    If x is already in a, insert it to the left of the leftmost x.
    
    Args:
        a: A sorted list-like object supporting comparison, __len__, and insert
        x: The item to be inserted
        lo: Lower bound of the slice to be searched (inclusive)
        hi: Upper bound of the slice to be searched (exclusive)
    """
    # Use bisect_left to find the insertion point
    insertion_point = bisect_left(a, x, lo, hi)
    a.insert(insertion_point, x)

# Create aliases for backward compatibility
bisect = bisect_right
insort = insort_right
