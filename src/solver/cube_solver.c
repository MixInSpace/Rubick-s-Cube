#include "cube_solver.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

static int index_array(int value, int* array){
    for (int k = 0; k < 4; ++k) {
        if (array[k] == value) {
            return k;
        }
    }
    return -1;
}
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

static RGBColor get_color(const RGBColor (*cubeColors)[9], FaceIndex face, int position) {
    return cubeColors[face][position];
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

void move_sequence_add(MoveSequence* sequence, Move move, RGBColor (*cubeColors)[9]) {

    apply_move_to_cube_colors(cubeColors, move);

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
        case MOVE_U2: return "U2";
        case MOVE_D: return "D";
        case MOVE_D_PRIME: return "D'";
        case MOVE_D2: return "D2";
        case MOVE_F: return "F";
        case MOVE_F_PRIME: return "F'";
        case MOVE_F2: return "F2";
        case MOVE_B: return "B";
        case MOVE_B_PRIME: return "B'";
        case MOVE_B2: return "B2";
        case MOVE_R: return "R";
        case MOVE_R_PRIME: return "R'";
        case MOVE_R2: return "R2";
        case MOVE_L: return "L";
        case MOVE_L_PRIME: return "L'";
        case MOVE_L2: return "L2";
        default: return "?";
    }
}

FaceIndex move_to_face(Move move) {
    switch (move) {
        case MOVE_U:
        case MOVE_U_PRIME:
        case MOVE_U2:
            return FACE_IDX_TOP;
        case MOVE_D:
        case MOVE_D_PRIME:
        case MOVE_D2:
            return FACE_IDX_BOTTOM;
        case MOVE_F:
        case MOVE_F_PRIME:
        case MOVE_F2:
            return FACE_IDX_FRONT;
        case MOVE_B:
        case MOVE_B_PRIME:
        case MOVE_B2:
            return FACE_IDX_BACK;
        case MOVE_R:
        case MOVE_R_PRIME:
        case MOVE_R2:
            return FACE_IDX_RIGHT;
        case MOVE_L:
        case MOVE_L_PRIME:
        case MOVE_L2:
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
        case MOVE_U2:
        case MOVE_D2:
        case MOVE_F2:
        case MOVE_B2:
        case MOVE_R2:
        case MOVE_L2:
            return ROTATE_180;
        default:
            return ROTATE_CLOCKWISE; // fallback
    }
}

Move get_move_from_face_and_direction(FaceIndex face, RotationDirection direction) {
    switch (face) {
        case FACE_IDX_TOP:
            return direction == ROTATE_CLOCKWISE ? MOVE_U : direction == ROTATE_COUNTERCLOCKWISE ? MOVE_U_PRIME : MOVE_U2;
        case FACE_IDX_BOTTOM:
            return direction == ROTATE_CLOCKWISE ? MOVE_D : direction == ROTATE_COUNTERCLOCKWISE ? MOVE_D_PRIME : MOVE_D2;
        case FACE_IDX_FRONT:
            return direction == ROTATE_CLOCKWISE ? MOVE_F : direction == ROTATE_COUNTERCLOCKWISE ? MOVE_F_PRIME : MOVE_F2;
        case FACE_IDX_BACK:
            return direction == ROTATE_CLOCKWISE ? MOVE_B : direction == ROTATE_COUNTERCLOCKWISE ? MOVE_B_PRIME : MOVE_B2;
        case FACE_IDX_RIGHT:
            return direction == ROTATE_CLOCKWISE ? MOVE_R : direction == ROTATE_COUNTERCLOCKWISE ? MOVE_R_PRIME : MOVE_R2;
        case FACE_IDX_LEFT:
            return direction == ROTATE_CLOCKWISE ? MOVE_L : direction == ROTATE_COUNTERCLOCKWISE ? MOVE_L_PRIME : MOVE_L2;
        default:
            return MOVE_U; // fallback
    }
}

void apply_move_to_cube_colors(RGBColor (*cubeColors)[9], Move move) {
    printf("apply_move_to_cube_colors: %s\n", move_to_string(move));
    printf("cubeColors: %s\n", scene_get_cube_state_as_string(cubeColors));
    FaceIndex face = move_to_face(move);


    printf("face: %d\n", face);
    RotationDirection direction = move_to_direction(move);
    if ((face == FACE_IDX_BOTTOM || face == FACE_IDX_BACK || face == FACE_IDX_LEFT) && direction != ROTATE_180){
        direction = -direction;
    }
    printf("direction: %d\n", direction);
    if (direction == ROTATE_180){
        direction = ROTATE_CLOCKWISE;
        rotate_face_colors(cubeColors, face, direction);
        printf("apply_move_to_cube_colors: %s\n", scene_get_cube_state_as_string(cubeColors));

        rotate_face_colors(cubeColors, face, direction);
        printf("apply_move_to_cube_colors: %s\n", scene_get_cube_state_as_string(cubeColors));
    }
    else {
        rotate_face_colors(cubeColors, face, direction);
        printf("apply_move_to_cube_colors: %s\n", scene_get_cube_state_as_string(cubeColors));

    }
    

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
    // Your static edge map
    static const struct {
        FaceIndex face1, pos1, face2, pos2;
    } edge_map[] = {
        {FACE_IDX_TOP, 1, FACE_IDX_FRONT, 1},
        {FACE_IDX_TOP, 3, FACE_IDX_LEFT, 1},
        {FACE_IDX_TOP, 5, FACE_IDX_RIGHT, 1},
        {FACE_IDX_TOP, 7, FACE_IDX_BACK, 1},

        {FACE_IDX_BOTTOM, 1, FACE_IDX_BACK, 7},
        {FACE_IDX_BOTTOM, 3, FACE_IDX_LEFT, 7},
        {FACE_IDX_BOTTOM, 5, FACE_IDX_RIGHT, 7},
        {FACE_IDX_BOTTOM, 7, FACE_IDX_FRONT, 7},

        { FACE_IDX_FRONT, 3, FACE_IDX_LEFT, 3},
        { FACE_IDX_FRONT, 5, FACE_IDX_RIGHT, 5},
        { FACE_IDX_BACK, 5, FACE_IDX_LEFT, 5},
        { FACE_IDX_BACK, 3, FACE_IDX_RIGHT, 3},
    };

    const int EDGE_COUNT = sizeof(edge_map) / sizeof(edge_map[0]);

    for (int i = 0; i < EDGE_COUNT; ++i) {
        FaceIndex f1 = edge_map[i].face1;
        int p1 = edge_map[i].pos1;
        FaceIndex f2 = edge_map[i].face2;
        int p2 = edge_map[i].pos2;

        RGBColor c1 = cubeColors[f1][p1];
        RGBColor c2 = cubeColors[f2][p2];

        // Match both (color1, color2) and (color2, color1)
        if ((colors_equal(c1, color1) && colors_equal(c2, color2))) {
            *face1 = f1; *pos1 = p1;
            *face2 = f2; *pos2 = p2;
            return true;
        } else if ((colors_equal(c1, color2) && colors_equal(c2, color1))) {
            *face1 = f2; *pos1 = p2;
            *face2 = f1; *pos2 = p1;
            return true;
        }
    }

    return false;
}



static void solve_white_cross(RGBColor (*cubeColors)[9], MoveSequence* solution) {
    printf("Solving white cross...\n");
    
    RGBColor white = get_center_color(cubeColors, FACE_IDX_TOP);
    
    /*
           [7]
        [3]   [5]
           [1]
        нижняя грань (вид спереди, передняя грань со стороны 7)
    */
    int bottom_target_positions[] = {7, 5, 1, 3};

    /*
           [1]
        [5]   [3]
           [7]
        верхняя грань (вид спереди, передняя грань со стороны 1)
    */
    int top_target_positions[] = {1, 5, 7, 3};

    /*
        T
        F R B L
        D    
    */
    FaceIndex adjacent_faces[] = {FACE_IDX_FRONT, FACE_IDX_RIGHT, FACE_IDX_BACK, FACE_IDX_LEFT};
    Move down_moves[] = {MOVE_D_PRIME, MOVE_D2, MOVE_D};
    
    for (int i = 0; i < 4; i++) {
        printf("Solving white cross: %d\n", i);
        int target_pos = top_target_positions[i];
        FaceIndex adj_face = adjacent_faces[i];
        RGBColor target_color = get_center_color(cubeColors, adj_face);

        // Проверяем, если это ребро уже стоит на месте        
        if (colors_equal(cubeColors[FACE_IDX_TOP][target_pos], white) &&
            position_matches_center(cubeColors, adj_face, 1) &&
            colors_equal(cubeColors[adj_face][1], target_color)) {
            continue; 
        }

        // Находим ребро
        FaceIndex white_face, color_face;
        int white_pos, color_pos;
        Move return_move = -1;

        if (!find_edge_piece(cubeColors, white, target_color, &white_face, &white_pos, &color_face, &color_pos)) {
            printf("No edge piece found\n");
            return;
        }

        // Если белый цвет сверху, но не на своем месте, то опускаем его вниз и обновляем положение 
        if (white_face == FACE_IDX_TOP) {
            move_sequence_add(solution,
                get_move_from_face_and_direction(color_face, ROTATE_180),
                cubeColors);
            find_edge_piece(cubeColors, white, target_color, &white_face, &white_pos, &color_face, &color_pos);
        } 

        // Если белый цвет на одной из боковых граней, то
        else if (white_face != FACE_IDX_BOTTOM) {

            // Если белый цвет сверху на боковой грани, то поворачиваем эту грань чтобы следующим ходом опустить его вниз
            if (white_pos == 1) {
                move_sequence_add(solution, get_move_from_face_and_direction(white_face, ROTATE_CLOCKWISE), cubeColors);
                find_edge_piece(cubeColors, white, target_color, &white_face, &white_pos, &color_face, &color_pos);
            } 
            // Если снизу, то 
            else if (white_pos == 7) {
                // Чтобы не помешать другим граням, поворачиваем нижнюю грань на свое место
                int white_pos_index = index_array(white_face, (int*)adjacent_faces);
                int move_index = ((white_pos_index - i) + 4) % 4;
                if (move_index != 0)
                    move_sequence_add(solution, down_moves[move_index - 1], cubeColors);
                find_edge_piece(cubeColors, white, target_color, &white_face, &white_pos, &color_face, &color_pos);
                // И делаем поворот чтобы следующим ходом опустить его вниз
                move_sequence_add(solution, get_move_from_face_and_direction(white_face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                find_edge_piece(cubeColors, white, target_color, &white_face, &white_pos, &color_face, &color_pos);
            }

            // Если цвет на боковой грани, то
            if ((color_pos == 5 || color_pos == 3) && color_face != adj_face) {
                // В зависимости от положения цвета, поворачиваем грань по часовой или против часовой стрелки
                RotationDirection direction = color_pos == 5 ? ROTATE_CLOCKWISE : ROTATE_COUNTERCLOCKWISE;
                // Если цвет на левой или правой грани, то поворачиваем в противоположную сторону
                if (color_face == FACE_IDX_LEFT || color_face == FACE_IDX_RIGHT) direction *= -1;
                // Если сверху деталь на своем месте, то запоминаем, что надо будет вернуть его на место 
                if (colors_equal(get_color(cubeColors, FACE_IDX_TOP, top_target_positions[index_array(color_face, (int*)adjacent_faces)]), white)) {
                    return_move = get_move_from_face_and_direction(color_face, -direction);
                }
                move_sequence_add(solution, get_move_from_face_and_direction(color_face, direction), cubeColors);
                find_edge_piece(cubeColors, white, target_color, &white_face, &white_pos, &color_face, &color_pos);
            }

            // Если цвет на своей грани, то поворачиваем ее на свое место
            if (color_face == adj_face) {
                RotationDirection direction = color_pos == 3 ? ROTATE_CLOCKWISE : ROTATE_COUNTERCLOCKWISE;
                if (color_face == FACE_IDX_LEFT || color_face == FACE_IDX_RIGHT) {
                    direction = -direction;
                }
                move_sequence_add(solution, get_move_from_face_and_direction(color_face, direction), cubeColors);
                continue;
            }
        }

        // Если белый цвет на нижней грани, то поворачиваем нижнюю грань на свое место и поднимаем деталь вверх
        int white_pos_index = index_array(white_pos, bottom_target_positions);
        int move_index = ((white_pos_index - i) + 4) % 4;
        if (move_index != 0)
            move_sequence_add(solution, down_moves[move_index - 1], cubeColors);

        // Если надо вернуть деталь на место, то делаем это
        if (return_move != -1) {
            move_sequence_add(solution, return_move, cubeColors);
        }

        // Поворачиваем грань на 180 градусов, чтобы поставить деталь на свое место
        move_sequence_add(solution, get_move_from_face_and_direction(adj_face, ROTATE_180), cubeColors);

    }
}

// Simplified white corners solving
static void solve_white_corners(RGBColor (*cubeColors)[9], MoveSequence* solution) {

}

// Simplified middle layer solving
static void solve_middle_layer(RGBColor (*cubeColors)[9], MoveSequence* solution) {

}

// Simplified last layer solving
static void solve_last_layer(RGBColor (*cubeColors)[9], MoveSequence* solution) {

}

// Main solver function
char** cube_solver_solve(Scene* scene) {
    if (!scene || !scene->isRubiksCube) {
        printf("Error: Invalid scene or not a Rubik's cube\n");
        return NULL;
    }
    
    printf("Starting Rubik's cube solver...\n");
    
    // Check if cube is already solved
    if (is_cube_solved(scene->cubeColors)) {
        printf("Cube is already solved!\n");
        // Return empty move sequence (just NULL terminator)
        char** moveSequence = malloc(sizeof(char*));
        moveSequence[0] = NULL;
        return moveSequence;
    }
    
    // Create a working copy of the cube colors
    RGBColor working_colors[6][9];
    copy_cube_state(scene->cubeColors, working_colors);
    
    // Initialize solution sequence
    MoveSequence solution;
    move_sequence_init(&solution);
    
    // Layer-by-layer solving approach
    solve_white_cross(working_colors, &solution);
    // solve_white_corners(working_colors, &solution);
    // solve_middle_layer(working_colors, &solution);
    // solve_last_layer(working_colors, &solution);
    
    printf("Solver completed with %d moves\n", solution.count);
    
    // Convert MoveSequence to char** format
    char** moveSequence = malloc((solution.count + 1) * sizeof(char*));
    if (!moveSequence) {
        move_sequence_destroy(&solution);
        return NULL;
    }
    
    // Convert each move to string and allocate memory for it
    for (int i = 0; i < solution.count; i++) {
        const char* moveStr = move_to_string(solution.moves[i]);
        moveSequence[i] = malloc((strlen(moveStr) + 1) * sizeof(char));
        if (moveSequence[i]) {
            strcpy(moveSequence[i], moveStr);
        } else {
            // Handle allocation failure by cleaning up
            for (int j = 0; j < i; j++) {
                free(moveSequence[j]);
            }
            free(moveSequence);
            move_sequence_destroy(&solution);
            return NULL;
        }
    }
    
    // Null terminate the array
    moveSequence[solution.count] = NULL;
    
    // Clean up the temporary MoveSequence
    move_sequence_destroy(&solution);
    
    return moveSequence;
} 