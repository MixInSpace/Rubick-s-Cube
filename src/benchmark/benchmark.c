#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdbool.h>

#include "../solver/cube_solver.h"
#include "benchmark.h"

static void set_solved_cube(Scene* scene) {
    // White (U)
    for (int i = 0; i < 9; i++) scene->cubeColors[FACE_IDX_TOP][i] = (RGBColor){1.0f, 1.0f, 1.0f};
    // Red (F)
    for (int i = 0; i < 9; i++) scene->cubeColors[FACE_IDX_FRONT][i] = (RGBColor){1.0f, 0.0f, 0.0f};
    // Blue (R)
    for (int i = 0; i < 9; i++) scene->cubeColors[FACE_IDX_RIGHT][i] = (RGBColor){0.0f, 0.0f, 1.0f};
    // Orange (B)
    for (int i = 0; i < 9; i++) scene->cubeColors[FACE_IDX_BACK][i] = (RGBColor){1.0f, 0.5f, 0.0f};
    // Green (L)
    for (int i = 0; i < 9; i++) scene->cubeColors[FACE_IDX_LEFT][i] = (RGBColor){0.0f, 0.8f, 0.0f};
    // Yellow (D)
    for (int i = 0; i < 9; i++) scene->cubeColors[FACE_IDX_BOTTOM][i] = (RGBColor){1.0f, 1.0f, 0.0f};
}

static size_t count_move_sequence(char** seq) {
    if (!seq) return 0;
    size_t n = 0;
    while (seq[n] != NULL) n++;
    return n;
}

static void free_move_sequence(char** seq) {
    if (!seq) return;
    for (size_t i = 0; seq[i] != NULL; ++i) {
        free(seq[i]);
    }
    free(seq);
}


static Move pick_move_like_gui(char excludeFace, char* outFaceChar) {
    static const char* move_names[] = {
        "U", "U'", "U2",
        "D", "D'", "D2",
        "F", "F'", "F2",
        "B", "B'", "B2",
        "L", "L'", "L2",
        "R", "R'", "R2"
    };
    static const Move move_vals[] = {
        MOVE_U, MOVE_U_PRIME, MOVE_U2,
        MOVE_D, MOVE_D_PRIME, MOVE_D2,
        MOVE_F, MOVE_F_PRIME, MOVE_F2,
        MOVE_B, MOVE_B_PRIME, MOVE_B2,
        MOVE_L, MOVE_L_PRIME, MOVE_L2,
        MOVE_R, MOVE_R_PRIME, MOVE_R2
    };

    int available_idx[18];
    int available_count = 0;
    for (int i = 0; i < 18; ++i) {
        if (move_names[i][0] != excludeFace) {
            available_idx[available_count++] = i;
        }
    }

    int r = rand() % available_count;
    int idx = available_idx[r];
    if (outFaceChar) *outFaceChar = move_names[idx][0];
    return move_vals[idx];
}

int run_benchmark(int runs, int scramble_len, const char* csv_path, unsigned int seed, bool quiet) {
    if (runs <= 0 || scramble_len < 0) return 1;
    if (!csv_path || csv_path[0] == '\0') csv_path = "benchmark_results.csv";

    if (seed != 0u) {
        srand(seed);
    }

    FILE* fp = fopen(csv_path, "w");
    if (!fp) {
        fprintf(stderr, "Failed to open output file: %s\n", csv_path);
        return 2;
    }
    fprintf(fp, "run,moves,solved\n");

    cube_solver_set_quiet(quiet);
    for (int r = 0; r < runs; ++r) {
        Scene scene;
        memset(&scene, 0, sizeof(Scene));
        set_solved_cube(&scene);

        char lastFace = '\0';
        if (scramble_len <= 0) {
            if (!quiet) printf("{NULL};\n");
        } else {
            if (!quiet) printf("{");
            for (int i = 0; i < scramble_len; ++i) {
                char chosenFace = '\0';
                Move m = pick_move_like_gui(lastFace, &chosenFace);
                apply_move_to_cube_colors(scene.cubeColors, m);
                const char* ms = move_to_string(m);
                if (!quiet) {
                if (i > 0) printf(", ");
                printf("\"%s\"", ms);
                }
                lastFace = chosenFace;
            }
            if (!quiet) printf(", NULL};\n");
        }

        bool solved = false;
        char** sequence = cube_solver_solve(&scene, &solved);
        size_t moves = count_move_sequence(sequence);

        fprintf(fp, "%d,%zu,%d\n", r + 1, moves, solved ? 1 : 0);

        free_move_sequence(sequence);
    }

    fclose(fp);
    return 0;
}
