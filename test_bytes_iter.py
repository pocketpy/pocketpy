#!/usr/bin/env python3
# Test script for bytes iteration fix (issue #450)

print("Testing bytes iteration...")
print()

# Test 1: Basic bytes creation and representation
text = "Hello"
byte_data = text.encode()
print("Test 1: Basic bytes creation and representation")
print(f"Text: {text}")
print(f"Bytes: {byte_data}")
print()

# Test 2: list(bytes)
print("Test 2: list(byte_data):")
try:
    result = list(byte_data)
    print(f"Result: {result}")
    expected = [72, 101, 108, 108, 111]
    assert result == expected, f"Expected {expected}, got {result}"
    print("✓ PASS")
except Exception as e:
    print(f"✗ FAIL: {e}")
print()

# Test 3: for loop iteration
print("Test 3: for loop iteration")
try:
    result = []
    for x in byte_data:
        result.append(x)
    print(f"Result: {result}")
    expected = [72, 101, 108, 108, 111]
    assert result == expected, f"Expected {expected}, got {result}"
    print("✓ PASS")
except Exception as e:
    print(f"✗ FAIL: {e}")
print()

# Test 4: bytes indexing (should still work)
print("Test 4: bytes indexing")
try:
    result = byte_data[0]
    print(f"byte_data[0] = {result}")
    assert result == 72, f"Expected 72, got {result}"
    print("✓ PASS")
except Exception as e:
    print(f"✗ FAIL: {e}")
print()

# Test 5: len(bytes)
print("Test 5: len(byte_data)")
try:
    result = len(byte_data)
    print(f"len(byte_data) = {result}")
    assert result == 5, f"Expected 5, got {result}"
    print("✓ PASS")
except Exception as e:
    print(f"✗ FAIL: {e}")
print()

# Test 6: bytes slicing
print("Test 6: bytes slicing")
try:
    result = byte_data[1:3]
    result_list = list(result)
    print(f"list(byte_data[1:3]) = {result_list}")
    expected = [101, 108]
    assert result_list == expected, f"Expected {expected}, got {result_list}"
    print("✓ PASS")
except Exception as e:
    print(f"✗ FAIL: {e}")
print()

print("All tests completed!")
