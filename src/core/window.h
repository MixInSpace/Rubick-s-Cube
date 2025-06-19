#ifndef WINDOW_H
#define WINDOW_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdbool.h>
#include "camera.h"
#include "../types.h"
#include "../scene/scene.h"

// Forward declarations
struct Application;

struct Window {
    GLFWwindow* handle;
    int width;
    int height;
    const char* title;
    bool shouldClose;
    
    // Координаты мыши
    double mouseX;
    double mouseY;
    
    // Инстанс камеры
    Camera camera;
    
    // Поинтер на родительское приложение для управления кнопками
    struct Application* app;
};

bool window_init         (Window* window, int width, int height, const char* title);
void window_swap_buffers (Window* window);
void window_poll_events  (void);
void window_destroy      (Window* window);
bool window_should_close (Window* window);

// Get camera-related information
Camera* window_get_camera(Window* window);

#endif /* WINDOW_H */