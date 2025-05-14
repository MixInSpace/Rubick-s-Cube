#include "mesh.h"
#include "../resources/resource_manager.h"
#include <stdio.h>
#include <stdlib.h>

// Forward declarations for helper functions
static bool is_corner_piece(int x, int y, int z);
static bool is_edge_piece(int x, int y, int z);
static bool is_center_piece(int x, int y, int z);

bool mesh_init(Mesh* mesh) {
    // Generate VAO, VBO, and EBO
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);
    
    // Initialize default values
    mesh->hasTexture = false;
    mesh->textureType = 0;
    
    return true;
}

void mesh_bind(Mesh* mesh) {
    glBindVertexArray(mesh->VAO);
}

void mesh_unbind(void) {
    glBindVertexArray(0);
}

void mesh_draw(Mesh* mesh) {
    mesh_bind(mesh);
    
    // If this mesh has a texture, bind it
    if (mesh->hasTexture) {
        Texture* texture = resource_manager_get_texture(mesh->textureType);
        if (texture) {
            texture_bind(texture, 0);
        }
    }
    
    if (mesh->indexCount > 0) {
        glDrawElements(mesh->drawMode, mesh->indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(mesh->drawMode, 0, mesh->vertexCount);
    }
    
    // If we bound a texture, unbind it
    if (mesh->hasTexture) {
        texture_unbind(0);
    }
    
    mesh_unbind();
}

void mesh_destroy(Mesh* mesh) {
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->EBO);
    
    mesh->vertexCount = 0;
    mesh->indexCount = 0;
}

Mesh create_cube_mesh(void) {
    Mesh mesh;
    mesh_init(&mesh);
    
    // Cube vertex data: positions (x,y,z) and colors (r,g,b)
    float vertices[] = {
        // Front face - Red (Rubik's standard)
        -0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // 0
         0.5f, -0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // 1
         0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // 2
        -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, 0.0f,  // 3
        
        // Back face - Orange (Rubik's standard)
        -0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  // 4
         0.5f, -0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  // 5
         0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  // 6
        -0.5f,  0.5f, -0.5f,  1.0f, 0.5f, 0.0f,  // 7
        
        // Top face - White (Rubik's standard)
        -0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  // 8 (same as 3)
         0.5f,  0.5f,  0.5f,  1.0f, 1.0f, 1.0f,  // 9 (same as 2)
         0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  // 10 (same as 6)
        -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, 1.0f,  // 11 (same as 7)
        
        // Bottom face - Yellow (Rubik's standard)
        -0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // 12 (same as 0)
         0.5f, -0.5f,  0.5f,  1.0f, 1.0f, 0.0f,  // 13 (same as 1)
         0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  // 14 (same as 5)
        -0.5f, -0.5f, -0.5f,  1.0f, 1.0f, 0.0f,  // 15 (same as 4)
        
        // Right face - Blue (Rubik's standard)
         0.5f, -0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // 16 (same as 1)
         0.5f, -0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  // 17 (same as 5)
         0.5f,  0.5f, -0.5f,  0.0f, 0.0f, 1.0f,  // 18 (same as 6)
         0.5f,  0.5f,  0.5f,  0.0f, 0.0f, 1.0f,  // 19 (same as 2)
        
        // Left face - Green (Rubik's standard)
        -0.5f, -0.5f, -0.5f,  0.0f, 0.8f, 0.0f,  // 20 (same as 4)
        -0.5f, -0.5f,  0.5f,  0.0f, 0.8f, 0.0f,  // 21 (same as 0)
        -0.5f,  0.5f,  0.5f,  0.0f, 0.8f, 0.0f,  // 22 (same as 3)
        -0.5f,  0.5f, -0.5f,  0.0f, 0.8f, 0.0f   // 23 (same as 7)
    };
    
    // Cube indices (triangles)
    unsigned int indices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,
        
        // Back face
        4, 5, 6,
        6, 7, 4,
        
        // Top face
        8, 9, 10,
        10, 11, 8,
        
        // Bottom face
        12, 13, 14,
        14, 15, 12,
        
        // Right face
        16, 17, 18,
        18, 19, 16,
        
        // Left face
        20, 21, 22,
        22, 23, 20
    };
    
    mesh.vertexCount = 24;  // 6 faces, 4 vertices per face
    mesh.indexCount = 36;   // 12 triangles, 3 indices each
    mesh.drawMode = GL_TRIANGLES;
    
    // Bind the Vertex Array Object first
    glBindVertexArray(mesh.VAO);
    
    // Bind and set vertex buffer(s)
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind to prevent accidental modification
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return mesh;
}

// Create a cube with specified visible faces (others will be black)
Mesh create_cube_mesh_with_visible_faces(unsigned int visibleFaces) {
    Mesh mesh;
    mesh_init(&mesh);
    
    // Color definitions
    // Standard colors for visible faces
    float red[3] = {1.0f, 0.0f, 0.0f};        // Front face - Red
    float orange[3] = {1.0f, 0.5f, 0.0f};     // Back face - Orange
    float white[3] = {1.0f, 1.0f, 1.0f};      // Top face - White
    float yellow[3] = {1.0f, 1.0f, 0.0f};     // Bottom face - Yellow
    float blue[3] = {0.0f, 0.0f, 1.0f};       // Right face - Blue
    float green[3] = {0.0f, 0.8f, 0.0f};      // Left face - Green
    float black[3] = {0.05f, 0.05f, 0.05f};   // Hidden faces - Almost black (not pure black)
    
    // Colors for each face based on visibility
    float* frontColor = (visibleFaces & FACE_FRONT) ? red : black;
    float* backColor = (visibleFaces & FACE_BACK) ? orange : black;
    float* topColor = (visibleFaces & FACE_TOP) ? white : black;
    float* bottomColor = (visibleFaces & FACE_BOTTOM) ? yellow : black;
    float* rightColor = (visibleFaces & FACE_RIGHT) ? blue : black;
    float* leftColor = (visibleFaces & FACE_LEFT) ? green : black;
    
    // Cube vertex data: positions (x,y,z) and colors (r,g,b)
    float vertices[] = {
        // Front face - Red
        -0.5f, -0.5f,  0.5f,  frontColor[0], frontColor[1], frontColor[2],  // 0
         0.5f, -0.5f,  0.5f,  frontColor[0], frontColor[1], frontColor[2],  // 1
         0.5f,  0.5f,  0.5f,  frontColor[0], frontColor[1], frontColor[2],  // 2
        -0.5f,  0.5f,  0.5f,  frontColor[0], frontColor[1], frontColor[2],  // 3
        
        // Back face - Orange
        -0.5f, -0.5f, -0.5f,  backColor[0], backColor[1], backColor[2],  // 4
         0.5f, -0.5f, -0.5f,  backColor[0], backColor[1], backColor[2],  // 5
         0.5f,  0.5f, -0.5f,  backColor[0], backColor[1], backColor[2],  // 6
        -0.5f,  0.5f, -0.5f,  backColor[0], backColor[1], backColor[2],  // 7
        
        // Top face - White
        -0.5f,  0.5f,  0.5f,  topColor[0], topColor[1], topColor[2],  // 8
         0.5f,  0.5f,  0.5f,  topColor[0], topColor[1], topColor[2],  // 9
         0.5f,  0.5f, -0.5f,  topColor[0], topColor[1], topColor[2],  // 10
        -0.5f,  0.5f, -0.5f,  topColor[0], topColor[1], topColor[2],  // 11
        
        // Bottom face - Yellow
        -0.5f, -0.5f,  0.5f,  bottomColor[0], bottomColor[1], bottomColor[2],  // 12
         0.5f, -0.5f,  0.5f,  bottomColor[0], bottomColor[1], bottomColor[2],  // 13
         0.5f, -0.5f, -0.5f,  bottomColor[0], bottomColor[1], bottomColor[2],  // 14
        -0.5f, -0.5f, -0.5f,  bottomColor[0], bottomColor[1], bottomColor[2],  // 15
        
        // Right face - Blue
         0.5f, -0.5f,  0.5f,  rightColor[0], rightColor[1], rightColor[2],  // 16
         0.5f, -0.5f, -0.5f,  rightColor[0], rightColor[1], rightColor[2],  // 17
         0.5f,  0.5f, -0.5f,  rightColor[0], rightColor[1], rightColor[2],  // 18
         0.5f,  0.5f,  0.5f,  rightColor[0], rightColor[1], rightColor[2],  // 19
        
        // Left face - Green
        -0.5f, -0.5f, -0.5f,  leftColor[0], leftColor[1], leftColor[2],  // 20
        -0.5f, -0.5f,  0.5f,  leftColor[0], leftColor[1], leftColor[2],  // 21
        -0.5f,  0.5f,  0.5f,  leftColor[0], leftColor[1], leftColor[2],  // 22
        -0.5f,  0.5f, -0.5f,  leftColor[0], leftColor[1], leftColor[2]   // 23
    };
    
    // Cube indices (triangles)
    unsigned int indices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,
        
        // Back face
        4, 5, 6,
        6, 7, 4,
        
        // Top face
        8, 9, 10,
        10, 11, 8,
        
        // Bottom face
        12, 13, 14,
        14, 15, 12,
        
        // Right face
        16, 17, 18,
        18, 19, 16,
        
        // Left face
        20, 21, 22,
        22, 23, 20
    };
    
    mesh.vertexCount = 24;  // 6 faces, 4 vertices per face
    mesh.indexCount = 36;   // 12 triangles, 3 indices each
    mesh.drawMode = GL_TRIANGLES;
    
    // Bind the Vertex Array Object first
    glBindVertexArray(mesh.VAO);
    
    // Bind and set vertex buffer(s)
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Unbind to prevent accidental modification
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return mesh;
}

// Create a cube with specified visible faces and texture support
Mesh create_textured_cube_mesh(unsigned int visibleFaces) {
    Mesh mesh;
    mesh_init(&mesh);
    
    // Color definitions
    // Standard colors for visible faces
    float red[3] = {1.0f, 0.0f, 0.0f};        // Front face - Red
    float orange[3] = {1.0f, 0.5f, 0.0f};     // Back face - Orange
    float white[3] = {1.0f, 1.0f, 1.0f};      // Top face - White
    float yellow[3] = {1.0f, 1.0f, 0.0f};     // Bottom face - Yellow
    float blue[3] = {0.0f, 0.0f, 1.0f};       // Right face - Blue
    float green[3] = {0.0f, 0.8f, 0.0f};      // Left face - Green
    float black[3] = {0.05f, 0.05f, 0.05f};   // Hidden faces - Almost black (not pure black)
    
    // Colors for each face based on visibility
    float* frontColor = (visibleFaces & FACE_FRONT) ? red : black;
    float* backColor = (visibleFaces & FACE_BACK) ? orange : black;
    float* topColor = (visibleFaces & FACE_TOP) ? white : black;
    float* bottomColor = (visibleFaces & FACE_BOTTOM) ? yellow : black;
    float* rightColor = (visibleFaces & FACE_RIGHT) ? blue : black;
    float* leftColor = (visibleFaces & FACE_LEFT) ? green : black;
    
    // Cube vertex data: positions (x,y,z), colors (r,g,b), and texture coordinates (s,t)
    // Each vertex has 8 floats: 3 for position, 3 for color, 2 for texture coordinates
    float vertices[] = {
        // Front face - standard orientation (bottom-left origin)
        -0.5f, -0.5f,  0.5f,  frontColor[0], frontColor[1], frontColor[2],  0.0f, 0.0f,  // 0
        0.5f, -0.5f,  0.5f,  frontColor[0], frontColor[1], frontColor[2],  1.0f, 0.0f,  // 1
        0.5f,  0.5f,  0.5f,  frontColor[0], frontColor[1], frontColor[2],  1.0f, 1.0f,  // 2
        -0.5f,  0.5f,  0.5f,  frontColor[0], frontColor[1], frontColor[2],  0.0f, 1.0f,  // 3
        
        // Back face - standard orientation without rotation
        -0.5f, -0.5f, -0.5f,  backColor[0], backColor[1], backColor[2],  0.0f, 0.0f,  // 4
        0.5f, -0.5f, -0.5f,  backColor[0], backColor[1], backColor[2],  1.0f, 0.0f,  // 5
        0.5f,  0.5f, -0.5f,  backColor[0], backColor[1], backColor[2],  1.0f, 1.0f,  // 6
        -0.5f,  0.5f, -0.5f,  backColor[0], backColor[1], backColor[2],  0.0f, 1.0f,  // 7
        
        // Top face - standard orientation
        -0.5f,  0.5f,  0.5f,  topColor[0], topColor[1], topColor[2],  0.0f, 0.0f,  // 8
        0.5f,  0.5f,  0.5f,  topColor[0], topColor[1], topColor[2],  1.0f, 0.0f,  // 9
        0.5f,  0.5f, -0.5f,  topColor[0], topColor[1], topColor[2],  1.0f, 1.0f,  // 10
        -0.5f,  0.5f, -0.5f,  topColor[0], topColor[1], topColor[2],  0.0f, 1.0f,  // 11
        
        // Bottom face - standard orientation without rotation
        -0.5f, -0.5f,  0.5f,  bottomColor[0], bottomColor[1], bottomColor[2],  0.0f, 0.0f,  // 12
        0.5f, -0.5f,  0.5f,  bottomColor[0], bottomColor[1], bottomColor[2],  1.0f, 0.0f,  // 13
        0.5f, -0.5f, -0.5f,  bottomColor[0], bottomColor[1], bottomColor[2],  1.0f, 1.0f,  // 14
        -0.5f, -0.5f, -0.5f,  bottomColor[0], bottomColor[1], bottomColor[2],  0.0f, 1.0f,  // 15
        
        // Right face - standard orientation
        0.5f, -0.5f,  0.5f,  rightColor[0], rightColor[1], rightColor[2],  0.0f, 0.0f,  // 16
        0.5f, -0.5f, -0.5f,  rightColor[0], rightColor[1], rightColor[2],  1.0f, 0.0f,  // 17
        0.5f,  0.5f, -0.5f,  rightColor[0], rightColor[1], rightColor[2],  1.0f, 1.0f,  // 18
        0.5f,  0.5f,  0.5f,  rightColor[0], rightColor[1], rightColor[2],  0.0f, 1.0f,  // 19
        
        // Left face - standard orientation
        -0.5f, -0.5f, -0.5f,  leftColor[0], leftColor[1], leftColor[2],  0.0f, 0.0f,  // 20
        -0.5f, -0.5f,  0.5f,  leftColor[0], leftColor[1], leftColor[2],  1.0f, 0.0f,  // 21
        -0.5f,  0.5f,  0.5f,  leftColor[0], leftColor[1], leftColor[2],  1.0f, 1.0f,  // 22
        -0.5f,  0.5f, -0.5f,  leftColor[0], leftColor[1], leftColor[2],  0.0f, 1.0f   // 23
    };
    
    // Cube indices (triangles)
    unsigned int indices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,
        
        // Back face
        4, 5, 6,
        6, 7, 4,
        
        // Top face
        8, 9, 10,
        10, 11, 8,
        
        // Bottom face
        12, 13, 14,
        14, 15, 12,
        
        // Right face
        16, 17, 18,
        18, 19, 16,
        
        // Left face
        20, 21, 22,
        22, 23, 20
    };
    
    mesh.vertexCount = 24;  // 6 faces, 4 vertices per face
    mesh.indexCount = 36;   // 12 triangles, 3 indices each
    mesh.drawMode = GL_TRIANGLES;
    mesh.hasTexture = true;
    
    // Assign appropriate texture based on position
    if (visibleFaces & FACE_FRONT || visibleFaces & FACE_BACK || 
        visibleFaces & FACE_LEFT || visibleFaces & FACE_RIGHT) {
        // Center pieces
        mesh.textureType = TEXTURE_STICKER_CENTER;
    } else if (visibleFaces & FACE_TOP || visibleFaces & FACE_BOTTOM) {
        // Edge pieces
        mesh.textureType = TEXTURE_STICKER_EDGE_U;
    } else {
        // Default
        mesh.textureType = TEXTURE_STICKER_BLANK;
    }
    
    // Bind the Vertex Array Object first
    glBindVertexArray(mesh.VAO);
    
    // Bind and set vertex buffer(s)
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Unbind to prevent accidental modification
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return mesh;
}

// Create a 3x3x3 grid of cubes with gaps between them (like a Rubik's cube)
Mesh* create_rubiks_cube_mesh(float gap) {
    // Allocate memory for 27 cubes (3x3x3 grid)
    Mesh* cubes = (Mesh*)malloc(27 * sizeof(Mesh));
    if (!cubes) {
        fprintf(stderr, "Failed to allocate memory for Rubik's cube\n");
        return NULL;
    }
    
    // Size of each subcube (scaled down to make the complete cube look better)
    float size = 0.3f; 
    
    // Total size of one cube plus the gap
    float step = size + gap;
    
    // Starting offset to center the 3x3x3 grid at origin
    // For a 3x3x3 grid, we need to offset by -step for the first cube
    float offset = -step;
    
    int index = 0;
    
    // Create a 3x3x3 grid of cubes
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                // Determine which faces should be visible based on position
                unsigned int visibleFaces = 0;
                
                // Front face is visible for cubes at z=2 (front layer)
                if (z == 2) visibleFaces |= FACE_FRONT;
                
                // Back face is visible for cubes at z=0 (back layer)
                if (z == 0) visibleFaces |= FACE_BACK;
                
                // Top face is visible for cubes at y=2 (top layer)
                if (y == 2) visibleFaces |= FACE_TOP;
                
                // Bottom face is visible for cubes at y=0 (bottom layer)
                if (y == 0) visibleFaces |= FACE_BOTTOM;
                
                // Right face is visible for cubes at x=2 (right layer)
                if (x == 2) visibleFaces |= FACE_RIGHT;
                
                // Left face is visible for cubes at x=0 (left layer)
                if (x == 0) visibleFaces |= FACE_LEFT;
                
                // Create a cube with only the visible faces colored normally
                // (internal faces will be black)
                cubes[index] = create_cube_mesh_with_visible_faces(visibleFaces);
                
                // Store position for this cube
                float posX = offset + x * step;
                float posY = offset + y * step;
                float posZ = offset + z * step;
                
                // This position info will be used in scene_render to position each cube
                // with a model matrix
                
                index++;
            }
        }
    }
    
    return cubes;
}

// Create a 3x3x3 grid of textured cubes (like a Rubik's cube with stickers)
Mesh* create_textured_rubiks_cube_mesh(float gap) {
    // Allocate memory for 27 cubes (3x3x3 grid)
    Mesh* cubes = (Mesh*)malloc(27 * sizeof(Mesh));
    if (!cubes) {
        fprintf(stderr, "Failed to allocate memory for textured Rubik's cube\n");
        return NULL;
    }
    
    // Size of each subcube (scaled down to make the complete cube look better)
    float size = 0.3f; 
    
    // Total size of one cube plus the gap
    float step = size + gap;
    
    // Starting offset to center the 3x3x3 grid at origin
    float offset = -step;
    
    int index = 0;
    
    // Create a 3x3x3 grid of cubes
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                // Determine which faces should be visible based on position
                unsigned int visibleFaces = 0;
                
                // Front face is visible for cubes at z=2 (front layer)
                if (z == 2) visibleFaces |= FACE_FRONT;
                
                // Back face is visible for cubes at z=0 (back layer)
                if (z == 0) visibleFaces |= FACE_BACK;
                
                // Top face is visible for cubes at y=2 (top layer)
                if (y == 2) visibleFaces |= FACE_TOP;
                
                // Bottom face is visible for cubes at y=0 (bottom layer)
                if (y == 0) visibleFaces |= FACE_BOTTOM;
                
                // Right face is visible for cubes at x=2 (right layer)
                if (x == 2) visibleFaces |= FACE_RIGHT;
                
                // Left face is visible for cubes at x=0 (left layer)
                if (x == 0) visibleFaces |= FACE_LEFT;
                
                // Create a textured cube with only the visible faces enabled
                cubes[index] = create_textured_cube_mesh(visibleFaces);
                
                // Select texture type based on cube position
                if (is_corner_piece(x, y, z)) {
                    cubes[index].textureType = TEXTURE_STICKER_CORNER;
                } else if (is_edge_piece(x, y, z)) {
                    // Select edge texture based on position
                    if (y == 0 || y == 2) {
                        cubes[index].textureType = TEXTURE_STICKER_EDGE_D;
                    } else if (x == 0 || x == 2) {
                        cubes[index].textureType = TEXTURE_STICKER_EDGE_L;
                    } else {
                        cubes[index].textureType = TEXTURE_STICKER_EDGE_R;
                    }
                } else if (is_center_piece(x, y, z)) {
                    cubes[index].textureType = TEXTURE_STICKER_CENTER;
                } else {
                    cubes[index].textureType = TEXTURE_STICKER_BLANK;
                }
                
                index++;
            }
        }
    }
    
    return cubes;
}

// Helper function to determine if a cube is a corner piece
static bool is_corner_piece(int x, int y, int z) {
    return (x == 0 || x == 2) && (y == 0 || y == 2) && (z == 0 || z == 2);
}

// Helper function to determine if a cube is an edge piece
static bool is_edge_piece(int x, int y, int z) {
    int extremeCount = 0;
    if (x == 0 || x == 2) extremeCount++;
    if (y == 0 || y == 2) extremeCount++;
    if (z == 0 || z == 2) extremeCount++;
    return extremeCount == 2;
}

// Helper function to determine if a cube is a center piece
static bool is_center_piece(int x, int y, int z) {
    int extremeCount = 0;
    if (x == 0 || x == 2) extremeCount++;
    if (y == 0 || y == 2) extremeCount++;
    if (z == 0 || z == 2) extremeCount++;
    return extremeCount == 1;
} 