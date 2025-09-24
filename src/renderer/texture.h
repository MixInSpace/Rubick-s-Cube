#ifndef TEXTURE_H
#define TEXTURE_H

#include <stdbool.h>
#include <GL/glew.h>

typedef enum {
    TEXTURE_STICKER_CENTER,
    TEXTURE_STICKER,
    TEXTURE_COUNT
} TextureType;

typedef struct {
    GLuint id;
    int width;
    int height;
    int channels;
} Texture;

bool texture_init_system(void);

void texture_cleanup_system(void);

Texture* texture_get(TextureType type);

void texture_bind(Texture* texture, GLuint unit);

void texture_unbind(GLuint unit);

#endif // TEXTURE_H 