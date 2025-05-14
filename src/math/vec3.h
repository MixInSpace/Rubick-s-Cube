#ifndef VEC3_H
#define VEC3_H

#include <math.h>


typedef struct {
    float x, y, z;
} Vec3;

Vec3 vec3_create(float x, float y, float z);
Vec3 vec3_add(Vec3 a, Vec3 b);
Vec3 vec3_sub(Vec3 a, Vec3 b);
Vec3 vec3_mul(Vec3 a, float scalar);
Vec3 vec3_div(Vec3 a, float scalar);
Vec3 vec3_cross(Vec3 a, Vec3 b);
float vec3_dot(Vec3 a, Vec3 b);
float vec3_length(Vec3 v);
Vec3 vec3_normalize(Vec3 v);
Vec3 vec3_lerp(Vec3 a, Vec3 b, float t);

#endif /* VEC3_H */