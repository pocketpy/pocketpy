#!/usr/bin/env python3
"""
Static analysis tool to check Python binding functions for missing py_retval() assignments.

This tool checks whether Python binding functions properly set return values before returning true.
According to pocketpy conventions, when a binding function returns true, it MUST either:
1. Assign a value to py_retval() using py_new* functions, py_assign, etc.
2. Set the return value to None using py_newnone(py_retval())
3. Call a function that sets py_retval() internally (like py_import, py_call, py_iter, etc.)

Usage:
    python scripts/check_binding_retval.py [--verbose]
    
Exit codes:
    0: No issues found
    1: Issues found
    2: Script error
"""

import os
import re
import sys
import argparse
from typing import List, Dict, Tuple, Set

# Functions that set py_retval() internally
RETVAL_SETTING_FUNCTIONS = {
    'py_import',      # Sets py_retval() on success
    'py_call',        # Sets py_retval() with result
    'py_iter',        # Sets py_retval() with iterator
    'py_str',         # Sets py_retval() with string representation
    'py_repr',        # Sets py_retval() with repr string
    'py_getattr',     # Sets py_retval() with attribute value
    'py_next',        # Sets py_retval() with next value
    'py_getitem',     # Sets py_retval() with item
    'py_vectorcall',  # Sets py_retval() with call result
}

# Patterns that indicate py_retval() is being set
RETVAL_PATTERNS = [
    r'py_retval\(\)',                    # Direct py_retval() usage
    r'py_new\w+\s*\(\s*py_retval\(\)',   # py_newint(py_retval(), ...)
    r'py_assign\s*\(\s*py_retval\(\)',   # py_assign(py_retval(), ...)
    r'\*py_retval\(\)\s*=',              # *py_retval() = ...
]


class BindingChecker:
    """Checker for Python binding functions."""
    
    def __init__(self, verbose: bool = False):
        self.verbose = verbose
        self.issues: List[Dict] = []
        
    def log(self, message: str):
        """Log message if verbose mode is enabled."""
        if self.verbose:
            print(f"[DEBUG] {message}")
    
    def find_c_files(self, *directories: str) -> List[str]:
        """Find all .c files in the given directories."""
        c_files = []
        for directory in directories:
            if not os.path.exists(directory):
                self.log(f"Directory not found: {directory}")
                continue
            for root, _, files in os.walk(directory):
                for file in files:
                    if file.endswith('.c'):
                        c_files.append(os.path.join(root, file))
        return c_files
    
    def extract_functions(self, content: str) -> Dict[str, Dict]:
        """Extract all bool-returning functions from C code."""
        # Pattern to match bool-returning functions
        pattern = r'(?:static\s+)?bool\s+(\w+)\s*\(([^)]*)\)\s*\{([^{}]*(?:\{[^{}]*\}[^{}]*)*)\}'
        
        functions = {}
        for match in re.finditer(pattern, content, re.MULTILINE | re.DOTALL):
            func_name = match.group(1)
            func_params = match.group(2)
            func_body = match.group(3)
            full_func = match.group(0)
            
            functions[func_name] = {
                'params': func_params,
                'body': func_body,
                'full': full_func,
                'start_pos': match.start(),
            }
        
        return functions
    
    def get_bound_functions(self, content: str) -> Set[str]:
        """Find functions that are bound as Python callables."""
        bound_funcs = set()
        
        # Binding patterns used in pocketpy
        patterns = [
            r'py_bindfunc\s*\([^,]+,\s*"[^"]+",\s*(\w+)\)',
            r'py_bind\s*\([^,]+,\s*"[^"]*",\s*(\w+)\)',
            r'py_bindmagic\s*\([^,]+,\s*\w+,\s*(\w+)\)',
            r'py_bindmethod\s*\([^,]+,\s*"[^"]+",\s*(\w+)\)',
            r'py_bindproperty\s*\([^,]+,\s*"[^"]+",\s*(\w+)',
        ]
        
        for pattern in patterns:
            for match in re.finditer(pattern, content):
                func_name = match.group(1)
                bound_funcs.add(func_name)
                self.log(f"Found bound function: {func_name}")
        
        return bound_funcs
    
    def remove_comments(self, code: str) -> str:
        """Remove C-style comments from code."""
        # Remove single-line comments
        code = re.sub(r'//.*?$', '', code, flags=re.MULTILINE)
        # Remove multi-line comments
        code = re.sub(r'/\*.*?\*/', '', code, flags=re.DOTALL)
        return code
    
    def has_retval_usage(self, func_body: str) -> bool:
        """Check if function body uses py_retval() in any form."""
        # Remove comments to avoid false positives
        code_without_comments = self.remove_comments(func_body)
        
        # Check for direct patterns
        for pattern in RETVAL_PATTERNS:
            if re.search(pattern, code_without_comments):
                return True
        
        # Check for functions that set py_retval internally
        for func in RETVAL_SETTING_FUNCTIONS:
            if func + '(' in code_without_comments:
                return True
        
        return False
    
    def analyze_return_statements(self, func_body: str, func_name: str) -> List[Dict]:
        """Analyze return true statements in the function."""
        lines = func_body.split('\n')
        suspicious_returns = []
        
        for i, line in enumerate(lines):
            # Look for "return true" statements
            if re.search(r'\breturn\s+true\b', line):
                # Get context (10 lines before the return)
                start = max(0, i - 10)
                context_lines = lines[start:i+1]
                context = '\n'.join(context_lines)
                
                suspicious_returns.append({
                    'line_num': i + 1,
                    'line': line.strip(),
                    'context': context,
                })
        
        return suspicious_returns
    
    def check_function(self, func_name: str, func_info: Dict, filepath: str) -> bool:
        """
        Check if a bound function properly sets py_retval() before returning true.
        Returns True if there's an issue, False otherwise.
        """
        func_body = func_info['body']
        
        # Skip if function doesn't return true
        if 'return true' not in func_body:
            self.log(f"Function {func_name} doesn't return true, skipping")
            return False
        
        # Check if function has any py_retval usage
        if self.has_retval_usage(func_body):
            self.log(f"Function {func_name} uses py_retval(), OK")
            return False
        
        # Found a potential issue
        self.log(f"Function {func_name} returns true without py_retval()!")
        
        suspicious_returns = self.analyze_return_statements(func_body, func_name)
        
        issue = {
            'file': filepath,
            'function': func_name,
            'full_code': func_info['full'],
            'suspicious_returns': suspicious_returns,
        }
        
        self.issues.append(issue)
        return True
    
    def check_file(self, filepath: str) -> int:
        """Check all bound functions in a file."""
        self.log(f"Checking file: {filepath}")
        
        try:
            with open(filepath, 'r', encoding='utf-8', errors='ignore') as f:
                content = f.read()
        except Exception as e:
            print(f"Error reading {filepath}: {e}", file=sys.stderr)
            return 0
        
        # Extract functions and find bound ones
        functions = self.extract_functions(content)
        bound_funcs = self.get_bound_functions(content)
        
        if not bound_funcs:
            self.log(f"No bound functions found in {filepath}")
            return 0
        
        issues_count = 0
        for func_name in bound_funcs:
            if func_name not in functions:
                self.log(f"Bound function {func_name} not found in extracted functions")
                continue
            
            if self.check_function(func_name, functions[func_name], filepath):
                issues_count += 1
        
        return issues_count
    
    def check_directories(self, *directories: str) -> int:
        """Check all C files in the given directories."""
        c_files = self.find_c_files(*directories)
        
        if not c_files:
            print("No C files found to check", file=sys.stderr)
            return 0
        
        self.log(f"Found {len(c_files)} C files to check")
        
        total_issues = 0
        for filepath in c_files:
            issues = self.check_file(filepath)
            total_issues += issues
        
        return total_issues
    
    def print_report(self):
        """Print a detailed report of all issues found."""
        if not self.issues:
            print("✓ No issues found! All Python binding functions properly set py_retval().")
            return
        
        print(f"\n{'='*80}")
        print(f"Found {len(self.issues)} function(s) with potential issues:")
        print(f"{'='*80}\n")
        
        for i, issue in enumerate(self.issues, 1):
            print(f"Issue #{i}:")
            print(f"  File: {issue['file']}")
            print(f"  Function: {issue['function']}")
            print(f"  Problem: Function returns true but doesn't set py_retval()")
            print(f"\n  Function code:")
            print("  " + "-" * 76)
            for line in issue['full_code'].split('\n'):
                print(f"  {line}")
            print("  " + "-" * 76)
            
            if issue['suspicious_returns']:
                print(f"\n  Found {len(issue['suspicious_returns'])} 'return true' statement(s):")
                for ret in issue['suspicious_returns']:
                    print(f"    Line {ret['line_num']}: {ret['line']}")
            
            print(f"\n{'='*80}\n")


def main():
    parser = argparse.ArgumentParser(
        description='Check Python binding functions for missing py_retval() assignments',
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__
    )
    parser.add_argument(
        '--verbose', '-v',
        action='store_true',
        help='Enable verbose output for debugging'
    )
    parser.add_argument(
        '--dirs',
        nargs='+',
        default=['src/bindings', 'src/modules'],
        help='Directories to check (default: src/bindings src/modules)'
    )
    
    args = parser.parse_args()
    
    # Create checker and run analysis
    checker = BindingChecker(verbose=args.verbose)
    
    print("Checking Python binding functions for missing py_retval() assignments...")
    print(f"Target directories: {', '.join(args.dirs)}")
    print()
    
    try:
        total_issues = checker.check_directories(*args.dirs)
        checker.print_report()
        
        # Exit with appropriate code
        sys.exit(1 if total_issues > 0 else 0)
        
    except Exception as e:
        print(f"\nError during analysis: {e}", file=sys.stderr)
        if args.verbose:
            import traceback
            traceback.print_exc()
        sys.exit(2)


if __name__ == '__main__':
    main()
