#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Используется STB Image для того чтобы загрузить файлы текстур
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>
#include <GL/glew.h>

// Текстуры в бинарном виде
#include "../resources/embedded_textures.h"

// Глобальное хранилище текстур
static Texture textures[TEXTURE_COUNT];
static bool textures_initialized = false;

// map texturetype to raw data
typedef struct {
    const unsigned char* data;
    unsigned int size;
} EmbeddedTextureData;

// массив данных
static EmbeddedTextureData embedded_textures[TEXTURE_COUNT];

static const char* texture_names[TEXTURE_COUNT] = {
    "stickerCenter.png",
    "sticker.png"
};

static void init_embedded_textures(void) {
    embedded_textures[TEXTURE_STICKER_CENTER].data = stickerCenter_png;
    embedded_textures[TEXTURE_STICKER_CENTER].size = stickerCenter_png_size;
    
    embedded_textures[TEXTURE_STICKER].data = sticker_png;
    embedded_textures[TEXTURE_STICKER].size = sticker_png_size;
}

// Загрузить текстуру из raw_data в массив
static bool load_texture(TextureType type) {
    Texture* texture = &textures[type];
    const EmbeddedTextureData* tex_data = &embedded_textures[type];
    const char* filename = texture_names[type]; 
    
    // Создаем текстуру
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    
    // Парметры текстуры
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    stbi_set_flip_vertically_on_load(1); 
    
    // Загружаем из пемяти библиотекой
    unsigned char* data = stbi_load_from_memory(
        tex_data->data, tex_data->size,
        &texture->width, &texture->height, &texture->channels, 0
    );
    
    // Ошибки
    const char* error_msg = stbi_failure_reason();
    
    if (!data) {
        fprintf(stderr, "Failed to load embedded texture: %s\n", filename);
        fprintf(stderr, "STB error: %s\n", error_msg ? error_msg : "Unknown error");
        glDeleteTextures(1, &texture->id);
        texture->id = 0;
        return false;
    }
    
    // Определение формата на основе числа каналов
    GLenum format;
    switch (texture->channels) {
        case 1:
            format = GL_RED;
            break;
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
        default:
            fprintf(stderr, "Unsupported channel format (%d) in texture: %s\n", texture->channels, filename);
            stbi_image_free(data);
            glDeleteTextures(1, &texture->id);
            texture->id = 0;
            return false;
    }
    
    // Загрузить данные о текстурах на GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    //  Освобоить данные
    stbi_image_free(data);
    
    printf("Loaded embedded texture '%s' (%dx%d, %d channels)\n", filename, texture->width, texture->height, texture->channels);
    return true;
}


bool texture_init_system(void) {
    if (textures_initialized) {
        return true;
    }
    
    memset(textures, 0, sizeof(textures));
    
    init_embedded_textures();
    
    int loaded_count = 0;
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (load_texture(i)) {
            loaded_count++;
        }
    }
    
    textures_initialized = true;
    printf("Texture system initialized: %d/%d textures loaded\n", loaded_count, TEXTURE_COUNT);
    return loaded_count == TEXTURE_COUNT;
}

void texture_cleanup_system(void) {
    if (!textures_initialized) {
        return;
    }
    
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (textures[i].id != 0) {
            glDeleteTextures(1, &textures[i].id);
            textures[i].id = 0;
        }
    }
    
    textures_initialized = false;
    printf("Texture system cleaned up\n");
}

Texture* texture_get(TextureType type) {
    if (!textures_initialized || type < 0 || type >= TEXTURE_COUNT) {
        return NULL;
    }
    
    return &textures[type];
}

void texture_bind(Texture* texture, GLuint unit) {
    if (!texture || texture->id == 0) {
        return;
    }
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

void texture_unbind(GLuint unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
} 