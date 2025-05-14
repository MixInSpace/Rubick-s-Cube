#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include "camera.h"
#include "../types.h"

struct Window {
    GLFWwindow* handle;
    int width;
    int height;
    const char* title;
    bool shouldClose;
    
    // Mouse state for input handling
    double mouseX;
    double mouseY;
    
    // Camera instance
    Camera camera;
};

bool window_init(Window* window, int width, int height, const char* title);
void window_update(Window* window);
void window_swap_buffers(Window* window);
void window_poll_events(void);
void window_destroy(Window* window);
bool window_should_close(Window* window);
void window_set_should_close(Window* window, bool value);

// Get camera-related information
Camera* window_get_camera(Window* window);
Mat4 window_get_view_matrix(Window* window);

#endif /* WINDOW_H */