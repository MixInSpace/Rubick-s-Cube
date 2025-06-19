#ifndef MESH_H
#define MESH_H

#include <GL/glew.h>
#include <stdbool.h>

#include "texture.h"

typedef struct {
    float r, g, b;
} RGBColor;

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
    GLenum drawMode;   
    bool hasTexture;   
    TextureType textureType; 
} Mesh;

bool mesh_init        (Mesh* mesh);
void mesh_bind        (Mesh* mesh);
void mesh_unbind      (void);
void mesh_draw        (Mesh* mesh);
void mesh_destroy     (Mesh* mesh);

Mesh create_custom_colored_cube(unsigned int visibleFaces, RGBColor (*cubeColors)[9], int x, int y, int z);
#endif /* MESH_H */ 