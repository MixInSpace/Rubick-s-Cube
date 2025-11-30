#ifndef CUBE_SOLVER_H
#define CUBE_SOLVER_H

#include <stdbool.h>
#include "../scene/scene.h"

typedef enum {
    MOVE_U,
    MOVE_U_PRIME,
    MOVE_U2,
    MOVE_D,
    MOVE_D_PRIME,
    MOVE_D2,
    MOVE_F, 
    MOVE_F_PRIME, 
    MOVE_F2,
    MOVE_B, 
    MOVE_B_PRIME,
    MOVE_B2, 
    MOVE_R, 
    MOVE_R_PRIME, 
    MOVE_R2, 
    MOVE_L, 
    MOVE_L_PRIME, 
    MOVE_L2, 
    MOVE_COUNT
} Move;


typedef struct {
    Move* moves;
    int count;
    int capacity;
} MoveSequence;

char** cube_solver_solve(Scene* scene, bool* isSolved);
void cube_solver_set_quiet(bool quiet);
void move_sequence_init(MoveSequence* sequence);
void move_sequence_add(MoveSequence* sequence, Move move, RGBColor (*cubeColors)[9]);
void move_sequence_destroy(MoveSequence* sequence);
void move_sequence_print(const MoveSequence* sequence);

Move get_move_from_face_and_direction(FaceIndex face, RotationDirection direction);
void apply_move_to_cube_colors(RGBColor (*cubeColors)[9], Move move);
void copy_cube_state(const RGBColor (*source)[9], RGBColor (*dest)[9]);
bool is_cube_solved(const RGBColor (*cubeColors)[9]);

const char* move_to_string(Move move);
FaceIndex move_to_face(Move move);
RotationDirection move_to_direction(Move move);

#endif /* CUBE_SOLVER_H */ 
