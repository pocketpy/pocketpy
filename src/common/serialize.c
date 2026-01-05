#include "pocketpy/common/serialize.h"

// >>> ord('🥕')
// 129365
// >>> ord('🍋')
// 127819

static uint32_t c11__checksum_32bit(const void* data, int size){
    const uint8_t* p = (const uint8_t*)data;
    uint32_t res = 0;
    for(int i = 0; i < size; i++){
        res = res * 31 + p[i];
    }
    return res;
}

void c11_serializer__ctor(c11_serializer* self, int16_t magic, int8_t major_ver, int8_t minor_ver){
    c11_vector__ctor(&self->data, 1);
    c11_serializer__write_i16(self, magic);
    c11_serializer__write_i8(self, major_ver);
    c11_serializer__write_i8(self, minor_ver);
}

void c11_serializer__dtor(c11_serializer* self){
    c11_vector__dtor(&self->data);
}

void c11_serializer__write_cstr(c11_serializer *self, const char* cstr) {
    int len = (int)strlen(cstr);
    c11_serializer__write_bytes(self, cstr, len + 1);
}

void c11_serializer__write_bytes(c11_serializer* self, const void* data, int size){
    c11_vector__extend(&self->data, data, size);
}

void* c11_serializer__submit(c11_serializer* self, int* size){
    uint32_t checksum = c11__checksum_32bit(self->data.data, self->data.length);
    c11_serializer__write_bytes(self, &checksum, sizeof(uint32_t));
    return c11_vector__submit(&self->data, size);
}

void c11_deserializer__ctor(c11_deserializer* self, const void* data, int size){
    self->data = (const uint8_t*)data;
    self->size = size;
    self->index = 0;
    self->error_msg[0] = '\0';
}

void c11_deserializer__dtor(c11_deserializer* self){
    // nothing to do
}

bool c11_deserializer__error(c11_deserializer* self, const char* msg){
    snprintf(self->error_msg, sizeof(self->error_msg), "%s", msg);
    return false;
}

bool c11_deserializer__check_header(c11_deserializer* self, int16_t magic, int8_t major_ver, int8_t minor_ver_min){
    if(self->size < 8){
        return c11_deserializer__error(self, "bad header: size < 8");
    }
    // read 16bit magic
    int16_t file_magic = c11_deserializer__read_i16(self);
    if(file_magic != magic){
        return c11_deserializer__error(self, "bad header: magic mismatch");
    }
    // read 16bit version
    self->major_ver = c11_deserializer__read_i8(self);
    self->minor_ver = c11_deserializer__read_i8(self);

    // check checksum
    uint32_t checksum;
    memcpy(&checksum, self->data + self->size - 4, sizeof(uint32_t));
    uint32_t actual_checksum = c11__checksum_32bit(self->data, self->size - 4);
    if(checksum != actual_checksum){
        return c11_deserializer__error(self, "bad header: checksum mismatch");
    }
    // check version
    if(self->major_ver != major_ver){
        return c11_deserializer__error(self, "bad header: major version mismatch");
    }
    if(self->minor_ver < minor_ver_min){
        // file_ver: 1.1, require_ver: 1.0 => ok
        // file_ver: 1.1, require_ver: 1.1 => ok
        // file_ver: 1.1, require_ver: 1.2 => error
        return c11_deserializer__error(self, "bad header: minor version mismatch");
    }
    return true;
}

const char* c11_deserializer__read_cstr(c11_deserializer* self){
    const char* p = (const char*)(self->data + self->index);
    self->index += (strlen(p) + 1);
    return p;
}

void* c11_deserializer__read_bytes(c11_deserializer* self, int size){
    void* p = (void*)(self->data + self->index);
    self->index += size;
    return p;
}