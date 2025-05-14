#include "resource_manager.h"
#include <stdio.h>
#include <stdlib.h>

// Flag to track initialization state
static bool is_initialized = false;

bool resource_manager_init(void) {
    if (is_initialized) {
        return true;
    }
    
    // Initialize the texture system
    if (!texture_init_system()) {
        fprintf(stderr, "Failed to initialize texture system\n");
        return false;
    }
    
    is_initialized = true;
    printf("Resource manager initialized\n");
    return true;
}

void resource_manager_cleanup(void) {
    if (!is_initialized) {
        return;
    }
    
    // Clean up the texture system
    texture_cleanup_system();
    
    is_initialized = false;
    printf("Resource manager cleaned up\n");
}

bool resource_manager_load_texture(TextureType type) {
    if (!is_initialized) {
        fprintf(stderr, "Resource manager not initialized\n");
        return false;
    }
    
    // The texture loading is handled by the texture system
    // This is just a wrapper for potential future expansion
    Texture* texture = texture_get(type);
    return (texture != NULL && texture->id != 0);
}

bool resource_manager_load_all_textures(void) {
    if (!is_initialized) {
        fprintf(stderr, "Resource manager not initialized\n");
        return false;
    }
    
    // In our implementation, textures are already loaded in texture_init_system()
    // This is a convenience function for future expansion
    bool success = true;
    for (int i = 0; i < TEXTURE_COUNT; i++) {
        if (!resource_manager_load_texture((TextureType)i)) {
            fprintf(stderr, "Failed to load texture %d\n", i);
            success = false;
        }
    }
    
    return success;
}

Texture* resource_manager_get_texture(TextureType type) {
    if (!is_initialized) {
        fprintf(stderr, "Resource manager not initialized\n");
        return NULL;
    }
    
    return texture_get(type);
} 