#include "core/application.h"
#include <stdio.h>

static void display_help_message();

int main() {
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