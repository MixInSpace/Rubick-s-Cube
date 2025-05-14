#ifndef RESOURCE_MANAGER_H
#define RESOURCE_MANAGER_H

#include <stdbool.h>
#include "../renderer/texture.h"

// Resource types will be added here in the future
// Examples: Texture, Model, etc.

// Initialize the resource manager
bool resource_manager_init(void);

// Clean up and free all resources
void resource_manager_cleanup(void);

// Load a specific texture
bool resource_manager_load_texture(TextureType type);

// Load all textures
bool resource_manager_load_all_textures(void);

// Get a texture by type
Texture* resource_manager_get_texture(TextureType type);

// Functions to load and manage resources will be added here
// Examples: load_texture, etc.

#endif /* RESOURCE_MANAGER_H */