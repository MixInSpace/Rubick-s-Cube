#include "core/application.h"
#include <stdio.h>

int main() {
    Application app;
    
    if (!application_init(&app)) {
        fprintf(stderr, "Failed to initialize application\n");
        return -1;
    }
    
    // Test setting the cube state from a string
    // Format: "UUUUUUUUURRRRRRRRRFFFFFFFFFDDDDDDDDDLLLLLLLLLBBBBBBBBB"
    // Order in the string: 
    // - U: top face (white)
    // - R: right face (blue)
    // - F: front face (red)
    // - D: bottom face (yellow)
    // - L: left face (green)
    // - B: back face (orange)
    
    // Simple solid colors test - each face should be a solid color
    const char* solidColors = "WWWWWWWWWBBBBBBBBBRRRRRRRRRYYYYYYYYYGGGGGGGGGOOOOOOOOO";
    
    // Scrambled state (uncomment to test)
    //const char* scrambledColors = "WRBRWWRWWBOROBYYBORBOYOBYYRGBGYYGYGORRORGROGWGGBWOWLYLYL";
    
    // Checkerboard pattern (uncomment to test)
    const char* checkerboard = "WYWRORORYGBGORGBGBGBROROBORYWYWYGBGBGBYWYWYWWYWGBORORO";
    
    // Apply solid colors to verify basic functionality
    if (!scene_set_cube_state_from_string(&app.scene, solidColors)) {
        fprintf(stderr, "Failed to set cube state from string\n");
    }
    
    // Display controls instructions
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
    printf("  - ESC: Exit application\n\n");
    
    application_run(&app);
    
    application_cleanup(&app);
    
    return 0;
} 