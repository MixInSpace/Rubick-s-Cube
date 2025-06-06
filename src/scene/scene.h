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
    int rotationRepetitions; // Number of 90-degree state changes needed
    
    // Move sequence queue
    char** moveQueue;      // Array of move strings
    int moveQueueSize;     // Current number of moves in queue
    int moveQueueCapacity; // Maximum capacity of queue
    int currentMoveIndex;  // Index of currently executing move
    bool processingSequence; // Whether we're currently processing a sequence
};

bool scene_init(Scene* scene);
void scene_update(Scene* scene, float deltaTime);
void scene_render(Scene* scene, Window* window);
void scene_destroy(Scene* scene);

// Initialize the scene with a Rubik's cube grid
bool scene_init_rubiks(Scene* scene);

bool scene_set_cube_state_from_string(Scene* scene, const char* state);

// Get the current Rubik's cube state as a string
// Returns a dynamically allocated string that must be freed by the caller
// Format is the same as scene_set_cube_state_from_string
char* scene_get_cube_state_as_string(Scene* scene);

RGBColor* scene_get_cube_colors(Scene* scene);

// Helper function to create a cube with custom colors based on position and stored colors
Mesh create_custom_colored_cube(unsigned int visibleFaces, Scene* scene, int x, int y, int z);

void rotate_face_colors(RGBColor (*cubeColors)[9], FaceIndex face, RotationDirection direction);

// Face rotation functions
void scene_rotate_face(Scene* scene, FaceIndex face, RotationDirection direction);

// Start a face rotation animation
void scene_start_rotation(Scene* scene, FaceIndex face, RotationDirection direction, int repetitions);

// Check if rotation is in progress
bool scene_is_rotating(Scene* scene);

void apply_move_sequence(Scene* scene, char** moveSequence);

// Move sequence management functions
void scene_init_move_queue(Scene* scene);
void scene_destroy_move_queue(Scene* scene);
void scene_add_move_to_queue(Scene* scene, const char* move);
void scene_process_move_queue(Scene* scene);
bool scene_is_processing_sequence(Scene* scene);

#endif /* SCENE_H */