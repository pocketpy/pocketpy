#pragma once

#include <stdint.h>

typedef union c11_vec2i {
    struct { int x, y; };
    int data[2];
    int64_t _i64;
} c11_vec2i;

typedef union c11_vec3i {
    struct { int x, y, z; };
    int data[3];
} c11_vec3i;

typedef union c11_vec2 {
    struct { float x, y; };
    float data[2];
} c11_vec2;

typedef union c11_vec3 {
    struct { float x, y, z; };
    float data[3];
} c11_vec3;

typedef union c11_mat3x3 {
    struct {
        float _11, _12, _13;
        float _21, _22, _23;
        float _31, _32, _33;
    };

    float m[3][3];
    float data[9];
} c11_mat3x3;

typedef union c11_color32 {
    struct {
        unsigned char r;
        unsigned char g;
        unsigned char b;
        unsigned char a;
    };
    unsigned char data[4];
} c11_color32;
