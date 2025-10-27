#include "core/application.h"
#include "benchmark/benchmark.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static void display_help_message();

int main(int argc, char** argv) {
    // CLI benchmark mode: --benchmark N [--scramble S] [--out file.csv] [--seed X] [--quiet]
    if (argc >= 2 && (strcmp(argv[1], "--benchmark") == 0 || strcmp(argv[1], "-b") == 0)) {
        int runs = (argc >= 3) ? atoi(argv[2]) : 100;
        int scramble = 25;
        const char* out = "benchmark_results.csv";
        unsigned int seed = 0u;
        int quiet = 0;

        for (int i = 3; i < argc; ++i) {
            if ((strcmp(argv[i], "--scramble") == 0 || strcmp(argv[i], "-s") == 0) && i + 1 < argc) {
                scramble = atoi(argv[++i]);
            } else if ((strcmp(argv[i], "--out") == 0 || strcmp(argv[i], "-o") == 0) && i + 1 < argc) {
                out = argv[++i];
            } else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc) {
                seed = (unsigned int)strtoul(argv[++i], NULL, 10);
            } else if ((strcmp(argv[i], "--quiet") == 0) || (strcmp(argv[i], "-q") == 0)) {
                quiet = 1;
            }
        }

        printf("Running benchmark: runs=%d, scramble=%d, out=%s, seed=%u%s\n", runs, scramble, out, seed, quiet ? ", quiet" : "");
        int rc = run_benchmark(runs, scramble, out, seed, quiet);
        if (rc != 0) {
            fprintf(stderr, "Benchmark failed (rc=%d)\n", rc);
            return rc;
        }
        printf("Benchmark complete. Results saved to %s\n", out);
        return 0;
    }

    Application app;
    
    if (!application_init(&app)) {
        fprintf(stderr, "Failed to initialize application\n");
        return -1;
    }
    
    display_help_message();

    application_run(&app);
    
    application_cleanup(&app);
    
    return 0;
} 

static void display_help_message(){
    printf("\n=== Rubik's Cube Controls ===\n");
    printf("Camera Controls:\n");
    printf("  - Left Mouse Button: Rotate camera view\n");
    printf("  - Mouse Wheel: Zoom in/out\n");
    printf("  - +/-: Adjust field of view\n");
    printf("  - 0: Reset field of view to default\n");
    printf("\n");
    printf("Cube Controls:\n");
    printf("  - U: Rotate top face clockwise\n");
    printf("  - D: Rotate bottom face clockwise\n");
    printf("  - F: Rotate front face clockwise\n");
    printf("  - B: Rotate back face clockwise\n");
    printf("  - L: Rotate left face clockwise\n");
    printf("  - R: Rotate right face clockwise\n");
    printf("  - Shift + Letter: Rotate counter-clockwise\n");
    printf("\n");
    printf("Move sequence controls: \n");
    printf("  - O: Reset cube to solved state\n");
    printf("  - M: Scramble cube\n");
    printf("  - S: Solve cube\n");
    printf("  - 1-2: Patterns\n");
    printf("  - Shift + Char: Play sequence in browse mode\n");
    printf("\n");
    printf("Browse mode controls: \n");
    printf("  - Left/Right Arrow Keys: Rotate camera view\n");
    printf("  - Shift + Up/Down Arrow Keys: Zoom in/out\n");
    printf("  - ESC: Exit browse mode\n");
    printf("\n");
    printf("Color mode controls\n");
    printf("  - 1-6: Set Color for selected element\n");
    printf("  - Left/Right Arrow Keys: Switch between faces\n");
    printf("  - C: Enter/Exit color mode\n\n");
}
