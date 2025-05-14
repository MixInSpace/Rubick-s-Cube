#include "mat4.h"
#include <math.h>
#include <string.h>

Mat4 mat4_identity(void) {
    Mat4 result;
    memset(result.elements, 0, sizeof(float) * 16);
    
    // Set diagonal to 1
    result.elements[0] = 1.0f;
    result.elements[5] = 1.0f;
    result.elements[10] = 1.0f;
    result.elements[15] = 1.0f;
    
    return result;
}

Mat4 mat4_translation(float x, float y, float z) {
    Mat4 result = mat4_identity();
    
    result.elements[12] = x;
    result.elements[13] = y;
    result.elements[14] = z;
    
    return result;
}

Mat4 mat4_translate(Mat4 mat, Vec3 translation) {
    // Create a translation matrix
    Mat4 trans = mat4_translation(translation.x, translation.y, translation.z);
    
    // Multiply the original matrix by the translation matrix
    return mat4_multiply(mat, trans);
}

Mat4 mat4_rotation_x(float radians) {
    Mat4 result = mat4_identity();
    
    float cos_result = cosf(radians);
    float sin_result = sinf(radians);
    
    result.elements[5] = cos_result;
    result.elements[6] = sin_result;
    result.elements[9] = -sin_result;
    result.elements[10] = cos_result;
    
    return result;
}

Mat4 mat4_rotation_y(float radians) {
    Mat4 result = mat4_identity();
    
    float cos_result = cosf(radians);
    float sin_result = sinf(radians);
    
    result.elements[0] = cos_result;
    result.elements[2] = -sin_result;
    result.elements[8] = sin_result;
    result.elements[10] = cos_result;
    
    return result;
}

Mat4 mat4_rotation_z(float radians) {
    Mat4 result = mat4_identity();
    
    float cos_result = cosf(radians);
    float sin_result = sinf(radians);
    
    result.elements[0] = cos_result;
    result.elements[1] = sin_result;
    result.elements[4] = -sin_result;
    result.elements[5] = cos_result;
    
    return result;
}

Mat4 mat4_scale(float x, float y, float z) {
    Mat4 result = mat4_identity();
    
    result.elements[0] = x;
    result.elements[5] = y;
    result.elements[10] = z;
    
    return result;
}

Mat4 mat4_scale_vec3(Mat4 mat, Vec3 scale) {
    // Create a scaling matrix
    Mat4 scaling = mat4_scale(scale.x, scale.y, scale.z);
    
    // Multiply the original matrix by the scaling matrix
    return mat4_multiply(mat, scaling);
}

Mat4 mat4_perspective(float fov, float aspect, float near, float far) {
    Mat4 result;
    memset(result.elements, 0, sizeof(float) * 16);
    
    float tan_half_fov = tanf(fov / 2.0f);
    float range = near - far;
    
    result.elements[0] = 1.0f / (aspect * tan_half_fov);
    result.elements[5] = 1.0f / tan_half_fov;
    result.elements[10] = (near + far) / range;
    result.elements[11] = -1.0f;
    result.elements[14] = (2.0f * near * far) / range;
    
    return result;
}

Mat4 mat4_look_at(Vec3 eye, Vec3 center, Vec3 up) {
    Mat4 result;
    
    Vec3 f = vec3_normalize(vec3_sub(center, eye));
    Vec3 s = vec3_normalize(vec3_cross(f, up));
    Vec3 u = vec3_cross(s, f);
    
    result.elements[0] = s.x;
    result.elements[4] = s.y;
    result.elements[8] = s.z;
    
    result.elements[1] = u.x;
    result.elements[5] = u.y;
    result.elements[9] = u.z;
    
    result.elements[2] = -f.x;
    result.elements[6] = -f.y;
    result.elements[10] = -f.z;
    
    result.elements[12] = -vec3_dot(s, eye);
    result.elements[13] = -vec3_dot(u, eye);
    result.elements[14] = vec3_dot(f, eye);
    
    result.elements[3] = 0.0f;
    result.elements[7] = 0.0f;
    result.elements[11] = 0.0f;
    result.elements[15] = 1.0f;
    
    return result;
}

Mat4 mat4_multiply(Mat4 a, Mat4 b) {
    Mat4 result;
    
    for (int row = 0; row < 4; row++) {
        for (int col = 0; col < 4; col++) {
            float sum = 0.0f;
            for (int i = 0; i < 4; i++) {
                sum += mat4_get_element(a, row, i) * mat4_get_element(b, i, col);
            }
            mat4_set_element(&result, row, col, sum);
        }
    }
    
    return result;
}

void mat4_set_element(Mat4* mat, int row, int col, float value) {
    // Column-major order: element[col * 4 + row]
    mat->elements[col * 4 + row] = value;
}

float mat4_get_element(Mat4 mat, int row, int col) {
    // Column-major order: element[col * 4 + row]
    return mat.elements[col * 4 + row];
} 