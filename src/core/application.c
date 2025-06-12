#include "application.h"
#include "../resources/resource_manager.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

bool application_init(Application* app) {
    // Initialize window
    if (!window_init(&app->window, 800, 600, "Rubik's Cube Visualization")) {
        fprintf(stderr, "Failed to initialize window\n");
        return false;
    }
    
    // Set window's application pointer
    app->window.app = (struct Application*)app;
    
    // Initialize resource manager
    if (!resource_manager_init()) {
        fprintf(stderr, "Failed to initialize resource manager\n");
        return false;
    }
    
    // Initialize scene with a Rubik's cube (increased gap for better visibility)
    if (!scene_init_rubiks(&app->scene)) {
        fprintf(stderr, "Failed to initialize Rubik's cube scene\n");
        return false;
    }
    
    app->solvedCount = 0;
    app->running = true;
    app->lastFrame = 0.0f;
    app->deltaTime = 0.0f;
    app->targetFPS = 60.0f;
    app->frameTimeTarget = 1.0f / app->targetFPS;
    app->needsRedraw = true;
    
    return true;
}

// Function to sleep for milliseconds in a cross-platform way
void sleep_ms(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

void application_run(Application* app) {
    // Main loop
    while (app->running && !window_should_close(&app->window)) {
        // Calculate delta time
        float currentFrame = (float)glfwGetTime();
        app->deltaTime = currentFrame - app->lastFrame;
        
        // Poll events (always do this to keep the UI responsive)
        window_poll_events();
        
        // Update scene only if enough time has passed
        if (app->deltaTime >= app->frameTimeTarget) {
            // Update
            scene_update(&app->scene, app->deltaTime);
            app->lastFrame = currentFrame;
            app->needsRedraw = true;
        }
        
        // Render only when necessary
        if (app->needsRedraw) {
            scene_render(&app->scene, &app->window);
            window_swap_buffers(&app->window);
            app->needsRedraw = false;
        }
        
        // Calculate how much time we should sleep to maintain target FPS
        float frameTime = (float)glfwGetTime() - currentFrame;
        if (frameTime < app->frameTimeTarget) {
            int sleepTime = (int)((app->frameTimeTarget - frameTime) * 1000);
            if (sleepTime > 0) {
                sleep_ms(sleepTime);
            }
        }
    }
}

void application_cleanup(Application* app) {
    scene_destroy(&app->scene);
    resource_manager_cleanup();
    window_destroy(&app->window);
} 