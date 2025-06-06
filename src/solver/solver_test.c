#include <stdio.h>
#include <stdlib.h>
#include "cube_solver.h"
#include "../scene/scene.h"

// Function to apply a solution to the actual scene
void apply_solution_to_scene(Scene* scene, MoveSequence* solution) {
    if (!scene || !solution) {
        return;
    }
    
    printf("Applying solution to scene...\n");
    
    for (int i = 0; i < solution->count; i++) {
        Move move = solution->moves[i];
        FaceIndex face = move_to_face(move);
        RotationDirection direction = move_to_direction(move);
        
        printf("Applying move %d: %s (Face: %d, Direction: %d)\n", 
               i + 1, move_to_string(move), face, direction);
        
        // Apply the move to the scene's cube colors
        rotate_face_colors(scene->cubeColors, face, direction);
        
        // Update the visual representation
        scene_rotate_face(scene, face, direction);
    }
    
    printf("Solution applied successfully!\n");
}

// Function to scramble the cube for testing
void scramble_cube(Scene* scene, int moves) {
    printf("Scrambling cube with %d random moves...\n", moves);
    
    // Simple scrambling algorithm
    for (int i = 0; i < moves; i++) {
        FaceIndex face = rand() % 6;
        RotationDirection direction = (rand() % 2) ? ROTATE_CLOCKWISE : ROTATE_COUNTERCLOCKWISE;
        
        Move move = get_move_from_face_and_direction(face, direction);
        printf("Scramble move %d: %s\n", i + 1, move_to_string(move));
        
        rotate_face_colors(scene->cubeColors, face, direction);
        scene_rotate_face(scene, face, direction);
    }
    
    printf("Cube scrambled!\n");
}

// Function to print the current cube state in a readable format
void print_cube_state(Scene* scene) {
    printf("\nCurrent cube state:\n");
    
    const char* face_names[] = {"Top (U)", "Front (F)", "Right (R)", "Back (B)", "Left (L)", "Bottom (D)"};
    
    for (int face = 0; face < 6; face++) {
        printf("%s:\n", face_names[face]);
        for (int row = 0; row < 3; row++) {
            printf("  ");
            for (int col = 0; col < 3; col++) {
                int pos = row * 3 + col;
                RGBColor color = scene->cubeColors[face][pos];
                
                // Convert RGB to character representation
                char colorChar = '?';
                const float tolerance = 0.1f;
                
                if (fabs(color.r - 1.0f) < tolerance && fabs(color.g - 1.0f) < tolerance && fabs(color.b - 1.0f) < tolerance) {
                    colorChar = 'W';
                } else if (fabs(color.r - 1.0f) < tolerance && fabs(color.g - 1.0f) < tolerance && fabs(color.b - 0.0f) < tolerance) {
                    colorChar = 'Y';
                } else if (fabs(color.r - 1.0f) < tolerance && fabs(color.g - 0.0f) < tolerance && fabs(color.b - 0.0f) < tolerance) {
                    colorChar = 'R';
                } else if (fabs(color.r - 1.0f) < tolerance && fabs(color.g - 0.5f) < tolerance && fabs(color.b - 0.0f) < tolerance) {
                    colorChar = 'O';
                } else if (fabs(color.r - 0.0f) < tolerance && fabs(color.g - 0.0f) < tolerance && fabs(color.b - 1.0f) < tolerance) {
                    colorChar = 'B';
                } else if (fabs(color.r - 0.0f) < tolerance && fabs(color.g - 0.8f) < tolerance && fabs(color.b - 0.0f) < tolerance) {
                    colorChar = 'G';
                }
                
                printf("%c ", colorChar);
            }
            printf("\n");
        }
        printf("\n");
    }
}

// Test function for the solver
int test_cube_solver() {
    printf("=== Rubik's Cube Solver Test ===\n\n");
    
    // Initialize a scene with Rubik's cube
    Scene scene;
    if (!scene_init_rubiks(&scene)) {
        printf("Failed to initialize Rubik's cube scene\n");
        return -1;
    }
    
    printf("1. Initial solved cube state:\n");
    print_cube_state(&scene);
    
    // Check if solver recognizes solved cube
    if (is_cube_solved(scene.cubeColors)) {
        printf("✓ Solver correctly recognizes solved cube\n\n");
    } else {
        printf("✗ Error: Solver doesn't recognize solved cube\n\n");
    }
    
    // Scramble the cube
    printf("2. Scrambling cube...\n");
    scramble_cube(&scene, 10);
    print_cube_state(&scene);
    
    // Test the solver
    printf("3. Running solver...\n");
    MoveSequence* solution = cube_solver_solve(&scene);
    
    if (solution) {
        printf("\n4. Solution found:\n");
        move_sequence_print(solution);
        
        // Apply solution to scene (for visual verification)
        printf("\n5. Applying solution to scene...\n");
        apply_solution_to_scene(&scene, solution);
        
        printf("\n6. Final cube state after applying solution:\n");
        print_cube_state(&scene);
        
        // Check if cube is solved
        if (is_cube_solved(scene.cubeColors)) {
            printf("✓ Cube successfully solved!\n");
        } else {
            printf("✗ Cube is not fully solved. Algorithm needs improvement.\n");
        }
        
        // Clean up
        move_sequence_destroy(solution);
        free(solution);
    } else {
        printf("✗ Failed to generate solution\n");
    }
    
    // Clean up scene
    scene_destroy(&scene);
    
    printf("\n=== Test Complete ===\n");
    return 0;
}

// Example usage function
void demonstrate_solver_usage(Scene* scene) {
    printf("\n=== Solver Usage Example ===\n");
    
    if (!scene || !scene->isRubiksCube) {
        printf("Error: Invalid scene or not a Rubik's cube\n");
        return;
    }
    
    // 1. Get current cube state
    printf("1. Getting current cube colors...\n");
    RGBColor* cube_colors = scene_get_cube_colors(scene);
    
    // 2. Check if cube is solved
    printf("2. Checking if cube is solved...\n");
    if (is_cube_solved(cube_colors)) {
        printf("   Cube is already solved!\n");
        return;
    }
    
    // 3. Run the solver
    printf("3. Running solver algorithm...\n");
    MoveSequence* solution = cube_solver_solve(scene);
    
    if (solution) {
        // 4. Display the solution
        printf("4. Solution found:\n   ");
        move_sequence_print(solution);
        
        // 5. Optionally apply each move individually
        printf("5. You can apply moves one by one:\n");
        for (int i = 0; i < solution->count && i < 5; i++) { // Show first 5 moves as example
            Move move = solution->moves[i];
            printf("   Move %d: %s\n", i + 1, move_to_string(move));
            
            // To apply this move to your scene:
            // FaceIndex face = move_to_face(move);
            // RotationDirection direction = move_to_direction(move);
            // rotate_face_colors(scene->cubeColors, face, direction);
        }
        
        if (solution->count > 5) {
            printf("   ... and %d more moves\n", solution->count - 5);
        }
        
        // Clean up
        move_sequence_destroy(solution);
        free(solution);
    } else {
        printf("   Failed to find solution\n");
    }
    
    printf("=== Example Complete ===\n");
}

// Main function (if running as standalone)
#ifdef SOLVER_STANDALONE_TEST
int main() {
    return test_cube_solver();
}
#endif 