#include <stdio.h>
#include <stdlib.h>
#include "cube_solver.h"
#include "../scene/scene.h"

// Example: How to use the Rubik's cube solver
int main() {
    printf("=== Rubik's Cube Solver Example ===\n\n");
    
    // Step 1: Initialize a scene with Rubik's cube
    Scene scene;
    if (!scene_init_rubiks(&scene)) {
        printf("Failed to initialize Rubik's cube scene\n");
        return -1;
    }
    
    printf("✓ Rubik's cube scene initialized\n");
    
    // Step 2: Get current cube colors using the required function
    printf("✓ Getting cube colors using scene_get_cube_colors()...\n");
    RGBColor* cube_colors = scene_get_cube_colors(&scene);
    
    // Step 3: Check if cube is already solved
    if (is_cube_solved(cube_colors)) {
        printf("✓ Cube is already solved!\n");
    }
    
    // Step 4: Scramble the cube for demonstration
    printf("🔄 Scrambling cube for demonstration...\n");
    
    // Apply some random moves using rotate_face_colors
    rotate_face_colors(scene.cubeColors, FACE_IDX_RIGHT, ROTATE_CLOCKWISE);     // R
    rotate_face_colors(scene.cubeColors, FACE_IDX_TOP, ROTATE_CLOCKWISE);       // U
    rotate_face_colors(scene.cubeColors, FACE_IDX_RIGHT, ROTATE_COUNTERCLOCKWISE); // R'
    rotate_face_colors(scene.cubeColors, FACE_IDX_TOP, ROTATE_COUNTERCLOCKWISE);    // U'
    rotate_face_colors(scene.cubeColors, FACE_IDX_FRONT, ROTATE_CLOCKWISE);     // F
    rotate_face_colors(scene.cubeColors, FACE_IDX_RIGHT, ROTATE_CLOCKWISE);     // R
    rotate_face_colors(scene.cubeColors, FACE_IDX_TOP, ROTATE_CLOCKWISE);       // U
    rotate_face_colors(scene.cubeColors, FACE_IDX_RIGHT, ROTATE_COUNTERCLOCKWISE); // R'
    
    printf("✓ Applied scramble moves: R U R' U' F R U R'\n");
    
    // Step 5: Solve the cube using the solver
    printf("🧩 Running cube solver...\n");
    MoveSequence* solution = cube_solver_solve(&scene);
    
    if (solution) {
        printf("✅ Solution found!\n");
        move_sequence_print(solution);
        
        // Step 6: Apply the solution using rotate_face_colors
        printf("🔧 Applying solution using rotate_face_colors()...\n");
        
        for (int i = 0; i < solution->count; i++) {
            Move move = solution->moves[i];
            FaceIndex face = move_to_face(move);
            RotationDirection direction = move_to_direction(move);
            
            printf("   Applying move %d/%d: %s\n", i + 1, solution->count, move_to_string(move));
            
            // This is the key function from scene.c that we're using
            rotate_face_colors(scene.cubeColors, face, direction);
        }
        
        printf("✓ Solution applied successfully!\n");
        
        // Step 7: Verify the cube is solved
        if (is_cube_solved(scene.cubeColors)) {
            printf("🎉 Cube is now solved!\n");
        } else {
            printf("❌ Cube is not fully solved (algorithm needs improvement)\n");
        }
        
        // Clean up
        move_sequence_destroy(solution);
        free(solution);
        
    } else {
        printf("❌ Failed to find solution\n");
    }
    
    // Step 8: Clean up scene
    scene_destroy(&scene);
    
    printf("\n=== Example Complete ===\n");
    printf("\nSummary of functions used:\n");
    printf("- scene_get_cube_colors() : Get current cube state\n");
    printf("- rotate_face_colors()    : Apply moves to cube\n");
    printf("- cube_solver_solve()     : Generate solution\n");
    
    return 0;
}

// Additional example: Working with custom cube states
void example_with_custom_state() {
    printf("\n=== Custom Cube State Example ===\n");
    
    Scene scene;
    if (!scene_init_rubiks(&scene)) {
        return;
    }
    
    // Set a specific cube state (scrambled configuration)
    const char* scrambled_state = "WOWGYBWYORWGYBWRYGOWGYBWROGOWGYBWBYGOWGYBGOWGYBWRYWROG";
    
    if (scene_set_cube_state_from_string(&scene, scrambled_state)) {
        printf("✓ Set custom scrambled state\n");
        
        // Solve this specific state
        MoveSequence* solution = cube_solver_solve(&scene);
        
        if (solution) {
            printf("Solution for custom state:\n");
            move_sequence_print(solution);
            
            move_sequence_destroy(solution);
            free(solution);
        }
    }
    
    scene_destroy(&scene);
} 