#include "cube_solver.h"
#include "oll.h"
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

static bool bottom_equal(int* array1, int* array2) {
    for (int i = 0; i < 9; i++) {
        if (array1[i] != array2[i]) {
            return false;
        }
    }
    return true;
}

static bool sides_equal(int (*array1)[3], int (*array2)[3]) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            if (array1[i][j] != array2[i][j]) {
                return false;
            }
        }
    }
    return true;
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
    FaceIndex face = move_to_face(move);

    RotationDirection direction = move_to_direction(move);
    if ((face == FACE_IDX_BOTTOM || face == FACE_IDX_BACK || face == FACE_IDX_LEFT) && direction != ROTATE_180){
        direction = -direction;
    }
    if (direction == ROTATE_180){
        direction = ROTATE_CLOCKWISE;
        rotate_face_colors(cubeColors, face, direction);
        rotate_face_colors(cubeColors, face, direction);
    }
    else {
        rotate_face_colors(cubeColors, face, direction);
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

static bool find_corner_piece(const RGBColor (*cubeColors)[9], RGBColor color1, RGBColor color2, RGBColor color3,
                              FaceIndex* face1, int* pos1, FaceIndex* face2, int* pos2, FaceIndex* face3, int* pos3) {
    static const struct {
        FaceIndex face1, pos1, face2, pos2, face3, pos3;
    } corner_map[] = {
        {FACE_IDX_TOP, 0, FACE_IDX_FRONT, 0, FACE_IDX_LEFT, 0},
        {FACE_IDX_TOP, 2, FACE_IDX_RIGHT, 2, FACE_IDX_FRONT, 2},
        {FACE_IDX_TOP, 6, FACE_IDX_LEFT, 2, FACE_IDX_BACK, 2},
        {FACE_IDX_TOP, 8, FACE_IDX_BACK, 0, FACE_IDX_RIGHT, 0},

        {FACE_IDX_BOTTOM, 0, FACE_IDX_BACK, 8, FACE_IDX_LEFT, 8},
        {FACE_IDX_BOTTOM, 2, FACE_IDX_RIGHT, 6, FACE_IDX_BACK, 6},
        {FACE_IDX_BOTTOM, 6, FACE_IDX_LEFT, 6, FACE_IDX_FRONT, 6},
        {FACE_IDX_BOTTOM, 8, FACE_IDX_FRONT, 8, FACE_IDX_RIGHT, 8},
    };

    const int CORNER_COUNT = sizeof(corner_map) / sizeof(corner_map[0]);

    for (int i = 0; i < CORNER_COUNT; ++i) {
        FaceIndex f1 = corner_map[i].face1;
        int p1 = corner_map[i].pos1;
        FaceIndex f2 = corner_map[i].face2;
        int p2 = corner_map[i].pos2;
        FaceIndex f3 = corner_map[i].face3;
        int p3 = corner_map[i].pos3;    
        
        RGBColor c1 = cubeColors[f1][p1];
        RGBColor c2 = cubeColors[f2][p2];
        RGBColor c3 = cubeColors[f3][p3];

        // printf("f1: %d, f2: %d, f3: %d\n", f1, f2, f3);
        // printf("p1: %d, p2: %d, p3: %d\n", p1, p2, p3);


        // printf("c1: %f, %f, %f\n", c1.r, c1.g, c1.b);
        // printf("c2: %f, %f, %f\n", c2.r, c2.g, c2.b);
        // printf("c3: %f, %f, %f\n", c3.r, c3.g, c3.b);
        // printf("color1: %f, %f, %f\n", color1.r, color1.g, color1.b);
        // printf("color2: %f, %f, %f\n", color2.r, color2.g, color2.b);
        // printf("color3: %f, %f, %f\n", color3.r, color3.g, color3.b);
        if (colors_equal(c1, color1) && colors_equal(c2, color2) && colors_equal(c3, color3)) {
            *face1 = f1; *pos1 = p1;    
            *face2 = f2; *pos2 = p2;
            *face3 = f3; *pos3 = p3;
            return true;
        } else if (colors_equal(c1, color1) && colors_equal(c2, color3) && colors_equal(c3, color2)) {
            *face1 = f1; *pos1 = p1;
            *face2 = f3; *pos2 = p3;
            *face3 = f2; *pos3 = p2;
            return true;
        } else if (colors_equal(c1, color2) && colors_equal(c2, color1) && colors_equal(c3, color3)) {
            *face1 = f2; *pos1 = p2;
            *face2 = f1; *pos2 = p1;
            *face3 = f3; *pos3 = p3;
            return true;
        } else if (colors_equal(c1, color2) && colors_equal(c2, color3) && colors_equal(c3, color1)) {
            *face1 = f2; *pos1 = p2;
            *face2 = f3; *pos2 = p3;
            *face3 = f1; *pos3 = p1;
            return true;
        } else if (colors_equal(c1, color3) && colors_equal(c2, color1) && colors_equal(c3, color2)) {
            *face1 = f3; *pos1 = p3;
            *face2 = f1; *pos2 = p1;
            *face3 = f2; *pos3 = p2;
            return true;
        } else if (colors_equal(c1, color3) && colors_equal(c2, color2) && colors_equal(c3, color1)) {
            *face1 = f3; *pos1 = p3;
            *face2 = f2; *pos2 = p2;
            *face3 = f1; *pos3 = p1;
            return true;
        }
    }

    return false;
}

static void get_yellow_positions(const RGBColor (*cubeColors)[9], int* positions_bottom, int (*positions_sides)[3]) {
    // find all yellow cells on the cube and export their positions in form of indexes on the cube
    RGBColor yellow = get_center_color(cubeColors, FACE_IDX_BOTTOM);
    RGBColor green = get_center_color(cubeColors, FACE_IDX_FRONT);
    RGBColor blue = get_center_color(cubeColors, FACE_IDX_RIGHT);
    RGBColor red = get_center_color(cubeColors, FACE_IDX_BACK);
    RGBColor orange = get_center_color(cubeColors, FACE_IDX_LEFT);
    for (int i = 0; i < 9; i++) {
        positions_bottom[i] = 0;
    }
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            positions_sides[i][j] = 0;
        }
    }

    for (int i = 0; i < 9; i++) {
        if (colors_equal(cubeColors[FACE_IDX_BOTTOM][i], yellow)) {
            positions_bottom[i] = 1;
        } 
    }
    int temp = 0;
    temp = positions_bottom[1];
    positions_bottom[1] = positions_bottom[3];
    positions_bottom[3] = temp;

    temp = positions_bottom[2];
    positions_bottom[2] = positions_bottom[6];
    positions_bottom[6] = temp;

    temp = positions_bottom[5];
    positions_bottom[5] = positions_bottom[7];
    positions_bottom[7] = temp;

    for (int i = 6; i < 9; i++) {
        if (colors_equal(cubeColors[FACE_IDX_FRONT][i], yellow)) {
            positions_sides[1][i - 6] = 1;
        }
        if (colors_equal(cubeColors[FACE_IDX_RIGHT][i], yellow)) {
            positions_sides[2][8 - i] = 1;
        }
        if (colors_equal(cubeColors[FACE_IDX_BACK][i], yellow)) {
            positions_sides[3][i - 6] = 1;
        }
        if (colors_equal(cubeColors[FACE_IDX_LEFT][i], yellow)) {
            positions_sides[0][8 - i] = 1;
        }
    }
}

static void get_side_positions(const RGBColor (*cubeColors)[9], int (*positions_sides)[3]) {
    RGBColor green = get_center_color(cubeColors, FACE_IDX_LEFT);
    RGBColor blue = get_center_color(cubeColors, FACE_IDX_RIGHT);
    RGBColor red = get_center_color(cubeColors, FACE_IDX_FRONT);
    RGBColor orange = get_center_color(cubeColors, FACE_IDX_BACK);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            positions_sides[i][j] = 0;
        }
    }

    for (int face = 1; face < 5; face++) {
        for (int i = 6; i < 9; i++) {
            int id = i - 6;
            if (face == FACE_IDX_LEFT || face == FACE_IDX_RIGHT) {
                id = 8 - i;
            }

            if (colors_equal(cubeColors[face][i], red)) {
                positions_sides[face-1][id] = 1;
            }
            if (colors_equal(cubeColors[face][i], blue)) {
                positions_sides[face-1][id] = 2;
            }
            if (colors_equal(cubeColors[face][i], orange)) {
                positions_sides[face-1][id] = 3;
            }
            if (colors_equal(cubeColors[face][i], green)) {
                positions_sides[face-1][id] = 4;
            }
        }
    }
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

static void solve_F2L(RGBColor (*cubeColors)[9], MoveSequence* solution) {
    printf("Solving F2L...\n");

    int bottom_target_positions[] = {1, 3, 7, 5};
    Move down_moves[] = {MOVE_D_PRIME, MOVE_D2, MOVE_D};
    FaceIndex adjacent_faces[] = {FACE_IDX_FRONT, FACE_IDX_RIGHT, FACE_IDX_BACK, FACE_IDX_LEFT};

    RGBColor white = get_center_color(cubeColors, FACE_IDX_TOP);

    // Проверяем, если все угловые детали на своем месте
    for (int i = 0; i < 4; i++) {
        FaceIndex face = FACE_IDX_FRONT + i;
        int pos = 1;
        FaceIndex face2 = (face + 1) % 5 + 1;
        int pos2 = 1;

        RGBColor color1 = get_center_color(cubeColors, face);
        RGBColor color2 = get_center_color(cubeColors, face2);

        FaceIndex color_face, color_face2;
        int color_pos, color_pos2;

        find_edge_piece(cubeColors, color1, color2, &color_face, &color_pos, &color_face2, &color_pos2);

        if (!(color_face == face && color_face2 == face2)) {
            break;
        }

        FaceIndex white_face;
        int white_pos;

        find_corner_piece(cubeColors, color1, color2, white, &color_face, &color_pos, &color_face2, &color_pos2, &white_face, &white_pos);

        if (!(color_face == face && color_face2 == face2 && white_face == FACE_IDX_TOP)) {
            break;
        }        
    }
    
    for (int i = 0; i < 4; i++) {
        FaceIndex face = FACE_IDX_FRONT + i;
        int pos = 1;
        FaceIndex face2 = (face % 4) + 1;
        int pos2 = 1;
        FaceIndex face3 = (face2 % 4) + 1;
        FaceIndex face4 = (face3 % 4) + 1;

        RGBColor color1 = get_center_color(cubeColors, face);
        RGBColor color2 = get_center_color(cubeColors, face2);

        FaceIndex edge_color_face, edge_color_face2;
        int edge_color_pos, edge_color_pos2;

        find_edge_piece(cubeColors, color1, color2, &edge_color_face, &edge_color_pos, &edge_color_face2, &edge_color_pos2);

        FaceIndex corner_color_face, corner_color_face2, corner_white_face;
        int corner_color_pos, corner_color_pos2, corner_white_pos;

        find_corner_piece(cubeColors, color1, color2, white, &corner_color_face, &corner_color_pos, &corner_color_face2, &corner_color_pos2, &corner_white_face, &corner_white_pos);
        
        
        // stuck cases
        if((!(corner_color_face == face && corner_color_face2 == face2 || corner_color_face2 == face && corner_white_face == face2 || corner_color_face == face2 && corner_white_face == face) && 
            (corner_white_face == FACE_IDX_TOP || corner_color_face == FACE_IDX_TOP || corner_color_face2 == FACE_IDX_TOP)))    
        {   
            FaceIndex face_before = (corner_white_face == 1) ? 4 : corner_white_face - 1;
            FaceIndex face_before1 = (corner_color_face == 1) ? 4 : corner_color_face - 1;
            FaceIndex face_before2 = (corner_color_face2 == 1) ? 4 : corner_color_face2 - 1;

            RotationDirection rotation = ROTATE_CLOCKWISE;

            FaceIndex corner_white_face_to_rotate = corner_white_face;

            if (corner_white_face == FACE_IDX_TOP) {
                corner_white_face_to_rotate = corner_color_face;
                if (edge_color_face == face_before1 || edge_color_face2 == face_before1) {
                    rotation *= -1;
                }
            } else if (corner_color_face == FACE_IDX_TOP) {
                corner_white_face_to_rotate = corner_color_face2;
                if (edge_color_face == face_before2 || edge_color_face2 == face_before2) {
                    rotation *= -1;
                }
            } else if (corner_color_face2 == FACE_IDX_TOP) {
                corner_white_face_to_rotate = corner_white_face;
                if (edge_color_face == face_before || edge_color_face2 == face_before) {
                    rotation *= -1;
                }
            }
            move_sequence_add(solution, get_move_from_face_and_direction(corner_white_face_to_rotate, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, rotation), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(corner_white_face_to_rotate, ROTATE_COUNTERCLOCKWISE), cubeColors);

            find_corner_piece(cubeColors, color1, color2, white, &corner_color_face, &corner_color_pos, &corner_color_face2, &corner_color_pos2, &corner_white_face, &corner_white_pos);
            find_edge_piece(cubeColors, color1, color2, &edge_color_face, &edge_color_pos, &edge_color_face2, &edge_color_pos2);
        } 
        if ( 
            !((edge_color_face == face && edge_color_face2 == face2) || (edge_color_face == face2 && edge_color_face2 == face)) && 
            !(edge_color_face == FACE_IDX_BOTTOM || edge_color_face2 == FACE_IDX_BOTTOM))
        {
            RotationDirection rotation = ROTATE_CLOCKWISE;
            FaceIndex edge_color_face_to_rotate = edge_color_face;
            if (edge_color_face == FACE_IDX_FRONT || edge_color_face == FACE_IDX_BACK) {
                if (edge_color_pos == 5) rotation = ROTATE_CLOCKWISE;
                else rotation = ROTATE_COUNTERCLOCKWISE;
            } else if (edge_color_face == FACE_IDX_LEFT || edge_color_face == FACE_IDX_RIGHT) {
                if (edge_color_pos == 5) rotation = ROTATE_COUNTERCLOCKWISE;
                else rotation = ROTATE_CLOCKWISE;
            }
            for (int i = 0; i < 2; i++) {
                move_sequence_add(solution, get_move_from_face_and_direction(edge_color_face_to_rotate, rotation), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(edge_color_face_to_rotate, -rotation), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
            find_corner_piece(cubeColors, color1, color2, white, &corner_color_face, &corner_color_pos, &corner_color_face2, &corner_color_pos2, &corner_white_face, &corner_white_pos);
            find_edge_piece(cubeColors, color1, color2, &edge_color_face, &edge_color_pos, &edge_color_face2, &edge_color_pos2);
        } 

        // printf("face1: %d, face2: %d\n", face, face2);


        FaceIndex face_after_corner_white_face = (corner_white_face == 4) ? 1 : corner_white_face + 1;

        FaceIndex face_before_corner_white_face = (corner_white_face == 1) ? 4 : corner_white_face - 1;

        FaceIndex face_on_the_other_side_of_white = (corner_white_face + 1) % 4 + 1;

        FaceIndex face_on_the_other_side_of_color = (corner_color_face + 1) % 4 + 1;
        FaceIndex face_on_the_other_side_of_color2 = (corner_color_face2 + 1) % 4 + 1;
        
        // Basic Inserts

        // F2L 1,2
        if (edge_color_face == corner_color_face && edge_color_face2 == corner_color_face2 && corner_white_face != FACE_IDX_BOTTOM) {
            int white_pos_index = index_array(corner_white_face, (int*)adjacent_faces);
            if (edge_color_face == FACE_IDX_BOTTOM) {
                printf("F2L 1\n");
                RotationDirection direction = ROTATE_CLOCKWISE;
                if (corner_white_face == face) {
                    move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                } else if (corner_white_face == face2) {
                    move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
                } else if (corner_white_face == face3) {
                    direction = ROTATE_180;
                }

                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, direction), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            }
            else 
            // F2L 2 work
            if (edge_color_face2 == FACE_IDX_BOTTOM) {
                printf("F2L 2\n");
                RotationDirection direction = ROTATE_COUNTERCLOCKWISE;
                if (corner_white_face == face) {
                    move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
                } else if (corner_white_face == face2) {
                    move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                } else if (corner_white_face == face4) {
                    direction = ROTATE_180;
                }
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, direction), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
        }
        
        // F2L 3 work
        else if (edge_color_face2 == face_after_corner_white_face && edge_color_face == FACE_IDX_BOTTOM && corner_color_face2 == FACE_IDX_BOTTOM) {
            printf("F2L 3\n");
            if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
        }
        // F2L 4 work
        else if (edge_color_face == face_before_corner_white_face && edge_color_face2 == FACE_IDX_BOTTOM && corner_color_face == FACE_IDX_BOTTOM) {
            printf("F2L 4\n");
            if (corner_white_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // Reposition Edge
        
        // F2L 5 work
        else if (edge_color_face == face_on_the_other_side_of_white && edge_color_face2 == FACE_IDX_BOTTOM && corner_color_face2 == FACE_IDX_BOTTOM) {
            printf("F2L 5\n");
            if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }
    
        // F2L 6 work?
        else if (edge_color_face2 == face_on_the_other_side_of_white && edge_color_face == FACE_IDX_BOTTOM && corner_color_face == FACE_IDX_BOTTOM) {
            printf("F2L 6\n");
            if (corner_white_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);

            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);

        }
    
        // F2L 7 work
        else if (edge_color_face == face_after_corner_white_face && edge_color_face2 == FACE_IDX_BOTTOM && corner_color_face2 == FACE_IDX_BOTTOM) {
            printf("F2L 7\n");
            if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            for (int i = 0; i < 2; i++) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }  
        }
    
        // F2L 8
        else if (edge_color_face2 == face_before_corner_white_face && edge_color_face == FACE_IDX_BOTTOM && corner_color_face == FACE_IDX_BOTTOM) {
            printf("F2L 8\n");
            if (corner_white_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            for (int i = 0; i < 2; i++) {
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }
    
        // Reposition Edge and Flip Corner

        // F2L 9
        else if (edge_color_face2 == face_on_the_other_side_of_white && edge_color_face == FACE_IDX_BOTTOM && corner_color_face2 == FACE_IDX_BOTTOM) {
            printf("F2L 9\n");
            if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);            
        }
    
        // F2L 10 work
        else if (edge_color_face == face_on_the_other_side_of_white && edge_color_face2 == FACE_IDX_BOTTOM && corner_color_face == FACE_IDX_BOTTOM) {
            printf("F2L 10\n");
            if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            // move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }
    
        // F2L 11 work
        else if (edge_color_face2 == face_before_corner_white_face && edge_color_face == FACE_IDX_BOTTOM && corner_color_face2 == FACE_IDX_BOTTOM) {
            printf("F2L 11\n");
            if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);          
        }
    
        // F2L 12 work
        else if (edge_color_face == face_after_corner_white_face && edge_color_face2 == FACE_IDX_BOTTOM && corner_color_face == FACE_IDX_BOTTOM) {
            printf("F2L 12\n");
            if (corner_white_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }
            //R' U2 R2 U R2 U R
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
        }

        // F2L 13 work
        else if (edge_color_face2 == corner_white_face && edge_color_face == FACE_IDX_BOTTOM && corner_color_face2 == FACE_IDX_BOTTOM) {
            printf("F2L 13\n");
            if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }
    
        // F2L 14 work
        else if (edge_color_face == corner_white_face && edge_color_face2 == FACE_IDX_BOTTOM && corner_color_face == FACE_IDX_BOTTOM) {
            printf("F2L 14\n");
            if (corner_white_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }
    
        // Split Pair by Going Over

        // F2L 15 work
        else if (edge_color_face == corner_white_face && edge_color_face2 == FACE_IDX_BOTTOM && corner_color_face2 == FACE_IDX_BOTTOM) {
            printf("F2L 15\n");
            if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }
    
        // F2L 16 work
        else if (edge_color_face2 == corner_white_face && edge_color_face == FACE_IDX_BOTTOM && corner_color_face == FACE_IDX_BOTTOM) {
            printf("F2L 16\n");
            if (corner_white_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
        }
    
        // F2L 17 work
        else if (edge_color_face == corner_color_face2 && edge_color_face2 == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 17\n");
            if (corner_color_face2 == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_color_face2 == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face2 == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 18 work
        else if (edge_color_face2 == corner_color_face && edge_color_face == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 18\n");
            if (corner_color_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
            for (int i = 0; i < 2; i++) {
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
        }
    
        
        // Pair Made on Side

        // F2L 19 work
        else if (edge_color_face == face_on_the_other_side_of_color && edge_color_face2 == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 19\n");
            if (corner_color_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            for (int i = 0; i < 2; i++) {
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);   
        }
    
        // F2L 20
        else if (edge_color_face2 == face_on_the_other_side_of_color2 && edge_color_face == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 20\n");
            if (corner_color_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);          
        }
    
        // F2L 21 work
        else if (edge_color_face == face_on_the_other_side_of_color2 && edge_color_face2 == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 21\n");

            if (corner_color_face2 == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (corner_color_face2 == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face2 == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }
    
        // F2L 22 work
        else if (edge_color_face2 == face_on_the_other_side_of_color && edge_color_face == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 22\n");
            if (corner_color_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
        }
    

        // Weird

        // F2L 23 work
        else if (edge_color_face == corner_color_face && edge_color_face2 == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 23\n");
            if (corner_color_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 24 work
        else if (edge_color_face2 == corner_color_face2 && edge_color_face == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 24\n");
            if (corner_color_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 25
        else if (edge_color_face2 == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_TOP && corner_color_face == face) {
            printf("F2L 25\n");
            if (edge_color_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (edge_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (edge_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 26 work
        else if (edge_color_face == FACE_IDX_BOTTOM && corner_white_face == FACE_IDX_TOP && corner_color_face == face) {
            printf("F2L 26\n");
            if (edge_color_face2 == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (edge_color_face2 == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (edge_color_face2 == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
        }
            
        // F2L 27 work
        else if (edge_color_face2 == FACE_IDX_BOTTOM && corner_color_face == FACE_IDX_TOP) {
            printf("F2L 27\n");
            if (edge_color_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (edge_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (edge_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 28 work
        else if (edge_color_face == FACE_IDX_BOTTOM && corner_color_face2 == FACE_IDX_TOP) {
            printf("F2L 28\n");
            if (edge_color_face2 == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (edge_color_face2 == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (edge_color_face2 == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
        }

        // F2L 29
        else if (edge_color_face == FACE_IDX_BOTTOM && corner_color_face == FACE_IDX_TOP) {
            printf("F2L 29\n");
            if (edge_color_face2 == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (edge_color_face2 == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (edge_color_face2 == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            for (int i = 0; i < 2; i++) {
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
        }

        // F2L 30 work
        else if (edge_color_face2 == FACE_IDX_BOTTOM && corner_color_face2 == FACE_IDX_TOP) {
            printf("F2L 30\n");
            if (edge_color_face == face2) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            } else if (edge_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (edge_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // Edge in Place, Corner in D face

        // F2L 31 work
        else if (edge_color_face == face2 && edge_color_face2 == face && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 31\n");
            if (corner_color_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
        }

        // F2L 32 work
        else if (edge_color_face == face && edge_color_face2 == face2 && corner_white_face == FACE_IDX_BOTTOM) {
            printf("F2L 32\n");
            if (corner_color_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_color_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            for (int i = 0; i < 3; i++) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
        }

        // F2L 33 work
        else if (edge_color_face == face && edge_color_face2 == face2 && corner_color_face2 == FACE_IDX_BOTTOM) {
            printf("F2L 33\n");
            if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 34 work
        else if (edge_color_face == face && edge_color_face2 == face2 && corner_color_face == FACE_IDX_BOTTOM) {
            printf("F2L 34\n");
            if (corner_color_face2 == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_color_face2 == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_color_face2 == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 35 work
        else if (edge_color_face == face2 && edge_color_face2 == face && corner_color_face2 == FACE_IDX_BOTTOM) {
            printf("F2L 35\n");
            if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
        }

        // F2L 36 work
        else if (edge_color_face == face2 && edge_color_face2 == face && corner_color_face == FACE_IDX_BOTTOM) {
            printf("F2L 36\n");
            if (corner_white_face == face) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            } else if (corner_white_face == face4) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            } else if (corner_white_face == face3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // Edge and Corner in Place
        
        // F2L 37
        // solved state

        // F2L 38 work
        else if (edge_color_face == face2 && edge_color_face2 == face && corner_white_face == FACE_IDX_TOP && corner_color_face2 == face2) {
            printf("F2L 38\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 39
        else if (edge_color_face == face && edge_color_face2 == face2 && corner_color_face == FACE_IDX_TOP && corner_color_face2 == face) {
            printf("F2L 39\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 40
        else if (edge_color_face == face && edge_color_face2 == face2 && corner_color_face2 == FACE_IDX_TOP && corner_color_face == face2) {
            printf("F2L 40\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
        }

        // F2L 41
        else if (edge_color_face == face2 && edge_color_face2 == face && corner_color_face == FACE_IDX_TOP && corner_color_face2 == face) {
            printf("F2L 41\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }

        // F2L 42 work
        else if (edge_color_face == face2 && edge_color_face2 == face && corner_color_face2 == FACE_IDX_TOP && corner_color_face == face2) {
            printf("F2L 42\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
        }
        
        else {
            printf("AHTUNG\n");
            printf(scene_get_cube_state_as_string(cubeColors));
        }
        
    
    }   

}

static void solve_OLL(RGBColor (*cubeColors)[9], MoveSequence* solution) {
    printf("Solving OLL...\n");
    int positions_bottom[9];
    int positions_sides[4][3];

    RGBColor (*temp_cubeColors)[9] = malloc(sizeof(RGBColor[6][9]));
    copy_cube_state(cubeColors, temp_cubeColors);

    rotate_face_colors(temp_cubeColors, FACE_IDX_BOTTOM, ROTATE_CLOCKWISE);
    get_yellow_positions(temp_cubeColors, positions_bottom, positions_sides);

    FaceIndex face = FACE_IDX_RIGHT;
    FaceIndex face2 = FACE_IDX_BACK;
    FaceIndex face3 = FACE_IDX_LEFT;
    FaceIndex face4 = FACE_IDX_FRONT;

    Move moves[] = {MOVE_D_PRIME, MOVE_D2, MOVE_D};



    for (int i = 0; i < 4; i++) {
        if (bottom_equal(positions_bottom, b_OLL_1) && sides_equal(positions_sides, s_OLL_1)) {
            printf("OLL 1\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_2) && sides_equal(positions_sides, s_OLL_2)) {
            printf("OLL 2\n");

            // F R U R' U' F' U2 F U R U' R' F'
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_3) && sides_equal(positions_sides, s_OLL_3)) {
            printf("OLL 3\n");

            // U F U2 F R' F' R U R U R' U F'
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_4) && sides_equal(positions_sides, s_OLL_4)) {
            printf("OLL 4\n");

            // R' U' F' U' F R U' R' F' U' F U' R
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_5) && sides_equal(positions_sides, s_OLL_5)) {
            printf("OLL 5\n");

            // F R U R' U' F' U' F R U R' U' F'
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_6) && sides_equal(positions_sides, s_OLL_6)) {
            printf("OLL 6\n");

            // U2 F' U' F2 R' F' R U R U2 R' (U2 не было)
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_7) && sides_equal(positions_sides, s_OLL_7)) {
            printf("OLL 7\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_8) && sides_equal(positions_sides, s_OLL_8)) {
            printf("OLL 8\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_9) && sides_equal(positions_sides, s_OLL_9)) {
            printf("OLL 9\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_10) && sides_equal(positions_sides, s_OLL_10)) {
            printf("OLL 10\n");

            // U' R U R' U R' F R F' R U2 R'
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);          
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_11) && sides_equal(positions_sides, s_OLL_11)) {
            printf("OLL 11\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_12) && sides_equal(positions_sides, s_OLL_12)) {
            printf("OLL 12\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_13) && sides_equal(positions_sides, s_OLL_13)) {
            printf("OLL 13\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_14) && sides_equal(positions_sides, s_OLL_14)) {
            printf("OLL 14\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_15) && sides_equal(positions_sides, s_OLL_15)) {
            printf("OLL 15\n");

            // U2 F R U R' U' F' U R U R' U R U2 R'
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_16) && sides_equal(positions_sides, s_OLL_16)) {
            printf("OLL 16\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_17) && sides_equal(positions_sides, s_OLL_17)) {
            printf("OLL 17\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_18) && sides_equal(positions_sides, s_OLL_18)) {
            printf("OLL 18\n");

            // U' F R' F' R U R U' R' U F R U R' U' F'
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_19) && sides_equal(positions_sides, s_OLL_19)) {
            printf("OLL 19\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_20) && sides_equal(positions_sides, s_OLL_20)) {
            printf("OLL 20\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_TOP, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_TOP, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_180), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_21) && sides_equal(positions_sides, s_OLL_21)) {
            printf("OLL 21\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_22) && sides_equal(positions_sides, s_OLL_22)) {
            printf("OLL 22\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_23) && sides_equal(positions_sides, s_OLL_23)) {
            printf("OLL 23\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_TOP, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_TOP, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_24) && sides_equal(positions_sides, s_OLL_24)) {
            printf("OLL 24\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_25) && sides_equal(positions_sides, s_OLL_25)) {
            printf("OLL 25\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_26) && sides_equal(positions_sides, s_OLL_26)) {
            printf("OLL 26\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_27) && sides_equal(positions_sides, s_OLL_27)) {
            printf("OLL 27\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_28) && sides_equal(positions_sides, s_OLL_28)) {
            printf("OLL 28\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_29) && sides_equal(positions_sides, s_OLL_29)) {
            printf("OLL 29\n");

            // U R U R' U' R U' R' F' U' F R U R'
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_30) && sides_equal(positions_sides, s_OLL_30)) {
            printf("OLL 30\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_31) && sides_equal(positions_sides, s_OLL_31)) {
            printf("OLL 31\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_32) && sides_equal(positions_sides, s_OLL_32)) {
            printf("OLL 32\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_33) && sides_equal(positions_sides, s_OLL_33)) {
            printf("OLL 33\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_34) && sides_equal(positions_sides, s_OLL_34)) {
            printf("OLL 34\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_35) && sides_equal(positions_sides, s_OLL_35)) {
            printf("OLL 35\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_36) && sides_equal(positions_sides, s_OLL_36)) {
            printf("OLL 36\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_37) && sides_equal(positions_sides, s_OLL_37)) {
            printf("OLL 37\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_38) && sides_equal(positions_sides, s_OLL_38)) {
            printf("OLL 38\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_39) && sides_equal(positions_sides, s_OLL_39)) {
            printf("OLL 39\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_40) && sides_equal(positions_sides, s_OLL_40)) {
            printf("OLL 40\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_41) && sides_equal(positions_sides, s_OLL_41)) {
            printf("OLL 41\n");

            // R U' R' U2 R U B U' B' U' R'
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }   
        else if (bottom_equal(positions_bottom, b_OLL_42) && sides_equal(positions_sides, s_OLL_42)) {
            printf("OLL 42\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_43) && sides_equal(positions_sides, s_OLL_43)) {
            printf("OLL 43\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_44) && sides_equal(positions_sides, s_OLL_44)) {
            printf("OLL 44\n");

            // B U L U' L' B'
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_45) && sides_equal(positions_sides, s_OLL_45)) {
            printf("OLL 45\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_46) && sides_equal(positions_sides, s_OLL_46)) {
            printf("OLL 46\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_47) && sides_equal(positions_sides, s_OLL_47)) {
            printf("OLL 47\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_48) && sides_equal(positions_sides, s_OLL_48)) {
            printf("OLL 48\n");

            // F R U R' U' R U R' U' F'
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_49) && sides_equal(positions_sides, s_OLL_49)) {
            printf("OLL 49\n");

            // R B' R2 F R2 B R2 F' R
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            break;
        }   
        else if (bottom_equal(positions_bottom, b_OLL_50) && sides_equal(positions_sides, s_OLL_50)) {
            printf("OLL 50\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            break;
        }
        else if (bottom_equal(positions_bottom, b_OLL_51) && sides_equal(positions_sides, s_OLL_51)) {
            printf("OLL 51\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        } 
        else if (bottom_equal(positions_bottom, b_OLL_52) && sides_equal(positions_sides, s_OLL_52)) {
            printf("OLL 52\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            break;
        } 
        else if (bottom_equal(positions_bottom, b_OLL_53) && sides_equal(positions_sides, s_OLL_53)) {
            printf("OLL 53\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        } 
        else if (bottom_equal(positions_bottom, b_OLL_54) && sides_equal(positions_sides, s_OLL_54)) {
            printf("OLL 54\n");

            // F' L' U' L U F L' U' L U L F' L' F
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            break;
        } 
        else if (bottom_equal(positions_bottom, b_OLL_55) && sides_equal(positions_sides, s_OLL_55)) {
            printf("OLL 55\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        } 
        else if (bottom_equal(positions_bottom, b_OLL_56) && sides_equal(positions_sides, s_OLL_56)) {
            printf("OLL 56\n");

            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        } 
        else if (bottom_equal(positions_bottom, b_OLL_57) && sides_equal(positions_sides, s_OLL_57)) {
            printf("OLL 57\n");

            // L' R U R' U' L R' F R F'
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
            move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
            break;
        } else {
            printf("Position unknown\n");
        }

        face = (face % 4) + 1;
        face2 = (face2 % 4) + 1;
        face3 = (face3 % 4) + 1;
        face4 = (face4 % 4) + 1;

        rotate_face_colors(temp_cubeColors, FACE_IDX_BOTTOM, ROTATE_CLOCKWISE);
        get_yellow_positions(temp_cubeColors, positions_bottom, positions_sides);
    }
    free(temp_cubeColors);
}

static void solve_PLL(RGBColor (*cubeColors)[9], MoveSequence* solution) {
    printf("Solve PLL\n");

    if (is_cube_solved(cubeColors)) {
        printf("Cube is already solved!\n");
        return;
    }

    int positions_sides[4][3];
    get_side_positions(cubeColors, positions_sides);



    Move moves[] = {MOVE_D_PRIME, MOVE_D2, MOVE_D};
    Move return_moves[] = {MOVE_D, MOVE_D2, MOVE_D_PRIME};


    FaceIndex face = FACE_IDX_FRONT;
    FaceIndex face2 = FACE_IDX_RIGHT;
    FaceIndex face3 = FACE_IDX_BACK;
    FaceIndex face4 = FACE_IDX_LEFT;
    int id1 = 0, id2 = 1, id3 = 2, id4 = 3;
    bool f = false;

    for (int i = 0; i < 4; i++) {

        // printf("positions_sides[%d][0]: %d, positions_sides[%d][1]: %d, positions_sides[%d][2]: %d\n", id1, positions_sides[id1][0], id1, positions_sides[id1][1], id1, positions_sides[id1][2]);
        // printf("positions_sides[%d][0]: %d, positions_sides[%d][1]: %d, positions_sides[%d][2]: %d\n", id2, positions_sides[id2][0], id2, positions_sides[id2][1], id2, positions_sides[id2][2]);
        // printf("positions_sides[%d][0]: %d, positions_sides[%d][1]: %d, positions_sides[%d][2]: %d\n", id3, positions_sides[id3][0], id3, positions_sides[id3][1], id3, positions_sides[id3][2]);
        // printf("positions_sides[%d][0]: %d, positions_sides[%d][1]: %d, positions_sides[%d][2]: %d\n", id4, positions_sides[id4][0], id4, positions_sides[id4][1], id4, positions_sides[id4][2]);

        // If all the corners are in place
        if (
            positions_sides[id1][0] == 1 && positions_sides[id1][2] == 1 && 
            positions_sides[id2][0] == 2 && positions_sides[id2][2] == 2 && 
            positions_sides[id3][0] == 3 && positions_sides[id3][2] == 3 && 
            positions_sides[id4][0] == 4 && positions_sides[id4][2] == 4
        ) {
            printf("All corners are in place\n");
            if (id1 != 0) {
                move_sequence_add(solution, moves[id1-1], cubeColors);
                get_side_positions(cubeColors, positions_sides);
                if (is_cube_solved(cubeColors)) {
                    return;
                }
            }

            // H Permutation
            if (positions_sides[0][1] == 3 && positions_sides[1][1] == 4 && positions_sides[2][1] == 1 && positions_sides[3][1] == 2) {
                printf("H Permutation\n");

                move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
                return;
            } 

            // Z Permutation
            if (positions_sides[0][1] == 2 && positions_sides[1][1] == 1 && positions_sides[2][1] == 4 && positions_sides[3][1] == 3) {
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                f = true;
            }
            if ((positions_sides[0][1] == 4 && positions_sides[3][1] == 1 && positions_sides[1][1] == 3 && positions_sides[2][1] == 2) || f) {
                printf("Z Permutation\n");

                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_180), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
                
                if (f) {
                    move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                } else {
                    move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
                }
                return;
            }
            
            // U Permutations
            for (int j = 0; j < 4; j++) {
                // If one side has all the same color
                if (positions_sides[j][0] == positions_sides[j][1] && positions_sides[j][2] == positions_sides[j][1]) {
                    int non_color = positions_sides[j][0];

                    int next_color = (non_color % 4) + 1;
                    int prev_id = j == 0? 3 : j - 1;

                    // U Permutation : b
                    if (positions_sides[prev_id][1] == next_color) {
                        printf("U Permutation: b\n");

                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_180), cubeColors);
                    } else {
                        printf("U Permutation: a\n");

                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_180), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
                    }
                    return;
                }
                face = (face % 4) + 1;
                face2 = (face2 % 4) + 1;
                face3 = (face3 % 4) + 1;
                face4 = (face4 % 4) + 1;
            }
        }

        // If all the edges are in place
        else if (
            positions_sides[id1][1] == 1 &&
            positions_sides[id2][1] == 2 &&
            positions_sides[id3][1] == 3 &&
            positions_sides[id4][1] == 4
        ) 
        {
            printf("All edges are in place\n");
            if (id1 != 0) {
                move_sequence_add(solution, moves[id1-1], cubeColors);
                get_side_positions(cubeColors, positions_sides);
                if (is_cube_solved(cubeColors)) {
                    return;
                }
            }
            
            for (int j = 0; j < 4; j++) {
                int j_next = j == 3? 0: j + 1;
                int j_prev = j == 0? 3 : j - 1;

                // Aa/Ab Permutation
                // Find the corner with all the same color
                if (positions_sides[j][1] == positions_sides[j][2] && positions_sides[j_next][0] == positions_sides[j_next][1]) {
                    if (positions_sides[j][0] != positions_sides[j_prev][1]) {
                        printf("Aa Permutation\n");

                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_180), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_180), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_180), cubeColors);
                        
                    } else {
                        printf("Ab Permutation\n");

                        move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_180), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_180), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_180), cubeColors);
                    }
                    return;
                } 
                
                // E Permutation
                if (positions_sides[j][1] == positions_sides[j_next][0] && positions_sides[j][1] == positions_sides[j_prev][2]) {
                    printf("E Permutation\n");
                    
                    move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_180), cubeColors);
                    move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                    move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
                    for (int i = 0; i < 3; i++) {
                        move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_COUNTERCLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_CLOCKWISE), cubeColors);
                        move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                    }
                    move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
                    move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                    move_sequence_add(solution, get_move_from_face_and_direction(face2, ROTATE_180), cubeColors);
                    return;
                }
                
                face = (face % 4) + 1;
                face2 = (face2 % 4) + 1;
                face3 = (face3 % 4) + 1;
                face4 = (face4 % 4) + 1;
            }
        }
        
        id1 = id1 == 3? 0 : id1 + 1;
        id2 = id2 == 3? 0 : id2 + 1;
        id3 = id3 == 3? 0 : id3 + 1;
        id4 = id4 == 3? 0 : id4 + 1;
    }

    for (int i = 0; i < 4; i++) {
        // if one side has all the same color
        if (
            positions_sides[id1][0] == positions_sides[id1][1] && 
            positions_sides[id1][2] == positions_sides[id1][1]
        ) {
            printf("One side has all the same color\n");

            if (id1 != 0) {
                move_sequence_add(solution, moves[id1-1], cubeColors);
                get_side_positions(cubeColors, positions_sides);
                if (is_cube_solved(cubeColors)) {
                    return;
                }
            }

            // F Permutation
            if (positions_sides[2][1] == positions_sides[1][2] && positions_sides[2][1] == positions_sides[3][0]) {
                printf("F Permutation\n");

                move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_180), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face4, ROTATE_180), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
            }
            // J Permutation: a
            else if (positions_sides[2][1] == positions_sides[2][2] && positions_sides[3][0] == positions_sides[1][2]) {
                printf("J Permutation: a\n");

                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_180), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_COUNTERCLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(face3, ROTATE_CLOCKWISE), cubeColors);
                move_sequence_add(solution, get_move_from_face_and_direction(FACE_IDX_BOTTOM, ROTATE_CLOCKWISE), cubeColors);
                
                int idx = (0 - positions_sides[0][1] - 1 + 4) % 4;
                if (idx != 0) {
                    move_sequence_add(solution, return_moves[idx], cubeColors);
                }
            }
            return;
        }

        id1 = id1 == 3? 0 : id1 + 1;
        id2 = id2 == 3? 0 : id2 + 1;
        id3 = id3 == 3? 0 : id3 + 1;
        id4 = id4 == 3? 0 : id4 + 1;
    }
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
    solve_F2L(working_colors, &solution);
    solve_OLL(working_colors, &solution);
    solve_PLL(working_colors, &solution);
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