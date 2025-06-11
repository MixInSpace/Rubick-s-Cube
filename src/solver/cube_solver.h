#ifndef CUBE_SOLVER_H
#define CUBE_SOLVER_H

#include <stdbool.h>
#include "../scene/scene.h"

// Move notation for Rubik's cube
typedef enum {
    MOVE_U,   // Top face clockwise
    MOVE_U_PRIME, // Top face counterclockwise  
    MOVE_U2, // Top face 180 degree rotation
    MOVE_D,   // Bottom face clockwise
    MOVE_D_PRIME, // Bottom face counterclockwise
    MOVE_D2, // Bottom face 180 degree rotation
    MOVE_F,   // Front face clockwise
    MOVE_F_PRIME, // Front face counterclockwise
    MOVE_F2, // Front face 180 degree rotation
    MOVE_B,   // Back face clockwise
    MOVE_B_PRIME, // Back face counterclockwise
    MOVE_B2, // Back face 180 degree rotation
    MOVE_R,   // Right face clockwise
    MOVE_R_PRIME, // Right face counterclockwise
    MOVE_R2, // Right face 180 degree rotation
    MOVE_L,   // Left face clockwise
    MOVE_L_PRIME, // Left face counterclockwise
    MOVE_L2, // Left face 180 degree rotation
    MOVE_COUNT
} Move;

// struct {
//     FaceIndex face1, pos1, face2, pos2;
// } edge_map[] = {
//     {FACE_IDX_TOP, 1, FACE_IDX_FRONT, 1},
//     {FACE_IDX_TOP, 3, FACE_IDX_LEFT, 1},
//     {FACE_IDX_TOP, 5, FACE_IDX_RIGHT, 1},
//     {FACE_IDX_TOP, 7, FACE_IDX_BACK, 1},

//     {FACE_IDX_BOTTOM, 1, FACE_IDX_BACK, 7},
//     {FACE_IDX_BOTTOM, 3, FACE_IDX_LEFT, 7},
//     {FACE_IDX_BOTTOM, 5, FACE_IDX_RIGHT, 7},
//     {FACE_IDX_BOTTOM, 7, FACE_IDX_FRONT, 7},

//     {FACE_IDX_FRONT, 3, FACE_IDX_LEFT, 3},
//     {FACE_IDX_FRONT, 5, FACE_IDX_RIGHT, 5},

//     {FACE_IDX_BACK, 3, FACE_IDX_RIGHT, 3},
//     {FACE_IDX_BACK, 5, FACE_IDX_LEFT, 5},
// };

// Structure to hold a sequence of moves
typedef struct {
    Move* moves;
    int count;
    int capacity;
} MoveSequence;

// Structure to represent the cube state
typedef struct {
    RGBColor colors[6][9]; // [face][position]
} CubeState;

// Solver functions
char** cube_solver_solve(Scene* scene);
void move_sequence_init(MoveSequence* sequence);
void move_sequence_add(MoveSequence* sequence, Move move, RGBColor (*cubeColors)[9]);
void move_sequence_destroy(MoveSequence* sequence);
void move_sequence_print(const MoveSequence* sequence);

// Utility functions
Move get_move_from_face_and_direction(FaceIndex face, RotationDirection direction);
void apply_move_to_cube_colors(RGBColor (*cubeColors)[9], Move move);
void copy_cube_state(const RGBColor (*source)[9], RGBColor (*dest)[9]);
bool is_cube_solved(const RGBColor (*cubeColors)[9]);

// Move conversion functions
const char* move_to_string(Move move);
FaceIndex move_to_face(Move move);
RotationDirection move_to_direction(Move move);

#endif /* CUBE_SOLVER_H */ 