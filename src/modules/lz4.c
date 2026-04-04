#ifdef PK_BUILD_MODULE_LZ4

#include <string.h>
#include <assert.h>
#include "pocketpy/common/utils.h"
#include "pocketpy/pocketpy.h"
#include "lz4/lib/lz4.h"

static bool lz4_compress(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_bytes);
    int src_size;
    const void* src = py_tobytes(argv, &src_size);
    uint32_t dst_capacity = LZ4_compressBound(src_size);
    char* p = (char*)py_newbytes(py_retval(), sizeof(uint32_t) + dst_capacity);
    memcpy(p, &src_size, sizeof(uint32_t));
    char* dst = p + sizeof(uint32_t);
    int dst_size = LZ4_compress_default(src, dst, src_size, dst_capacity);
    if(dst_size <= 0) return ValueError("LZ4 compression failed");
    py_bytes_resize(py_retval(), sizeof(uint32_t) + dst_size);
    return true;
}

static bool lz4_decompress(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_bytes);
    int total_size;
    const uint32_t* p = (uint32_t*)py_tobytes(argv, &total_size);
    const char* src = (const char*)(p + 1);
    if(total_size < sizeof(uint32_t)) return ValueError("invalid LZ4 data");
    uint32_t uncompressed_size;
    memcpy(&uncompressed_size, p, sizeof(uint32_t));
    if(uncompressed_size >= INT32_MAX) return ValueError("invalid LZ4 data");
    char* dst = (char*)py_newbytes(py_retval(), uncompressed_size);
    int dst_size = LZ4_decompress_safe(src, dst, total_size - sizeof(uint32_t), uncompressed_size);
    if(dst_size < 0) return ValueError("LZ4 decompression failed");
    c11__rtassert(dst_size == uncompressed_size);
    return true;
}

void pk__add_module_lz4() {
    py_Ref mod = py_newmodule("lz4");
    py_bindfunc(mod, "compress", lz4_compress);
    py_bindfunc(mod, "decompress", lz4_decompress);
}

#else

void pk__add_module_lz4() {}

#endif
