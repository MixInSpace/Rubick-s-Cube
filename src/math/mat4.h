#ifndef MAT4_H
#define MAT4_H

#include "vec3.h"
#include <stdbool.h>

typedef struct {
    float elements[16];
} Mat4;

// Создание матриц
Mat4 mat4_identity    (void);
Mat4 mat4_translation (float x, float y, float z);
Mat4 mat4_translate   (Mat4 mat, Vec3 translation);
Mat4 mat4_rotation_x  (float radians);
Mat4 mat4_rotation_y  (float radians);
Mat4 mat4_rotation_z  (float radians);
Mat4 mat4_scale       (float x, float y, float z);
Mat4 mat4_scale_vec3  (Mat4 mat, Vec3 scale);
Mat4 mat4_perspective (float fov, float aspect, float near, float far);
Mat4 mat4_look_at     (Vec3 eye, Vec3 center, Vec3 up);

// Операции над матрицами
Mat4 mat4_multiply     (Mat4 a, Mat4 b);
void mat4_set_element  (Mat4* mat, int row, int col, float value);
float mat4_get_element (Mat4 mat, int row, int col);

#endif /* MAT4_H */ 