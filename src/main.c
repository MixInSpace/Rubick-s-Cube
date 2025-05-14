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
    const char* checkerboard = "WYWYWYWYWBGBGBGBGBRORORORORYWYWYWYWYGBGBGBGBGORORORORO";
    
    // Apply solid colors to verify basic functionality
    if (!scene_set_cube_state_from_string(&app.scene, checkerboard)) {
        fprintf(stderr, "Failed to set cube state from string\n");
    }
    
    application_run(&app);
    
    application_cleanup(&app);
    
    return 0;
} 