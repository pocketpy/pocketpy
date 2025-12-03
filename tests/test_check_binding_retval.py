#!/usr/bin/env python3
"""
Test suite for check_binding_retval.py script.

This test verifies that the binding return value checker correctly identifies
issues in Python binding functions.
"""

import os
import sys
import tempfile
import subprocess
from pathlib import Path

# Get the repository root
REPO_ROOT = Path(__file__).parent.parent
CHECKER_SCRIPT = REPO_ROOT / "scripts" / "check_binding_retval.py"


def run_checker(test_dir):
    """Run the checker on a test directory and return the exit code and output."""
    result = subprocess.run(
        [sys.executable, str(CHECKER_SCRIPT), "--dirs", test_dir],
        capture_output=True,
        text=True
    )
    return result.returncode, result.stdout, result.stderr


def test_correct_binding():
    """Test that correct bindings pass validation."""
    with tempfile.TemporaryDirectory() as tmpdir:
        test_file = Path(tmpdir) / "test_correct.c"
        test_file.write_text("""
#include "pocketpy/pocketpy.h"

// Correct: sets py_retval() with py_newint
static bool correct_function_1(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    py_newint(py_retval(), 42);
    return true;
}

// Correct: sets py_retval() with py_newnone
static bool correct_function_2(int argc, py_Ref argv) {
    PY_CHECK_ARGC(0);
    py_newnone(py_retval());
    return true;
}

// Correct: uses py_import which sets py_retval()
static bool correct_function_3(int argc, py_Ref argv) {
    int res = py_import("test");
    if(res == 1) return true;
    return false;
}

void register_correct() {
    py_GlobalRef mod = py_newmodule("test");
    py_bindfunc(mod, "f1", correct_function_1);
    py_bindfunc(mod, "f2", correct_function_2);
    py_bindfunc(mod, "f3", correct_function_3);
}
""")
        
        exit_code, stdout, stderr = run_checker(tmpdir)
        assert exit_code == 0, f"Expected exit code 0, got {exit_code}\n{stdout}\n{stderr}"
        assert "No issues found" in stdout, f"Expected success message\n{stdout}"
        print("✓ test_correct_binding passed")


def test_incorrect_binding():
    """Test that incorrect bindings are detected."""
    with tempfile.TemporaryDirectory() as tmpdir:
        test_file = Path(tmpdir) / "test_incorrect.c"
        test_file.write_text("""
#include "pocketpy/pocketpy.h"

// Incorrect: returns true without setting py_retval()
static bool incorrect_function(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    // Missing py_retval() assignment
    return true;
}

void register_incorrect() {
    py_GlobalRef mod = py_newmodule("test");
    py_bindfunc(mod, "bad", incorrect_function);
}
""")
        
        exit_code, stdout, stderr = run_checker(tmpdir)
        assert exit_code == 1, f"Expected exit code 1, got {exit_code}\n{stdout}\n{stderr}"
        assert "incorrect_function" in stdout, f"Expected to find function name\n{stdout}"
        assert "potential issues" in stdout, f"Expected issues message\n{stdout}"
        print("✓ test_incorrect_binding passed")


def test_comments_ignored():
    """Test that comments mentioning py_retval() don't cause false negatives."""
    with tempfile.TemporaryDirectory() as tmpdir:
        test_file = Path(tmpdir) / "test_comments.c"
        test_file.write_text("""
#include "pocketpy/pocketpy.h"

// This function has comments about py_retval() but doesn't actually set it
static bool buggy_function(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    // TODO: Should call py_retval() here
    /* py_newint(py_retval(), 42); */
    return true;  // BUG: Missing actual py_retval()
}

void register_buggy() {
    py_GlobalRef mod = py_newmodule("test");
    py_bindfunc(mod, "bug", buggy_function);
}
""")
        
        exit_code, stdout, stderr = run_checker(tmpdir)
        assert exit_code == 1, f"Expected exit code 1, got {exit_code}\n{stdout}\n{stderr}"
        assert "buggy_function" in stdout, f"Expected to find function name\n{stdout}"
        print("✓ test_comments_ignored passed")


def test_actual_codebase():
    """Test that the actual codebase passes validation."""
    src_bindings = REPO_ROOT / "src" / "bindings"
    src_modules = REPO_ROOT / "src" / "modules"
    
    if not src_bindings.exists() or not src_modules.exists():
        print("⊘ test_actual_codebase skipped (directories not found)")
        return
    
    exit_code, stdout, stderr = run_checker(str(REPO_ROOT / "src"))
    assert exit_code == 0, f"Actual codebase should pass validation\n{stdout}\n{stderr}"
    print("✓ test_actual_codebase passed")


def main():
    """Run all tests."""
    print("Running tests for check_binding_retval.py...")
    print("=" * 80)
    
    tests = [
        test_correct_binding,
        test_incorrect_binding,
        test_comments_ignored,
        test_actual_codebase,
    ]
    
    failed = 0
    for test in tests:
        try:
            test()
        except AssertionError as e:
            print(f"✗ {test.__name__} failed: {e}")
            failed += 1
        except Exception as e:
            print(f"✗ {test.__name__} error: {e}")
            failed += 1
    
    print("=" * 80)
    if failed == 0:
        print(f"All {len(tests)} tests passed!")
        return 0
    else:
        print(f"{failed}/{len(tests)} tests failed!")
        return 1


if __name__ == "__main__":
    sys.exit(main())
