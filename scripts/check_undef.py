import re

def check_define_undef_pairs(code):
    # 使用正则表达式匹配#define和#undef指令
    define_pattern = re.compile(r'#define\s+(\w+)')
    undef_pattern = re.compile(r'#undef\s+(\w+)')

    # 查找所有的#define和#undef
    defines = define_pattern.findall(code)
    undefs = undef_pattern.findall(code)

    # 使用集合计算差集，找出不匹配的部分
    define_set = set(defines)
    undef_set = set(undefs)

    unmatched_defines = define_set - undef_set
    unmatched_undefs = undef_set - define_set

    if unmatched_defines or unmatched_undefs:
        if unmatched_defines:
            print("mismatched #define")
            for define in unmatched_defines:
                print(f"- {define}")

        if unmatched_undefs:
            print("mismatched #undef")
            for undef in unmatched_undefs:
                print(f"- {undef}")


# iterate over all the files in `path` directory

import os
import sys

def check_undef_in_dir(path):
    for root, dirs, files in os.walk(path):
        for file in files:
            if file.endswith(".c") or file.endswith(".h"):
                with open(os.path.join(root, file), "r", encoding='utf-8') as f:
                    print(f"==> {os.path.join(root, file)}")
                    check_define_undef_pairs(f.read())

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python scripts/check_undef.py <path>")
        sys.exit(1)

    check_undef_in_dir(sys.argv[1])