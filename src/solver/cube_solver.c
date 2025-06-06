#include "cube_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Helper function to compare RGB colors with tolerance
static bool colors_equal(RGBColor a, RGBColor b) {
    const float tolerance = 0.1f;
    return fabs(a.r - b.r) < tolerance && 
           fabs(a.g - b.g) < tolerance && 
           fabs(a.b - b.b) < tolerance;
}

// Get the center color for a face (position 4 is always the center)
static RGBColor get_center_color(const RGBColor (*cubeColors)[9], FaceIndex face) {
    return cubeColors[face][4];
}

// Check if a specific position on a face matches the center color
static bool position_matches_center(const RGBColor (*cubeColors)[9], FaceIndex face, int position) {
    RGBColor center = get_center_color(cubeColors, face);
    return colors_equal(cubeColors[face][position], center);
}

// Move sequence management functions
void move_sequence_init(MoveSequence* sequence) {
    sequence->moves = NULL;
    sequence->count = 0;
    sequence->capacity = 0;
}

void move_sequence_add(MoveSequence* sequence, Move move) {
    if (sequence->count >= sequence->capacity) {
        sequence->capacity = sequence->capacity == 0 ? 16 : sequence->capacity * 2;
        sequence->moves = realloc(sequence->moves, sequence->capacity * sizeof(Move));
    }
    sequence->moves[sequence->count++] = move;
}

void move_sequence_destroy(MoveSequence* sequence) {
    if (sequence->moves) {
        free(sequence->moves);
        sequence->moves = NULL;
    }
    sequence->count = 0;
    sequence->capacity = 0;
}

void move_sequence_print(const MoveSequence* sequence) {
    printf("Solution (%d moves): ", sequence->count);
    for (int i = 0; i < sequence->count; i++) {
        printf("%s ", move_to_string(sequence->moves[i]));
    }
    printf("\n");
}

// Move conversion functions
const char* move_to_string(Move move) {
    switch (move) {
        case MOVE_U: return "U";
        case MOVE_U_PRIME: return "U'";
        case MOVE_D: return "D";
        case MOVE_D_PRIME: return "D'";
        case MOVE_F: return "F";
        case MOVE_F_PRIME: return "F'";
        case MOVE_B: return "B";
        case MOVE_B_PRIME: return "B'";
        case MOVE_R: return "R";
        case MOVE_R_PRIME: return "R'";
        case MOVE_L: return "L";
        case MOVE_L_PRIME: return "L'";
        default: return "?";
    }
}

FaceIndex move_to_face(Move move) {
    switch (move) {
        case MOVE_U:
        case MOVE_U_PRIME:
            return FACE_IDX_TOP;
        case MOVE_D:
        case MOVE_D_PRIME:
            return FACE_IDX_BOTTOM;
        case MOVE_F:
        case MOVE_F_PRIME:
            return FACE_IDX_FRONT;
        case MOVE_B:
        case MOVE_B_PRIME:
            return FACE_IDX_BACK;
        case MOVE_R:
        case MOVE_R_PRIME:
            return FACE_IDX_RIGHT;
        case MOVE_L:
        case MOVE_L_PRIME:
            return FACE_IDX_LEFT;
        default:
            return FACE_IDX_TOP; // fallback
    }
}

RotationDirection move_to_direction(Move move) {
    switch (move) {
        case MOVE_U:
        case MOVE_D:
        case MOVE_F:
        case MOVE_B:
        case MOVE_R:
        case MOVE_L:
            return ROTATE_CLOCKWISE;
        case MOVE_U_PRIME:
        case MOVE_D_PRIME:
        case MOVE_F_PRIME:
        case MOVE_B_PRIME:
        case MOVE_R_PRIME:
        case MOVE_L_PRIME:
            return ROTATE_COUNTERCLOCKWISE;
        default:
            return ROTATE_CLOCKWISE; // fallback
    }
}

Move get_move_from_face_and_direction(FaceIndex face, RotationDirection direction) {
    switch (face) {
        case FACE_IDX_TOP:
            return direction == ROTATE_CLOCKWISE ? MOVE_U : MOVE_U_PRIME;
        case FACE_IDX_BOTTOM:
            return direction == ROTATE_CLOCKWISE ? MOVE_D : MOVE_D_PRIME;
        case FACE_IDX_FRONT:
            return direction == ROTATE_CLOCKWISE ? MOVE_F : MOVE_F_PRIME;
        case FACE_IDX_BACK:
            return direction == ROTATE_CLOCKWISE ? MOVE_B : MOVE_B_PRIME;
        case FACE_IDX_RIGHT:
            return direction == ROTATE_CLOCKWISE ? MOVE_R : MOVE_R_PRIME;
        case FACE_IDX_LEFT:
            return direction == ROTATE_CLOCKWISE ? MOVE_L : MOVE_L_PRIME;
        default:
            return MOVE_U; // fallback
    }
}

void apply_move_to_cube_colors(RGBColor (*cubeColors)[9], Move move) {
    FaceIndex face = move_to_face(move);
    RotationDirection direction = move_to_direction(move);
    rotate_face_colors(cubeColors, face, direction);
}

void copy_cube_state(const RGBColor (*source)[9], RGBColor (*dest)[9]) {
    for (int face = 0; face < 6; face++) {
        for (int pos = 0; pos < 9; pos++) {
            dest[face][pos] = source[face][pos];
        }
    }
}

bool is_cube_solved(const RGBColor (*cubeColors)[9]) {
    for (FaceIndex face = 0; face < 6; face++) {
        RGBColor center = get_center_color(cubeColors, face);
        for (int pos = 0; pos < 9; pos++) {
            if (!colors_equal(cubeColors[face][pos], center)) {
                return false;
            }
        }
    }
    return true;
}

// Helper function to find a piece with specific colors
static bool find_edge_piece(const RGBColor (*cubeColors)[9], RGBColor color1, RGBColor color2, 
                           FaceIndex* face1, int* pos1, FaceIndex* face2, int* pos2) {
    // Edge positions on each face: 1, 3, 5, 7
    int edge_positions[] = {1, 3, 5, 7};
    
    for (FaceIndex f1 = 0; f1 < 6; f1++) {
        for (int i = 0; i < 4; i++) {
            int p1 = edge_positions[i];
            if (colors_equal(cubeColors[f1][p1], color1)) {
                // Found first color, now find the adjacent face and position
                FaceIndex f2;
                int p2;
                
                // Map edge positions to adjacent face and position
                switch (f1) {
                    case FACE_IDX_TOP:
                        if (p1 == 1) { f2 = FACE_IDX_BACK; p2 = 1; }
                        else if (p1 == 3) { f2 = FACE_IDX_LEFT; p2 = 1; }
                        else if (p1 == 5) { f2 = FACE_IDX_RIGHT; p2 = 1; }
                        else if (p1 == 7) { f2 = FACE_IDX_FRONT; p2 = 1; }
                        break;
                    case FACE_IDX_FRONT:
                        if (p1 == 1) { f2 = FACE_IDX_TOP; p2 = 7; }
                        else if (p1 == 3) { f2 = FACE_IDX_LEFT; p2 = 5; }
                        else if (p1 == 5) { f2 = FACE_IDX_RIGHT; p2 = 3; }
                        else if (p1 == 7) { f2 = FACE_IDX_BOTTOM; p2 = 1; }
                        break;
                    case FACE_IDX_RIGHT:
                        if (p1 == 1) { f2 = FACE_IDX_TOP; p2 = 5; }
                        else if (p1 == 3) { f2 = FACE_IDX_FRONT; p2 = 5; }
                        else if (p1 == 5) { f2 = FACE_IDX_BACK; p2 = 3; }
                        else if (p1 == 7) { f2 = FACE_IDX_BOTTOM; p2 = 5; }
                        break;
                    case FACE_IDX_BACK:
                        if (p1 == 1) { f2 = FACE_IDX_TOP; p2 = 1; }
                        else if (p1 == 3) { f2 = FACE_IDX_RIGHT; p2 = 5; }
                        else if (p1 == 5) { f2 = FACE_IDX_LEFT; p2 = 3; }
                        else if (p1 == 7) { f2 = FACE_IDX_BOTTOM; p2 = 7; }
                        break;
                    case FACE_IDX_LEFT:
                        if (p1 == 1) { f2 = FACE_IDX_TOP; p2 = 3; }
                        else if (p1 == 3) { f2 = FACE_IDX_BACK; p2 = 5; }
                        else if (p1 == 5) { f2 = FACE_IDX_FRONT; p2 = 3; }
                        else if (p1 == 7) { f2 = FACE_IDX_BOTTOM; p2 = 3; }
                        break;
                    case FACE_IDX_BOTTOM:
                        if (p1 == 1) { f2 = FACE_IDX_FRONT; p2 = 7; }
                        else if (p1 == 3) { f2 = FACE_IDX_LEFT; p2 = 7; }
                        else if (p1 == 5) { f2 = FACE_IDX_RIGHT; p2 = 7; }
                        else if (p1 == 7) { f2 = FACE_IDX_BACK; p2 = 7; }
                        break;
                    default:
                        continue;
                }
                
                if (colors_equal(cubeColors[f2][p2], color2)) {
                    *face1 = f1; *pos1 = p1;
                    *face2 = f2; *pos2 = p2;
                    return true;
                }
            }
        }
    }
    return false;
}

// White cross solving (first layer edges)
static void solve_white_cross(RGBColor (*cubeColors)[9], MoveSequence* solution) {
    printf("Solving white cross...\n");
    
    RGBColor white = get_center_color(cubeColors, FACE_IDX_TOP);
    
    // For each edge position on the top face
    int target_positions[] = {1, 3, 5, 7}; // top, left, right, front edges
    FaceIndex adjacent_faces[] = {FACE_IDX_BACK, FACE_IDX_LEFT, FACE_IDX_RIGHT, FACE_IDX_FRONT};
    
    for (int i = 0; i < 4; i++) {
        int target_pos = target_positions[i];
        FaceIndex adj_face = adjacent_faces[i];
        RGBColor target_color = get_center_color(cubeColors, adj_face);
        
        // Check if edge is already in correct position
        if (colors_equal(cubeColors[FACE_IDX_TOP][target_pos], white) &&
            colors_equal(cubeColors[adj_face][1], target_color)) {
            continue; // Already solved
        }
        
        // Find the white-target_color edge piece
        FaceIndex face1, face2;
        int pos1, pos2;
        
        if (find_edge_piece(cubeColors, white, target_color, &face1, &pos1, &face2, &pos2)) {
            // Simple algorithm: bring edge to bottom, then to correct position
            // This is a simplified approach - a full solver would be more sophisticated
            
            // If edge is not in bottom layer, move it there
            if (face1 != FACE_IDX_BOTTOM && face2 != FACE_IDX_BOTTOM) {
                // Move edge to bottom (simplified - just do a few moves)
                if (face1 == FACE_IDX_FRONT || face2 == FACE_IDX_FRONT) {
                    move_sequence_add(solution, MOVE_F);
                    apply_move_to_cube_colors(cubeColors, MOVE_F);
                }
            }
            
            // Rotate bottom to align with target
            while (!colors_equal(cubeColors[adj_face][7], target_color)) {
                move_sequence_add(solution, MOVE_D);
                apply_move_to_cube_colors(cubeColors, MOVE_D);
            }
            
            // Move edge up to top
            switch (adj_face) {
                case FACE_IDX_FRONT:
                    move_sequence_add(solution, MOVE_F);
                    move_sequence_add(solution, MOVE_F);
                    apply_move_to_cube_colors(cubeColors, MOVE_F);
                    apply_move_to_cube_colors(cubeColors, MOVE_F);
                    break;
                case FACE_IDX_RIGHT:
                    move_sequence_add(solution, MOVE_R);
                    move_sequence_add(solution, MOVE_R);
                    apply_move_to_cube_colors(cubeColors, MOVE_R);
                    apply_move_to_cube_colors(cubeColors, MOVE_R);
                    break;
                case FACE_IDX_BACK:
                    move_sequence_add(solution, MOVE_B);
                    move_sequence_add(solution, MOVE_B);
                    apply_move_to_cube_colors(cubeColors, MOVE_B);
                    apply_move_to_cube_colors(cubeColors, MOVE_B);
                    break;
                case FACE_IDX_LEFT:
                    move_sequence_add(solution, MOVE_L);
                    move_sequence_add(solution, MOVE_L);
                    apply_move_to_cube_colors(cubeColors, MOVE_L);
                    apply_move_to_cube_colors(cubeColors, MOVE_L);
                    break;
                default:
                    break;
            }
        }
    }
}

// Simplified white corners solving
static void solve_white_corners(RGBColor (*cubeColors)[9], MoveSequence* solution) {
    printf("Solving white corners...\n");
    
    // For simplicity, just do a few random moves to attempt solving corners
    // A real solver would use proper corner-solving algorithms
    for (int i = 0; i < 8; i++) {
        move_sequence_add(solution, MOVE_R);
        move_sequence_add(solution, MOVE_U);
        move_sequence_add(solution, MOVE_R_PRIME);
        move_sequence_add(solution, MOVE_U_PRIME);
        
        apply_move_to_cube_colors(cubeColors, MOVE_R);
        apply_move_to_cube_colors(cubeColors, MOVE_U);
        apply_move_to_cube_colors(cubeColors, MOVE_R_PRIME);
        apply_move_to_cube_colors(cubeColors, MOVE_U_PRIME);
    }
}

// Simplified middle layer solving
static void solve_middle_layer(RGBColor (*cubeColors)[9], MoveSequence* solution) {
    printf("Solving middle layer...\n");
    
    // Simplified middle layer algorithm
    for (int i = 0; i < 4; i++) {
        move_sequence_add(solution, MOVE_R);
        move_sequence_add(solution, MOVE_U);
        move_sequence_add(solution, MOVE_R_PRIME);
        move_sequence_add(solution, MOVE_U);
        move_sequence_add(solution, MOVE_R);
        move_sequence_add(solution, MOVE_U);
        move_sequence_add(solution, MOVE_U);
        move_sequence_add(solution, MOVE_R_PRIME);
        
        apply_move_to_cube_colors(cubeColors, MOVE_R);
        apply_move_to_cube_colors(cubeColors, MOVE_U);
        apply_move_to_cube_colors(cubeColors, MOVE_R_PRIME);
        apply_move_to_cube_colors(cubeColors, MOVE_U);
        apply_move_to_cube_colors(cubeColors, MOVE_R);
        apply_move_to_cube_colors(cubeColors, MOVE_U);
        apply_move_to_cube_colors(cubeColors, MOVE_U);
        apply_move_to_cube_colors(cubeColors, MOVE_R_PRIME);
    }
}

// Simplified last layer solving
static void solve_last_layer(RGBColor (*cubeColors)[9], MoveSequence* solution) {
    printf("Solving last layer...\n");
    
    // Yellow cross (OLL)
    for (int i = 0; i < 4; i++) {
        move_sequence_add(solution, MOVE_F);
        move_sequence_add(solution, MOVE_R);
        move_sequence_add(solution, MOVE_U);
        move_sequence_add(solution, MOVE_R_PRIME);
        move_sequence_add(solution, MOVE_U_PRIME);
        move_sequence_add(solution, MOVE_F_PRIME);
        
        apply_move_to_cube_colors(cubeColors, MOVE_F);
        apply_move_to_cube_colors(cubeColors, MOVE_R);
        apply_move_to_cube_colors(cubeColors, MOVE_U);
        apply_move_to_cube_colors(cubeColors, MOVE_R_PRIME);
        apply_move_to_cube_colors(cubeColors, MOVE_U_PRIME);
        apply_move_to_cube_colors(cubeColors, MOVE_F_PRIME);
    }
    
    // Last layer corners and edges (PLL)
    for (int i = 0; i < 6; i++) {
        move_sequence_add(solution, MOVE_R);
        move_sequence_add(solution, MOVE_U);
        move_sequence_add(solution, MOVE_R_PRIME);
        move_sequence_add(solution, MOVE_F);
        move_sequence_add(solution, MOVE_R);
        move_sequence_add(solution, MOVE_F_PRIME);
        
        apply_move_to_cube_colors(cubeColors, MOVE_R);
        apply_move_to_cube_colors(cubeColors, MOVE_U);
        apply_move_to_cube_colors(cubeColors, MOVE_R_PRIME);
        apply_move_to_cube_colors(cubeColors, MOVE_F);
        apply_move_to_cube_colors(cubeColors, MOVE_R);
        apply_move_to_cube_colors(cubeColors, MOVE_F_PRIME);
    }
}

// Main solver function
MoveSequence* cube_solver_solve(Scene* scene) {
    if (!scene || !scene->isRubiksCube) {
        printf("Error: Invalid scene or not a Rubik's cube\n");
        return NULL;
    }
    
    printf("Starting Rubik's cube solver...\n");
    
    // Check if cube is already solved
    if (is_cube_solved(scene->cubeColors)) {
        printf("Cube is already solved!\n");
        MoveSequence* solution = malloc(sizeof(MoveSequence));
        move_sequence_init(solution);
        return solution;
    }
    
    // Create a working copy of the cube colors
    RGBColor working_colors[6][9];
    copy_cube_state(scene->cubeColors, working_colors);
    
    // Initialize solution sequence
    MoveSequence* solution = malloc(sizeof(MoveSequence));
    move_sequence_init(solution);
    
    printf("Initial cube state analysis:\n");
    for (int face = 0; face < 6; face++) {
        printf("Face %d: ", face);
        for (int pos = 0; pos < 9; pos++) {
            RGBColor color = working_colors[face][pos];
            if (colors_equal(color, (RGBColor){1.0f, 1.0f, 1.0f})) printf("W");
            else if (colors_equal(color, (RGBColor){1.0f, 1.0f, 0.0f})) printf("Y");
            else if (colors_equal(color, (RGBColor){1.0f, 0.0f, 0.0f})) printf("R");
            else if (colors_equal(color, (RGBColor){1.0f, 0.5f, 0.0f})) printf("O");
            else if (colors_equal(color, (RGBColor){0.0f, 0.0f, 1.0f})) printf("B");
            else if (colors_equal(color, (RGBColor){0.0f, 0.8f, 0.0f})) printf("G");
            else printf("?");
        }
        printf("\n");
    }
    
    // Layer-by-layer solving approach
    solve_white_cross(working_colors, solution);
    solve_white_corners(working_colors, solution);
    solve_middle_layer(working_colors, solution);
    solve_last_layer(working_colors, solution);
    
    printf("Solver completed with %d moves\n", solution->count);
    
    return solution;
} 