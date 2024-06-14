#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef struct c11_userdata{
    void* _0;
    void* _1;
} c11_userdata;

void c11_userdata__ctor(c11_userdata* self, void* ptr, int size);
#define c11_userdata__as(T, self) (*( (T*)(self) ))

#ifdef __cplusplus
}

namespace pkpy{
    struct any: c11_userdata{
        template<typename T>
        any(T value){
            c11_userdata__ctor(this, &value, sizeof(T));
        }

        any(){ }

        template<typename T>
        T as(){
            return c11_userdata__as(T, this);
        }
    };
}   // namespace pkpy
#endif