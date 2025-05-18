#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>
#include "../renderer/shader.h"
#include "../renderer/mesh.h"
#include "../math/mat4.h"
#include "../types.h"

// Color mapping for Rubik's cube faces
typedef enum {
    CUBE_COLOR_WHITE = 'W',   // Top face (U)
    CUBE_COLOR_YELLOW = 'Y',  // Bottom face (D)
    CUBE_COLOR_RED = 'R',     // Front face (F)
    CUBE_COLOR_ORANGE = 'O',  // Back face (B)
    CUBE_COLOR_BLUE = 'B',    // Right face (R)
    CUBE_COLOR_GREEN = 'G'    // Left face (L)
} CubeColor;

// Face indices for a Rubik's cube
typedef enum {
    FACE_IDX_TOP = 0,    // U - White
    FACE_IDX_FRONT = 1,  // F - Red
    FACE_IDX_RIGHT = 2,  // R - Blue 
    FACE_IDX_BACK = 3,   // B - Orange
    FACE_IDX_LEFT = 4,   // L - Green
    FACE_IDX_BOTTOM = 5  // D - Yellow
} FaceIndex;

// Rotation direction
typedef enum {
    ROTATE_CLOCKWISE = 1,
    ROTATE_COUNTERCLOCKWISE = -1
} RotationDirection;

// Struct to store RGB color values
typedef struct {
    float r, g, b;
} RGBColor;

struct Scene {
    int id;
    Shader shader;
    
    // For single cube mode
    Mesh cube;
    
    // For Rubik's cube mode
    Mesh* cubes;
    int numCubes;
    bool isRubiksCube;
    float positions[27][3]; // Positions for each cube in the Rubik's grid
    
    // Color data for faces
    RGBColor cubeColors[6][9]; // [face][facelet] RGB colors
    
    // Rotation animation
    bool isRotating;
    float rotationAngle;
    float rotationTarget;
    FaceIndex rotatingFace;
    RotationDirection rotationDirection;
    int rotatingLayer;     // 0=bottom/left/back, 1=middle, 2=top/right/front
    char rotationAxis;     // 'x', 'y', or 'z'
};

bool scene_init(Scene* scene);
void scene_update(Scene* scene, float deltaTime);
void scene_render(Scene* scene, Window* window);
void scene_destroy(Scene* scene);

// Initialize the scene with a Rubik's cube grid
bool scene_init_rubiks(Scene* scene);

// Set the Rubik's cube state from a string
// Format: "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
// where:
// U = top face (white)
// R = right face (blue)
// F = front face (red)
// D = bottom face (yellow)
// L = left face (green)
// B = back face (orange)
// Each face should have 9 characters in reading order (left-to-right, top-to-bottom)
bool scene_set_cube_state_from_string(Scene* scene, const char* state);

// Helper function to create a cube with custom colors based on position and stored colors
Mesh create_custom_colored_cube(unsigned int visibleFaces, Scene* scene, int x, int y, int z);

// Face rotation functions
void scene_rotate_face(Scene* scene, FaceIndex face, RotationDirection direction);

// Specific face rotations (wrapper functions)
void scene_rotate_top(Scene* scene, RotationDirection direction);
void scene_rotate_bottom(Scene* scene, RotationDirection direction);
void scene_rotate_left(Scene* scene, RotationDirection direction);
void scene_rotate_right(Scene* scene, RotationDirection direction);
void scene_rotate_front(Scene* scene, RotationDirection direction);
void scene_rotate_back(Scene* scene, RotationDirection direction);

// Start a face rotation animation
void scene_start_rotation(Scene* scene, FaceIndex face, RotationDirection direction);

// Check if rotation is in progress
bool scene_is_rotating(Scene* scene);

#endif /* SCENE_H */