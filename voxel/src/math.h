#pragma once
#include <math.h>
#include <string.h>
#include <stdint.h>
/* Ensure math functions are declared before any inline uses */
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/* ---- vec3 ---- */
typedef struct { float x, y, z; } vec3;

static inline vec3 vec3_add(vec3 a, vec3 b)      { return (vec3){a.x+b.x, a.y+b.y, a.z+b.z}; }
static inline vec3 vec3_sub(vec3 a, vec3 b)      { return (vec3){a.x-b.x, a.y-b.y, a.z-b.z}; }
static inline vec3 vec3_scale(vec3 a, float s)   { return (vec3){a.x*s, a.y*s, a.z*s}; }
static inline float vec3_dot(vec3 a, vec3 b)     { return a.x*b.x + a.y*b.y + a.z*b.z; }
static inline float vec3_len(vec3 a)             { return sqrtf(vec3_dot(a,a)); }
static inline vec3 vec3_norm(vec3 a)             { float l=vec3_len(a); return l>1e-6f?vec3_scale(a,1.f/l):a; }
static inline vec3 vec3_cross(vec3 a, vec3 b)    {
    return (vec3){a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x};
}

/* ---- mat4 (column-major, like OpenGL) ---- */
typedef struct { float m[16]; } mat4;

static inline mat4 mat4_identity(void) {
    mat4 r; memset(&r,0,sizeof(r));
    r.m[0]=r.m[5]=r.m[10]=r.m[15]=1.f;
    return r;
}

static inline mat4 mat4_mul(mat4 a, mat4 b) {
    mat4 r; memset(&r,0,sizeof(r));
    for(int c=0;c<4;c++) for(int row=0;row<4;row++) {
        float v=0;
        for(int k=0;k<4;k++) v += a.m[k*4+row] * b.m[c*4+k];
        r.m[c*4+row] = v;
    }
    return r;
}

static inline mat4 mat4_perspective(float fovy, float aspect, float near, float far) {
    mat4 r; memset(&r,0,sizeof(r));
    float f = 1.f / tanf(fovy * 0.5f);
    r.m[0]  = f / aspect;
    r.m[5]  = f;
    r.m[10] = (far + near) / (near - far);
    r.m[11] = -1.f;
    r.m[14] = (2.f * far * near) / (near - far);
    return r;
}

static inline mat4 mat4_lookat(vec3 eye, vec3 center, vec3 up) {
    vec3 f = vec3_norm(vec3_sub(center, eye));
    vec3 s = vec3_norm(vec3_cross(f, up));
    vec3 u = vec3_cross(s, f);
    mat4 r; memset(&r,0,sizeof(r));
    r.m[0]=s.x; r.m[4]=s.y; r.m[8] =s.z;
    r.m[1]=u.x; r.m[5]=u.y; r.m[9] =u.z;
    r.m[2]=-f.x;r.m[6]=-f.y;r.m[10]=-f.z;
    r.m[12]=-vec3_dot(s,eye);
    r.m[13]=-vec3_dot(u,eye);
    r.m[14]= vec3_dot(f,eye);
    r.m[15]=1.f;
    return r;
}

static inline mat4 mat4_ortho(float l, float r2, float b, float t, float n, float f) {
    mat4 m; memset(&m,0,sizeof(m));
    m.m[0]  =  2.f/(r2-l);
    m.m[5]  =  2.f/(t-b);
    m.m[10] = -2.f/(f-n);
    m.m[12] = -(r2+l)/(r2-l);
    m.m[13] = -(t+b)/(t-b);
    m.m[14] = -(f+n)/(f-n);
    m.m[15] =  1.f;
    return m;
}

#define RAD(deg) ((deg)*3.14159265f/180.f)
