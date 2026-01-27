//
// Created by jack lewis on 11/24/25.
//

#ifndef KXMATH_H
#define KXMATH_H

#include <math.h>
#include <stdint.h>

#ifndef KX_DEF_PRIMITIVES
#define KX_DEF_PRIMITIVES

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;

typedef enum     { X, Y, Z } Axis;

#endif

/* vectors */

#ifndef KX_DEF_VECTYPES
#define KX_DEF_VECTYPES

typedef struct {f32 x, y;} fvec2;
typedef struct {f32 x, y, z;} fvec3;
typedef struct {f32 x, y, z, w;} fvec4;
typedef struct {i32 x, y;} ivec2;
typedef struct {i32 x, y, z;} ivec3;
typedef struct {i32 x, y, z, w;} ivec4;
typedef struct {u32 x, y;} uvec2;
typedef struct {u32 x, y, z;} uvec3;
typedef struct {u32 x, y, z, w;} uvec4;
const fvec3 FVEC3_ZERO = {0,0,0};

#define IVEC2(a, b) (ivec2){a, b}
#endif



fvec2 fvec2_add(fvec2 a, fvec2 b);
fvec2 fvec2_sub(fvec2 a, fvec2 b);
fvec2 fvec2_multiply(fvec2 a, fvec2 b);
fvec2 fvec2_multiplyScalar(fvec2 a, f32 scalar);
fvec2 fvec2_abs(fvec2 a);
fvec2 fvec2_length(fvec2 a);
fvec2 fvec2_normalize(fvec2 a);

static int mod(int a, int b) {
    int r = a % b;
    return (r < 0) ? r + b : r;
}
typedef float mat4[16];
typedef struct {float r, g, b, a; } fcolor_rgba;


typedef struct {
    fvec3 pos;
    fvec3 rot;
    float fov;
    float aspect;
    float near, far;
} camera;

static void mat4Identity(mat4 dest) {
    const static float m[16] = {
        1,0,0,0,
        0,1,0,0,
        0,0,1,0,
        0,0,0,1
    };
    for (int i = 0; i < 16; i++) {
        dest[i] = m[i];
    }
}
static void mat4Ortho(mat4 dest, float left, float right, float bottom, float top, float nearZ, float farZ) {
    for (int i = 0; i < 16; i++) dest[i] = 0.0f;
    float rightMinusLeft = right - left;
    float topMinusBottom = top - bottom;
    float farZminusNearZ = farZ - nearZ;
    dest[0]  = 2.0f / (rightMinusLeft);
    dest[5]  = 2.0f / (topMinusBottom);
    dest[10] = -2.0f / (farZminusNearZ);
    dest[12] = - (right + left) / (rightMinusLeft);
    dest[13] = - (top + bottom) / (topMinusBottom);
    dest[14] = - (farZminusNearZ) / (farZminusNearZ);
    dest[15] = 1.0f;
}
static void mat4Multiply(mat4 dest, mat4 a, mat4 b) {
    for (int i = 0; i < 16; i++) { dest[i] = 0.0f; }
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            dest[col*4 + row] =
                a[0*4 + row] * b[col*4 + 0] +
                a[1*4 + row] * b[col*4 + 1] +
                a[2*4 + row] * b[col*4 + 2] +
                a[3*4 + row] * b[col*4 + 3];
        }
    }
}
static void mat4Translate(mat4 dest, float x, float y, float z) {
    mat4Identity(dest);
    dest[12] = x;
    dest[13] = y;
    dest[14] = z;
}
static void mat4Scale(mat4 dest, float x, float y, float z) {
    mat4Identity(dest);
    dest[0]  = x;
    dest[5]  = y;
    dest[10] = z;
}
static void mat4Proj(mat4 dest, float fovyRadians, float aspect, float nearZ, float farZ) {
    float f = 1.0f / tanf(fovyRadians / 2.0f);
    for (int i = 0; i < 16; i++) { dest[i] = 0.0f; }
    dest[0]  = f / aspect;
    dest[5]  = f;
    dest[10] = (farZ + nearZ) / (nearZ - farZ);
    dest[11] = -1.0f;
    dest[14] = (2.0f * farZ * nearZ) / (nearZ - farZ);
}
static void mat4RotateX(mat4 dest, float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    mat4Identity(dest);
    dest[5]  = c;
    dest[6]  = s;
    dest[9]  = -s;
    dest[10] = c;
}
static void mat4RotateY(mat4 dest, float radians) {
    float c = cosf(radians);
    float s = sinf(radians);
    mat4Identity(dest);
    dest[0]  = c;
    dest[2]  = -s;
    dest[8]  = s;
    dest[10] = c;
}
static void viewMatrix(mat4 * dest, camera cam) {
    mat4 m1;            mat4RotateX(m1,cam.rot.x);
    mat4 m2;            mat4RotateY(m2,cam.rot.y);
    mat4 m3;            mat4Multiply (m3, m1, m2);
    mat4 m4;            mat4Translate(m4, -cam.pos.x, -cam.pos.y, -cam.pos.z);
    mat4Multiply (*dest,m3, m4);
}
static fcolor_rgba colorFromUint(unsigned color) {
    fcolor_rgba rgba;
    rgba.a = ((color >> 24) & 0xFF) / 255.0;
    rgba.b = ((color >> 16) & 0xFF) / 255.0;
    rgba.g = ((color >> 8)  & 0xFF) / 255.0;   
    rgba.r = (color & 0xFF) / 255.0;
    return rgba;
}

#endif
