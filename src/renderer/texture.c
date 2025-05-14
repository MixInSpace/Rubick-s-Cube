#include "texture.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Use STB Image to load texture files
#define STB_IMAGE_IMPLEMENTATION
#include <STB/stb_image.h>
#include <GL/glew.h>

// Include embedded texture data
#include "../resources/embedded_textures.h"

// Global texture storage
static Texture textures[TEXTURE_COUNT];
static bool textures_initialized = false;

// Structure to map texture types to embedded texture data
typedef struct {
    const unsigned char* data;
    unsigned int size;
} EmbeddedTextureData;

// Array of embedded texture data - will be initialized in init function
static EmbeddedTextureData embedded_textures[TEXTURE_COUNT];

// For debugging: texture file paths (not used for loading)
static const char* texture_names[TEXTURE_COUNT] = {
    "stickerCenter.png",
    "stickerCorner.png",
    "stickerEdgeU.png",
    "stickerEdgeD.png",
    "stickerEdgeL.png",
    "stickerEdgeR.png",
    "stickerBlank.png"
};

// Initialize the embedded textures array
static void init_embedded_textures(void) {
    embedded_textures[TEXTURE_STICKER_CENTER].data = stickerCenter_png;
    embedded_textures[TEXTURE_STICKER_CENTER].size = stickerCenter_png_size;
    
    embedded_textures[TEXTURE_STICKER_CORNER].data = stickerCorner_png;
    embedded_textures[TEXTURE_STICKER_CORNER].size = stickerCorner_png_size;
    
    embedded_textures[TEXTURE_STICKER_EDGE_U].data = stickerEdgeU_png;
    embedded_textures[TEXTURE_STICKER_EDGE_U].size = stickerEdgeU_png_size;
    
    embedded_textures[TEXTURE_STICKER_EDGE_D].data = stickerEdgeD_png;
    embedded_textures[TEXTURE_STICKER_EDGE_D].size = stickerEdgeD_png_size;
    
    embedded_textures[TEXTURE_STICKER_EDGE_L].data = stickerEdgeL_png;
    embedded_textures[TEXTURE_STICKER_EDGE_L].size = stickerEdgeL_png_size;
    
    embedded_textures[TEXTURE_STICKER_EDGE_R].data = stickerEdgeR_png;
    embedded_textures[TEXTURE_STICKER_EDGE_R].size = stickerEdgeR_png_size;
    
    embedded_textures[TEXTURE_STICKER_BLANK].data = stickerBlank_png;
    embedded_textures[TEXTURE_STICKER_BLANK].size = stickerBlank_png_size;
}

// Load a single texture from embedded data
static bool load_texture(TextureType type) {
    Texture* texture = &textures[type];
    const EmbeddedTextureData* tex_data = &embedded_textures[type];
    const char* filename = texture_names[type]; // For debug messages only
    
    // Generate texture
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    // Load image data from memory
    stbi_set_flip_vertically_on_load(1); // Flip image vertically
    
    // Try to load the image from memory
    unsigned char* data = stbi_load_from_memory(
        tex_data->data, tex_data->size,
        &texture->width, &texture->height, &texture->channels, 0
    );
    
    // Get stbi error message
    const char* error_msg = stbi_failure_reason();
    
    if (!data) {
        fprintf(stderr, "Failed to load embedded texture: %s\n", filename);
        fprintf(stderr, "STB error: %s\n", error_msg ? error_msg : "Unknown error");
        glDeleteTextures(1, &texture->id);
        texture->id = 0;
        return false;
    }
    
    // Determine format based on number of channels
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
    
    // Upload texture data to GPU
    glTexImage2D(GL_TEXTURE_2D, 0, format, texture->width, texture->height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Free image data
    stbi_image_free(data);
    
    printf("Loaded embedded texture '%s' (%dx%d, %d channels)\n", filename, texture->width, texture->height, texture->channels);
    return true;
}

// Initialize the texture system
bool texture_init_system(void) {
    if (textures_initialized) {
        return true;
    }
    
    // Initialize textures array
    memset(textures, 0, sizeof(textures));
    
    // Initialize embedded textures
    init_embedded_textures();
    
    // Load all textures
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

// Clean up the texture system
void texture_cleanup_system(void) {
    if (!textures_initialized) {
        return;
    }
    
    // Delete all textures
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (textures[i].id != 0) {
            glDeleteTextures(1, &textures[i].id);
            textures[i].id = 0;
        }
    }
    
    textures_initialized = false;
    printf("Texture system cleaned up\n");
}

// Get a texture by type
Texture* texture_get(TextureType type) {
    if (!textures_initialized || type < 0 || type >= TEXTURE_COUNT) {
        return NULL;
    }
    
    return &textures[type];
}

// Bind a texture to a texture unit
void texture_bind(Texture* texture, GLuint unit) {
    if (!texture || texture->id == 0) {
        return;
    }
    
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, texture->id);
}

// Unbind a texture from a texture unit
void texture_unbind(GLuint unit) {
    glActiveTexture(GL_TEXTURE0 + unit);
    glBindTexture(GL_TEXTURE_2D, 0);
} 