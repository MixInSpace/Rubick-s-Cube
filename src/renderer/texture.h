#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdbool.h>
#include <GL/glew.h>

// Texture types enum
typedef enum {
    TEXTURE_STICKER_CENTER,
    TEXTURE_STICKER,
    TEXTURE_COUNT
} TextureType;

// Texture structure
typedef struct {
    GLuint id;          // OpenGL texture ID
    int width;          // Texture width
    int height;         // Texture height
    int channels;       // Number of color channels
} Texture;

// Initialize the texture system and load all textures
bool texture_init_system(void);

// Clean up the texture system
void texture_cleanup_system(void);

// Get a texture by type
Texture* texture_get(TextureType type);

// Bind a texture to a texture unit
void texture_bind(Texture* texture, GLuint unit);

// Unbind a texture from a texture unit
void texture_unbind(GLuint unit);

#endif // TEXTURE_H 