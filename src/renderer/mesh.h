#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <stdbool.h>
#include "texture.h"

// Define faces for visibility control
typedef enum {
    FACE_FRONT  = 0x01,
    FACE_BACK   = 0x02,
    FACE_TOP    = 0x04,
    FACE_BOTTOM = 0x08,
    FACE_RIGHT  = 0x10,
    FACE_LEFT   = 0x20,
    FACE_ALL    = 0x3F
} CubeFace;

typedef struct {
    GLuint VAO;        // Vertex Array Object
    GLuint VBO;        // Vertex Buffer Object
    GLuint EBO;        // Element Buffer Object
    int vertexCount;  
    int indexCount;
    GLenum drawMode;   // e.g., GL_TRIANGLES
    bool hasTexture;   // Whether this mesh has texture coordinates
    TextureType textureType; // Type of texture to use
} Mesh;

bool mesh_init(Mesh* mesh);
void mesh_bind(Mesh* mesh);
void mesh_unbind(void);
void mesh_draw(Mesh* mesh);
void mesh_destroy(Mesh* mesh);

// Specific mesh creation functions
Mesh create_cube_mesh(void);

// Create a cube with specified visible faces (others will be black)
Mesh create_cube_mesh_with_visible_faces(unsigned int visibleFaces);

// Create a cube with specified visible faces and texture support
Mesh create_textured_cube_mesh(unsigned int visibleFaces);

// Create a 3x3x3 grid of cubes with gaps between them (like a Rubik's cube)
Mesh* create_rubiks_cube_mesh(float gap);

// Create a 3x3x3 grid of textured cubes (like a Rubik's cube with stickers)
Mesh* create_textured_rubiks_cube_mesh(float gap);

#endif /* MESH_H */ 