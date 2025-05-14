#include "vec2.h"
#include <math.h>

Vec2 vec2_create(float x, float y) {
    Vec2 result = {x, y};
    return result;
}

Vec2 vec2_add(Vec2 a, Vec2 b) {
    Vec2 result = {a.x + b.x, a.y + b.y};
    return result;
}

Vec2 vec2_sub(Vec2 a, Vec2 b) {
    Vec2 result = {a.x - b.x, a.y - b.y};
    return result;
}

Vec2 vec2_mul(Vec2 a, float scalar) {
    Vec2 result = {a.x * scalar, a.y * scalar};
    return result;
}

float vec2_dot(Vec2 a, Vec2 b) {
    return a.x * b.x + a.y * b.y;
}

float vec2_length(Vec2 v) {
    return sqrtf(v.x * v.x + v.y * v.y);
}

Vec2 vec2_normalize(Vec2 v) {
    float length = vec2_length(v);
    if (length < 0.0001f) {
        return vec2_create(0.0f, 0.0f);
    }
    Vec2 result = {v.x / length, v.y / length};
    return result;
} 