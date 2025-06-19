#ifndef SHADER_H
#define SHADER_H

#include <GL/glew.h>
#include <stdbool.h>
#include "../math/mat4.h"

typedef struct {
    GLuint id;
} Shader;

bool shader_init_from_source (Shader* shader, const char* vertexSource, const char* fragmentSource);
void shader_use              (Shader* shader);
void shader_destroy          (Shader* shader);
void shader_set_bool         (Shader* shader, const char* name, bool value);
void shader_set_int          (Shader* shader, const char* name, int value);
void shader_set_float        (Shader* shader, const char* name, float value);
void shader_set_mat4         (Shader* shader, const char* name, Mat4 value);

#endif /* SHADER_H */ 