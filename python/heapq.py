# Heap queue algorithm (a.k.a. priority queue)

__all__ = ['heappush', 'heappop', 'heapify', 'heapreplace', 'merge',
           'nlargest', 'nsmallest', 'heappushpop']

def heappush(heap, item):
    """Push item onto heap, maintaining the heap invariant."""
    heap.append(item)
    _siftdown(heap, 0, len(heap)-1)

def heappop(heap):
    """Pop the smallest item off the heap, maintaining the heap invariant."""
    lastelt = heap.pop()    # raises appropriate IndexError if heap is empty
    if heap:
        returnitem = heap[0]
        heap[0] = lastelt
        _siftup(heap, 0)
        return returnitem
    return lastelt

def heapreplace(heap, item):
    """Pop and return the current smallest value, and add the new item.

    This is more efficient than heappop() followed by heappush(), and can be
    more appropriate when using a fixed-size heap.  Note that the value
    returned may be larger than item!  That constrains reasonable uses of
    this routine unless written as part of a conditional replacement:

        if item > heap[0]:
            item = heapreplace(heap, item)
    """
    returnitem = heap[0]    # raises appropriate IndexError if heap is empty
    heap[0] = item
    _siftup(heap, 0)
    return returnitem

def heappushpop(heap, item):
    """Fast version of a heappush followed by a heappop."""
    if heap and heap[0] < item:
        item, heap[0] = heap[0], item
        _siftup(heap, 0)
    return item

def heapify(x):
    """Transform list into a heap, in-place, in O(len(x)) time."""
    n = len(x)
    # Transform bottom-up.  The largest index there's any point to looking at
    # is the largest with a child index in-range, so must have 2*i + 1 < n,
    # or i < (n-1)/2.  If n is even = 2*j, this is (2*j-1)/2 = j-1/2 so
    # j-1 is the largest, which is n//2 - 1.  If n is odd = 2*j+1, this is
    # (2*j+1-1)/2 = j so j-1 is the largest, and that's again n//2-1.
    for i in reversed(range(n//2)):
        _siftup(x, i)

# 'heap' is a heap at all indices >= startpos, except possibly for pos.  pos
# is the index of a leaf with a possibly out-of-order value.  Restore the
# heap invariant.
def _siftdown(heap, startpos, pos):
    newitem = heap[pos]
    # Follow the path to the root, moving parents down until finding a place
    # newitem fits.
    while pos > startpos:
        parentpos = (pos - 1) >> 1
        parent = heap[parentpos]
        if newitem < parent:
            heap[pos] = parent
            pos = parentpos
            continue
        break
    heap[pos] = newitem

# The child indices of heap index pos are already heaps, and we want to make
# a heap at index pos too.  We do this by bubbling the smaller child of
# pos up (and so on with that child's children, etc) until hitting a leaf,
# then using _siftdown to move the oddball originally at index pos into place.
#
# We *could* break out of the loop as soon as we find a pos where newitem <=
# both its children, but turns out that's not a good idea, and despite that
# many books write the algorithm that way.  During a heap pop, the last array
# element is sifted in, and that tends to be large, so that comparing it
# against values starting from the root usually doesn't pay (= usually doesn't
# get us out of the loop early).  See Knuth, Volume 3, where this is
# explained and quantified in an exercise.
#
# Cutting the # of comparisons is important, since these routines have no
# way to extract "the priority" from an array element, so that intelligence
# is likely to be hiding in custom comparison methods, or in array elements
# storing (priority, record) tuples.  Comparisons are thus potentially
# expensive.
#
# On random arrays of length 1000, making this change cut the number of
# comparisons made by heapify() a little, and those made by exhaustive
# heappop() a lot, in accord with theory.  Here are typical results from 3
# runs (3 just to demonstrate how small the variance is):
#
# Compares needed by heapify     Compares needed by 1000 heappops
# --------------------------     --------------------------------
# 1837 cut to 1663               14996 cut to 8680
# 1855 cut to 1659               14966 cut to 8678
# 1847 cut to 1660               15024 cut to 8703
#
# Building the heap by using heappush() 1000 times instead required
# 2198, 2148, and 2219 compares:  heapify() is more efficient, when
# you can use it.
#
# The total compares needed by list.sort() on the same lists were 8627,
# 8627, and 8632 (this should be compared to the sum of heapify() and
# heappop() compares):  list.sort() is (unsurprisingly!) more efficient
# for sorting.

def _siftup(heap, pos):
    endpos = len(heap)
    startpos = pos
    newitem = heap[pos]
    # Bubble up the smaller child until hitting a leaf.
    childpos = 2*pos + 1    # leftmost child position
    while childpos < endpos:
        # Set childpos to index of smaller child.
        rightpos = childpos + 1
        if rightpos < endpos and not heap[childpos] < heap[rightpos]:
            childpos = rightpos
        # Move the smaller child up.
        heap[pos] = heap[childpos]
        pos = childpos
        childpos = 2*pos + 1
    # The leaf at pos is empty now.  Put newitem there, and bubble it up
    # to its final resting place (by sifting its parents down).
    heap[pos] = newitem
    _siftdown(heap, startpos, pos)