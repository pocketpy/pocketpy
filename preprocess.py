import os
from datetime import datetime

def generate_python_sources():
    sources = {}
    for file in os.listdir("python"):
        assert file.endswith(".py")
        key = file.split(".")[0]
        with open("python/" + file) as f:
            value = f.read()
            value = value.encode('utf-8').hex()
            new_value = []
            for i in range(0, len(value), 2):
                new_value.append("\\x" + value[i:i+2])
        sources[key] = "".join(new_value)

    timestamp = datetime.now().strftime("%Y-%m-%d %H:%M:%S")

    header = '''#pragma once
// generated on ''' + timestamp + '''
#include <map>
#include <string>

namespace pkpy{
    inline static std::map<std::string, const char*> kPythonLibs = {
'''
    for key, value in sources.items():
        header += ' '*8 + '{"' + key + '", "' + value + '"},'
        header += '\n'

    header += '''
    };
}   // namespace pkpy
'''
    return header

with open("src/_generated.h", "w", encoding='utf-8') as f:
    f.write(generate_python_sources())
