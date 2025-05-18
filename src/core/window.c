#define _USE_MATH_DEFINES

#include "window.h"
#include "application.h"
#include "../scene/scene.h"
#include <stdio.h>
#include <math.h>

static void framebuffer_size_callback(GLFWwindow* handle, int width, int height) {
    glViewport(0, 0, width, height);
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->width = width;
    window->height = height;
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
        
        // Rubik's Cube Controls
        // Only handle these if we have a valid application reference
        struct Application* app = window->app;
        if (app && app->scene.isRubiksCube && !scene_is_rotating(&app->scene)) {
            // Check for rotation keys only if no rotation is currently in progress
            int direction = (mods & GLFW_MOD_SHIFT) ? -1 : 1; // -1 for counter-clockwise, 1 for clockwise
            
            // Top face rotation
            if (key == GLFW_KEY_U) {
                scene_rotate_top(&app->scene, direction);
                printf("Rotating top face %s\n", 
                       direction == 1 ? "clockwise" : "counter-clockwise");
            }
            
            // Bottom face rotation
            if (key == GLFW_KEY_D) {
                scene_rotate_bottom(&app->scene, direction);
                printf("Rotating bottom face %s\n", 
                       direction == 1 ? "clockwise" : "counter-clockwise");
            }
            
            // Front face rotation
            if (key == GLFW_KEY_F) {
                scene_rotate_front(&app->scene, direction);
                printf("Rotating front face %s\n", 
                       direction == 1 ? "clockwise" : "counter-clockwise");
            }
            
            // Back face rotation
            if (key == GLFW_KEY_B) {
                scene_rotate_back(&app->scene, direction);
                printf("Rotating back face %s\n", 
                       direction == 1 ? "clockwise" : "counter-clockwise");
            }
            
            // Left face rotation
            if (key == GLFW_KEY_L) {
                scene_rotate_left(&app->scene, direction);
                printf("Rotating left face %s\n", 
                       direction == 1 ? "clockwise" : "counter-clockwise");
            }
            
            // Right face rotation
            if (key == GLFW_KEY_R) {
                scene_rotate_right(&app->scene, direction);
                printf("Rotating right face %s\n", 
                       direction == 1 ? "clockwise" : "counter-clockwise");
            }
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