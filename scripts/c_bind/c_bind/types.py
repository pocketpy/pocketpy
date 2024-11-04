C_INT_TYPES = [
    'char', 'short', 'int', 'long', 'long long',
    'signed char', 'signed short', 'signed int', 'signed long', 'signed long long',
    'unsigned char', 'unsigned short', 'unsigned int', 'unsigned long', 'unsigned long long',
    'int8_t', 'int16_t', 'int32_t', 'int64_t',
    'uint8_t', 'uint16_t', 'uint32_t', 'uint64_t',
    'intptr_t', 'uintptr_t',
    'ptrdiff_t', 'size_t',
    'unsigned', 'signed',
    'py_i64',
]

C_FLOAT_TYPES = [
    'float', 'double',
    'py_f64',
]

C_BOOL_TYPES = [
    'bool', '_Bool',
]

C_STRING_TYPES = [
    'const char*',
    'const char *',
]

LINALG_TYPES = [
    'vec2', 'vec3', 'vec2i', 'vec3i',
    'mat3x3'
]
