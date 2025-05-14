#include "shader.h"
#include "../utils/file.h"
#include <stdio.h>
#include <stdlib.h>

bool shader_init_from_source(Shader* shader, const char* vertexSource, const char* fragmentSource) {
    // Compile shaders
    GLuint vertex, fragment;
    int success;
    char infoLog[512];
    
    // Vertex Shader
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vertexSource, NULL);
    glCompileShader(vertex);
    
    // Check for compile errors
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        return false;
    }
    
    // Fragment Shader
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fragmentSource, NULL);
    glCompileShader(fragment);
    
    // Check for compile errors
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        glDeleteShader(vertex);
        return false;
    }
    
    // Shader Program
    shader->id = glCreateProgram();
    glAttachShader(shader->id, vertex);
    glAttachShader(shader->id, fragment);
    glLinkProgram(shader->id);
    
    // Check for linking errors
    glGetProgramiv(shader->id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader->id, 512, NULL, infoLog);
        fprintf(stderr, "ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }
    
    // Delete shaders as they're linked into our program now and no longer necessary
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    return true;
}

bool shader_init(Shader* shader, const char* vertexPath, const char* fragmentPath) {
    // Read shader source code from files
    char* vertexCode = file_read_text(vertexPath);
    char* fragmentCode = file_read_text(fragmentPath);
    
    if (!vertexCode || !fragmentCode) {
        fprintf(stderr, "Failed to read shader files\n");
        free(vertexCode);
        free(fragmentCode);
        return false;
    }
    
    // Initialize from source
    bool result = shader_init_from_source(shader, vertexCode, fragmentCode);
    
    // Free allocated memory
    free(vertexCode);
    free(fragmentCode);
    
    return result;
}

void shader_use(Shader* shader) {
    glUseProgram(shader->id);
}

void shader_destroy(Shader* shader) {
    glDeleteProgram(shader->id);
}

void shader_set_bool(Shader* shader, const char* name, bool value) {
    glUniform1i(glGetUniformLocation(shader->id, name), (int)value);
}

void shader_set_int(Shader* shader, const char* name, int value) {
    glUniform1i(glGetUniformLocation(shader->id, name), value);
}

void shader_set_float(Shader* shader, const char* name, float value) {
    glUniform1f(glGetUniformLocation(shader->id, name), value);
}

void shader_set_mat4(Shader* shader, const char* name, Mat4 value) {
    glUniformMatrix4fv(glGetUniformLocation(shader->id, name), 1, GL_FALSE, value.elements);
} 