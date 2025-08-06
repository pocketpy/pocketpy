#include "pocketpy.h"

#define CUTE_PNG_IMPLEMENTATION
#include "cute_png.h"

static bool cute_png_loads(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_bytes);
    int size;
    unsigned char* data = py_tobytes(argv, &size);
    cp_image_t image = cp_load_png_mem(data, size);
    if(image.pix == NULL) return ValueError("cute_png: %s", cp_error_reason);
    py_newarray2d(py_retval(), image.w, image.h);
    for(int y = 0; y < image.h; y++) {
        for(int x = 0; x < image.w; x++) {
            cp_pixel_t pixel = image.pix[y * image.w + x];
            py_ObjectRef slot = py_array2d_getitem(py_retval(), x, y);
            c11_color32 color;
            color.r = pixel.r;
            color.g = pixel.g;
            color.b = pixel.b;
            color.a = pixel.a;
            py_newcolor32(slot, color);
        }
    }
    return true;
}

static bool cute_png_dumps(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_array2d);
    int width = py_array2d_getwidth(argv);
    int height = py_array2d_getheight(argv);
    cp_image_t image = cp_load_blank(width, height);
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            py_ObjectRef slot = py_array2d_getitem(argv, x, y);
            if(!py_checktype(slot, tp_color32)) return false;
            c11_color32 color = py_tocolor32(slot);
            cp_pixel_t pixel = cp_make_pixel_a(color.r, color.g, color.b, color.a);
            image.pix[y * width + x] = pixel;
        }
    }
    cp_saved_png_t saved_image = cp_save_png_to_memory(&image);
    assert(saved_image.data != NULL);
    unsigned char* data = py_newbytes(py_retval(), saved_image.size);
    memcpy(data, saved_image.data, saved_image.size);
    return true;
}

void pk__add_module_cute_png() {
    py_GlobalRef mod = py_newmodule("cute_png");

    py_bindfunc(mod, "loads", cute_png_loads);
    py_bindfunc(mod, "dumps", cute_png_dumps);
}