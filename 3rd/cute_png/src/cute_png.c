#include "pocketpy.h"
#include <stdio.h>

#define CUTE_PNG_IMPLEMENTATION
#include "cute_png.h"

#if PY_SYS_PLATFORM == 5
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#endif

static bool cute_png_loads(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_bytes);
    int size;
    unsigned char* data = py_tobytes(argv, &size);
    cp_image_t image = cp_load_png_mem(data, size);
    if(image.pix == NULL) return ValueError("cp_load_png_mem() failed");
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
    cp_free_png(&image);
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
    CUTE_PNG_FREE(saved_image.data);
    cp_free_png(&image);
    return true;
}

static void cute_png_Image__dtor(void* ud) {
    cp_image_t* image = (cp_image_t*)ud;
    cp_free_png(image);
}

static bool cute_png_Image__new__(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    int width = py_toint(py_arg(1));
    int height = py_toint(py_arg(2));
    cp_image_t* ud = py_newobject(py_retval(), py_totype(argv), 0, sizeof(cp_image_t));
    *ud = cp_load_blank(width, height);
    return true;
}

static bool cute_png_Image__from_bytes_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_bytes);
    int size;
    unsigned char* data = py_tobytes(argv, &size);
    cp_image_t image = cp_load_png_mem(data, size);
    if(image.pix == NULL) return ValueError("cp_load_png_mem() failed");
    cp_image_t* ud =
        py_newobject(py_retval(), py_gettype("cute_png", py_name("Image")), 0, sizeof(cp_image_t));
    *ud = image;
    return true;
}

static bool cute_png_Image__from_file_STATIC(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    PY_CHECK_ARG_TYPE(0, tp_str);
    const char* path = py_tostr(argv);
    cp_image_t image = cp_load_png(path);
    if(image.pix == NULL) return ValueError("cp_load_png() failed");
    cp_image_t* ud =
        py_newobject(py_retval(), py_gettype("cute_png", py_name("Image")), 0, sizeof(cp_image_t));
    *ud = image;
    return true;
}

static bool cute_png_Image__width(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    cp_image_t* image = py_touserdata(argv);
    py_newint(py_retval(), image->w);
    return true;
}

static bool cute_png_Image__height(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    cp_image_t* image = py_touserdata(argv);
    py_newint(py_retval(), image->h);
    return true;
}

static bool cute_png_Image__setpixel(int argc, py_Ref argv) {
    PY_CHECK_ARGC(4);
    cp_image_t* image = py_touserdata(argv);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    PY_CHECK_ARG_TYPE(3, tp_color32);
    int x = py_toint(py_arg(1));
    int y = py_toint(py_arg(2));
    c11_color32 color = py_tocolor32(py_arg(3));
    if(x < 0 || x >= image->w || y < 0 || y >= image->h) {
        return IndexError("cute_png.Image: index out of range");
    }
    cp_pixel_t pixel = cp_make_pixel_a(color.r, color.g, color.b, color.a);
    image->pix[y * image->w + x] = pixel;
    py_newnone(py_retval());
    return true;
}

static bool cute_png_Image__getpixel(int argc, py_Ref argv) {
    PY_CHECK_ARGC(3);
    cp_image_t* image = py_touserdata(argv);
    PY_CHECK_ARG_TYPE(1, tp_int);
    PY_CHECK_ARG_TYPE(2, tp_int);
    int x = py_toint(py_arg(1));
    int y = py_toint(py_arg(2));
    if(x < 0 || x >= image->w || y < 0 || y >= image->h) {
        return IndexError("cute_png.Image: index out of range");
    }
    cp_pixel_t pixel = image->pix[y * image->w + x];
    c11_color32 color;
    color.r = pixel.r;
    color.g = pixel.g;
    color.b = pixel.b;
    color.a = pixel.a;
    py_newcolor32(py_retval(), color);
    return true;
}

static bool cute_png_Image__clear(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    cp_image_t* image = py_touserdata(argv);
    PY_CHECK_ARG_TYPE(1, tp_color32);
    c11_color32 color = py_tocolor32(py_arg(1));
    cp_pixel_t pixel = cp_make_pixel_a(color.r, color.g, color.b, color.a);
    for(int y = 0; y < image->h; y++) {
        for(int x = 0; x < image->w; x++) {
            image->pix[y * image->w + x] = pixel;
        }
    }
    py_newnone(py_retval());
    return true;
}

static bool cute_png_Image__to_png_bytes(int argc, py_Ref argv) {
    PY_CHECK_ARGC(1);
    cp_image_t* image = py_touserdata(argv);
    cp_saved_png_t saved_image = cp_save_png_to_memory(image);
    assert(saved_image.data != NULL);
    unsigned char* data = py_newbytes(py_retval(), saved_image.size);
    memcpy(data, saved_image.data, saved_image.size);
    CUTE_PNG_FREE(saved_image.data);
    return true;
}

static bool cute_png_Image__to_png_file(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    cp_image_t* image = py_touserdata(argv);
    const char* path = py_tostr(py_arg(1));
    FILE* fp = fopen(path, "wb");
    if(fp == NULL) return OSError("cannot open file '%s' for writing", path);
    cp_saved_png_t saved_image = cp_save_png_to_memory(image);
    assert(saved_image.data != NULL);
    size_t size = fwrite(saved_image.data, saved_image.size, 1, fp);
    CUTE_PNG_FREE(saved_image.data);
    fclose(fp);
    py_newint(py_retval(), size);
    return true;
}

static bool cute_png_Image__to_rgb565_file(int argc, py_Ref argv) {
    PY_CHECK_ARGC(2);
    PY_CHECK_ARG_TYPE(1, tp_str);
    cp_image_t* image = py_touserdata(argv);
    const char* path = py_tostr(py_arg(1));

#define CONVERT_TO_RGB565(__block)                                                                 \
    size_t size = 0;                                                                               \
    for(int y = 0; y < image->h; y++) {                                                            \
        for(int x = 0; x < image->w; x++) {                                                        \
            size_t idx = y * image->w + x;                                                         \
            cp_pixel_t pixel = image->pix[idx];                                                    \
            uint16_t r = (pixel.r >> 3) & 0x1F;                                                    \
            uint16_t g = (pixel.g >> 2) & 0x3F;                                                    \
            uint16_t b = (pixel.b >> 3) & 0x1F;                                                    \
            uint16_t rgb565 = (r << 11) | (g << 5) | b;                                            \
            __block                                                                                \
        }                                                                                          \
    }

#if PY_SYS_PLATFORM == 5
    if(strcmp(path, "/dev/fb0") == 0) {
        static struct fb_fix_screeninfo finfo;
        static uint8_t* vmem_base;
        if(vmem_base == NULL) {
            int fbdev_fd = open(path, O_RDWR);
            if(fbdev_fd < 0) return OSError("open() '/dev/fb0' failed");
            fcntl(fbdev_fd, F_SETFD, fcntl(fbdev_fd, F_GETFD) | FD_CLOEXEC);
            if(ioctl(fbdev_fd, FBIOGET_FSCREENINFO, &finfo) < 0) {
                close(fbdev_fd);
                return OSError("ioctl() '/dev/fb0' failed");
            }
            vmem_base = mmap(0, finfo.smem_len, PROT_READ | PROT_WRITE, MAP_SHARED, fbdev_fd, 0);
            if(vmem_base == MAP_FAILED) {
                vmem_base = NULL;
                close(fbdev_fd);
                return OSError("mmap() '/dev/fb0' failed");
            }
            close(fbdev_fd);
        }
        CONVERT_TO_RGB565(
            // use little endian
            if(size + 2 > finfo.smem_len) {
                py_newint(py_retval(), size);
                return true;
            } memcpy(vmem_base + size, &rgb565, 2);
            size += 2;)
        py_newint(py_retval(), size);
        return true;
    }
#endif

    FILE* fp = fopen(path, "wb");
    if(fp == NULL) return OSError("cannot open file '%s' for writing", path);
    CONVERT_TO_RGB565(
        // use little endian
        size += fwrite(&rgb565, 1, 2, fp);)
    fclose(fp);
    py_newint(py_retval(), size);
    return true;
#undef CONVERT_TO_RGB565
}

void pk__add_module_cute_png() {
    py_GlobalRef mod = py_newmodule("cute_png");
    py_Type tp_image = py_newtype("Image", tp_object, mod, cute_png_Image__dtor);
    py_tpsetfinal(tp_image);

    py_bindfunc(mod, "loads", cute_png_loads);
    py_bindfunc(mod, "dumps", cute_png_dumps);

    py_bindmethod(tp_image, "__new__", cute_png_Image__new__);
    py_bindstaticmethod(tp_image, "from_bytes", cute_png_Image__from_bytes_STATIC);
    py_bindstaticmethod(tp_image, "from_file", cute_png_Image__from_file_STATIC);

    py_bindproperty(tp_image, "width", cute_png_Image__width, NULL);
    py_bindproperty(tp_image, "height", cute_png_Image__height, NULL);

    py_bindmethod(tp_image, "setpixel", cute_png_Image__setpixel);
    py_bindmethod(tp_image, "getpixel", cute_png_Image__getpixel);
    py_bindmethod(tp_image, "clear", cute_png_Image__clear);

    py_bindmethod(tp_image, "to_png_bytes", cute_png_Image__to_png_bytes);
    py_bindmethod(tp_image, "to_png_file", cute_png_Image__to_png_file);
    py_bindmethod(tp_image, "to_rgb565_file", cute_png_Image__to_rgb565_file);
}