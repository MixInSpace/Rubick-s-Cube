#include "vec3.h"
#include <math.h>

Vec3 vec3_create(float x, float y, float z) {
    Vec3 result = {x, y, z};
    return result;
}

Vec3 vec3_add(Vec3 a, Vec3 b) {
    Vec3 result = {a.x + b.x, a.y + b.y, a.z + b.z};
    return result;
}

Vec3 vec3_sub(Vec3 a, Vec3 b) {
    Vec3 result = {a.x - b.x, a.y - b.y, a.z - b.z};
    return result;
}

Vec3 vec3_mul(Vec3 a, float scalar) {
    Vec3 result = {a.x * scalar, a.y * scalar, a.z * scalar};  
    return result;
}

Vec3 vec3_div(Vec3 a, float scalar) {
    Vec3 result = {a.x / scalar, a.y / scalar, a.z / scalar};
    return result;
}

// Векторное произведение векторов
Vec3 vec3_cross(Vec3 a, Vec3 b) {
    Vec3 result;
    result.x = a.y * b.z - a.z * b.y;
    result.y = a.z * b.x - a.x * b.z;
    result.z = a.x * b.y - a.y * b.x;
    return result;
}

// Скалярное произведение
float vec3_dot(Vec3 a, Vec3 b) {
    return a.x * b.x + a.y * b.y + a.z * b.z;
}

float vec3_length(Vec3 v) {
    return sqrtf(v.x * v.x + v.y * v.y + v.z * v.z);
}

Vec3 vec3_normalize(Vec3 v) {
    float length = vec3_length(v);
    if (length < 0.0001f) {
        return vec3_create(0.0f, 0.0f, 0.0f);
    }
    Vec3 result = {v.x / length, v.y / length, v.z / length};
    return result;
}

// Точка на t*100% между векторами 
Vec3 vec3_lerp(Vec3 a, Vec3 b, float t) {
    Vec3 result = {a.x + t * (b.x - a.x), a.y + t * (b.y - a.y), a.z + t * (b.z - a.z)};
    return result;
}