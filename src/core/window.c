#define _USE_MATH_DEFINES

#include "window.h"
#include "application.h"
#include "../scene/scene.h"
#include "../solver/cube_solver.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

// Forward declarations
static void handle_move_sequence(struct Application* app, char** moveSequence, bool browseMode, float speed);
static char** generate_random_move_sequence(int length);
static char* generate_random_move(char excludeFace);

static void framebuffer_size_callback(GLFWwindow* handle, int width, int height) {
    glViewport(0, 0, width, height);
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->width = width;
    window->height = height;
}


static char** generate_random_move_sequence(int length) {
    char** moveSequence = malloc((length + 1) * sizeof(char*));
    char lastFace = '\0'; // Track the last face used
    
    for (int i = 0; i < length; i++) {
        moveSequence[i] = generate_random_move(lastFace);
        lastFace = moveSequence[i][0]; // Store the face of this move
    }
    moveSequence[length] = NULL;
    return moveSequence;
}

static char* generate_random_move(char excludeFace) {
    char* move_names[] = {
        "U", "U'", "U2",
        "D", "D'", "D2",
        "F", "F'", "F2",
        "B", "B'", "B2",
        "L", "L'", "L2",
        "R", "R'", "R2"
    };
    
    // Create filtered list excluding moves with the same face
    char* available_moves[18];
    int available_count = 0;
    
    for (int i = 0; i < 18; i++) {
        if (move_names[i][0] != excludeFace) {
            available_moves[available_count++] = move_names[i];
        }
    }
    
    int random_move = rand() % available_count;
    return available_moves[random_move];
}

// Helper function to handle move sequences
static void handle_move_sequence(struct Application* app, char** moveSequence, bool browseMode, float speed) {
    if (browseMode) {
        // Load sequence into browse mode
        scene_destroy_move_queue(&app->scene);
        scene_init_move_queue(&app->scene);
        
        // Add moves to queue
        int i = 0;
        while (moveSequence[i] != NULL) {
            scene_add_move_to_queue(&app->scene, moveSequence[i]);
            i++;
        }
        
        scene_enter_browse_mode(&app->scene);
    } else {
        // Execute sequence directly at specified speed
        scene_set_speed_multiplier(&app->scene, speed);
        apply_move_sequence(&app->scene, moveSequence);
        // Note: speed will be reset when sequence completes in scene_process_move_queue
    }
}


static void key_callback(GLFWwindow* handle, int key, int scancode, int action, int mods) {
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    
    // Get camera reference
    Camera* camera = &window->camera;
    
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        window->shouldClose = true;
        glfwSetWindowShouldClose(handle, GLFW_TRUE);
    }
    
    // Field of View adjustment
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        float currentFov = camera->fov;
        
        // Increase FOV with + or =
        if (key == GLFW_KEY_EQUAL || key == GLFW_KEY_KP_ADD) {
            camera_set_fov(camera, currentFov + 5.0f);
            printf("FOV: %.1f degrees\n", camera->fov);
        }
        
        // Decrease FOV with - or _
        if (key == GLFW_KEY_MINUS || key == GLFW_KEY_KP_SUBTRACT) {
            camera_set_fov(camera, currentFov - 5.0f);
            printf("FOV: %.1f degrees\n", camera->fov);
        }
        
        // Reset FOV to default (45 degrees) with 0
        if (key == GLFW_KEY_0 || key == GLFW_KEY_KP_0) {
            camera_set_fov(camera, 45.0f);
            printf("FOV reset to 45.0 degrees\n");
        }
    }
    
    // Rubik's Cube Controls
    // Only handle these if we have a valid application reference
    struct Application* app = window->app;
    if (app && app->scene.isRubiksCube && !scene_is_rotating(&app->scene)) {
        // Check if we're in browse mode
        if (scene_is_in_browse_mode(&app->scene)) {
            // Browse mode navigation
            if (key == GLFW_KEY_RIGHT || key == GLFW_KEY_DOWN) {
                scene_browse_next(&app->scene);
            } else if (key == GLFW_KEY_LEFT || key == GLFW_KEY_UP) {
                scene_browse_previous(&app->scene);
            } else if (key == GLFW_KEY_C) {
                scene_exit_browse_mode(&app->scene);
            }
            return; // Don't process other keys in browse mode
        }
        
        // Check for rotation keys only if no rotation is currently in progress
        int direction = (mods & GLFW_MOD_SHIFT) ? -1 : 1; // -1 for counter-clockwise, 1 for clockwise
        
        // Top face rotation (U key - Up)
        if (key == GLFW_KEY_U) {
            scene_start_rotation(&app->scene, FACE_IDX_TOP, direction, 1);
            printf("Rotating top face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }
        
        // Bottom face rotation (D key - Down)
        if (key == GLFW_KEY_D) {
            scene_start_rotation(&app->scene, FACE_IDX_BOTTOM, direction, 1);
            printf("Rotating bottom face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }
        
        // Front face rotation (F key - Front)
        if (key == GLFW_KEY_F) {
            scene_start_rotation(&app->scene, FACE_IDX_FRONT, direction, 1);
            printf("Rotating front face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }
        
        if (key == GLFW_KEY_B) {
            scene_start_rotation(&app->scene, FACE_IDX_BACK, direction, 1);
            printf("Rotating back face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }
        
        // Left face rotation (L key - Left)
        if (key == GLFW_KEY_L) {
            scene_start_rotation(&app->scene, FACE_IDX_LEFT, direction, 1);
            printf("Rotating left face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }
        
        // Right face rotation (R key - Right)
        if (key == GLFW_KEY_R) {
            scene_start_rotation(&app->scene, FACE_IDX_RIGHT, direction, 1);
            printf("Rotating right face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }

        if (key == GLFW_KEY_O) {
            const char* solidColors = "WWWWWWWWWBBBBBBBBBRRRRRRRRRYYYYYYYYYGGGGGGGGGOOOOOOOOO";
            scene_set_cube_state_from_string(&app->scene, solidColors);
        }

        if (key == GLFW_KEY_E) {
            const char* solidColors = "WBRRGOOBYWBRRGOOBYWBRRGOOBYWBRRGOOBYWBRRGOOBYWBRRGOOBY";
            scene_set_cube_state_from_string(&app->scene, solidColors);
        }

        if (key == GLFW_KEY_M) {
            char** moveSequence = generate_random_move_sequence(30);
            handle_move_sequence(app, moveSequence, mods & GLFW_MOD_SHIFT, 7.0f);
        }
        if (key == GLFW_KEY_S) {
            char** moveSequence = cube_solver_solve(&app->scene);
            handle_move_sequence(app, moveSequence, mods & GLFW_MOD_SHIFT, 2.0f);
        }

        if (key == GLFW_KEY_1) { 
            //B F2 D' R2 F D B' F D' U F' D' L2 F D2 U'
            char* moveSequence[] = {"B", "F2", "D'", "R2", "F", "D", "B'", "F", "D'", "U", "F'", "D'", "L2", "F", "D2", "U'", NULL};
            
            handle_move_sequence(app, moveSequence, mods & GLFW_MOD_SHIFT, 3.0f);
        }
        if (key == GLFW_KEY_2) { 
            //U2 F2 R2 U' L2 D B R' B R' B R' D' L2 U'
            char* moveSequence[] = {"U2", "F2", "R2", "U'", "L2", "D", "B", "R'", "B", "R'", "B", "R'", "D'", "L2", "U'", NULL};
            
            handle_move_sequence(app, moveSequence, mods & GLFW_MOD_SHIFT, 3.0f);
        }

    }
}

static void mouse_button_callback(GLFWwindow* handle, int button, int action, int mods) {
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            // Start camera drag
            camera_start_drag(&window->camera, window->mouseX, window->mouseY);
        } else if (action == GLFW_RELEASE) {
            // End camera drag
            camera_end_drag(&window->camera);
        }
    }
}

static void cursor_pos_callback(GLFWwindow* handle, double xpos, double ypos) {
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    
    // Update current mouse position
    window->mouseX = xpos;
    window->mouseY = ypos;
    
    // Process mouse movement for camera
    camera_process_mouse_movement(&window->camera, xpos, ypos);
}

static void scroll_callback(GLFWwindow* handle, double xoffset, double yoffset) {
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    
    // Process scroll for camera zoom
    camera_process_scroll(&window->camera, yoffset);
}

bool window_init(Window* window, int width, int height, const char* title) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    // Request OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Enable MSAA (Multi-Sample Anti-Aliasing) with 4 samples
    glfwWindowHint(GLFW_SAMPLES, 4);

    window->handle = glfwCreateWindow(width, height, title, NULL, NULL);
    window->width = width;
    window->height = height;
    window->title = title;
    window->shouldClose = false;
    window->mouseX = 0.0;
    window->mouseY = 0.0;
    
    // Initialize camera
    camera_init(&window->camera);

    if (!window->handle) {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        return false;
    }

    glfwSetWindowUserPointer(window->handle, window);
    glfwMakeContextCurrent(window->handle);
    glfwSetFramebufferSizeCallback(window->handle, framebuffer_size_callback);
    glfwSetKeyCallback(window->handle, key_callback);
    glfwSetMouseButtonCallback(window->handle, mouse_button_callback);
    glfwSetCursorPosCallback(window->handle, cursor_pos_callback);
    glfwSetScrollCallback(window->handle, scroll_callback);
    
    // Enable VSync to limit frame rate
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return false;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

    // Configure OpenGL settings
    glViewport(0, 0, width, height);
    
    // Enable MSAA in OpenGL
    glEnable(GL_MULTISAMPLE);
    
    // Print anti-aliasing settings
    GLint samples;
    glGetIntegerv(GL_SAMPLES, &samples);
    printf("MSAA: %d samples\n", samples);
    
    return true;
}

void window_update(Window* window) {
    glfwSwapBuffers(window->handle);
    glfwPollEvents();
    
    if (glfwWindowShouldClose(window->handle)) {
        window->shouldClose = true;
    }
}

void window_swap_buffers(Window* window) {
    glfwSwapBuffers(window->handle);
    
    if (glfwWindowShouldClose(window->handle)) {
        window->shouldClose = true;
    }
}

void window_poll_events(void) {
    glfwPollEvents();
}

void window_destroy(Window* window) {
    glfwDestroyWindow(window->handle);
    glfwTerminate();
}

bool window_should_close(Window* window) {
    return window->shouldClose;
}

void window_set_should_close(Window* window, bool value) {
    window->shouldClose = value;
    glfwSetWindowShouldClose(window->handle, value);
}

Camera* window_get_camera(Window* window) {
    return &window->camera;
}

Mat4 window_get_view_matrix(Window* window) {
    return camera_get_view_matrix(&window->camera);
} 