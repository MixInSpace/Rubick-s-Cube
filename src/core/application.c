#include "application.h"
#include "../renderer/texture.h"
#include <stdio.h>
#include <GLFW/glfw3.h>
#include <time.h>
#include <stdbool.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

// Инициализация приложения
bool application_init(Application* app) {
    // Инициализируем окно
    if (!window_init(&app->window, 800, 600, "Rubik's Cube")) {
        fprintf(stderr, "Failed to initialize window\n");
        return false;
    }
    
    // Устанавливаем указатель на приложение для окна
    app->window.app = (struct Application*)app;
    
    // Инициализируем текстуры
    if (!texture_init_system()) {
        fprintf(stderr, "Failed to initialize texture system\n");
        return false;
    }
    
    printf("Texture system initialized\n");
    
    // Инициализируем сцену с кубом Рубика 
    if (!scene_init_rubiks(&app->scene)) {
        fprintf(stderr, "Failed to initialize Rubik's cube scene\n");
        return false;
    }
    
    app->running = true;
    app->lastFrame = 0.0f;
    app->deltaTime = 0.0f;
    app->targetFPS = 60.0f;
    app->frameTimeTarget = 1.0f / app->targetFPS;
    app->needsRedraw = true;
    
    return true;
}

// Кроссплатформенная функция сна
void sleep_ms(int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    usleep(milliseconds * 1000);
#endif
}

// Цикл приложения
void application_run(Application* app) {
    // Главный цикл приложения
    while (app->running && !window_should_close(&app->window)) {
        // Время с прошлого кадра
        float currentFrame = (float)glfwGetTime();
        app->deltaTime = currentFrame - app->lastFrame;
        
        // Опрашиваем события
        window_poll_events();
        
        // Обновляем сцену только если прошло достаточно времени для достижения целевого FPS
        if (app->deltaTime >= app->frameTimeTarget) {
            scene_update(&app->scene, &app->window, app->deltaTime);
            app->lastFrame = currentFrame;
            app->needsRedraw = true;
        }
        
        // Рисуем только когда необходимо
        if (app->needsRedraw) {
            scene_render(&app->scene, &app->window);
            window_swap_buffers(&app->window);
            app->needsRedraw = false;
        }
        
        // Вычисляем, сколько времени мы должны спать, чтобы поддерживать целевой FPS
        float frameTime = (float)glfwGetTime() - currentFrame;
        if (frameTime < app->frameTimeTarget) {
            int sleepTime = (int)((app->frameTimeTarget - frameTime) * 1000);
            if (sleepTime > 0) {
                sleep_ms(sleepTime);
            }
        }
    }
}

// Очистка ресурсов
void application_cleanup(Application* app) {
    scene_destroy(&app->scene);
    texture_cleanup_system();
    window_destroy(&app->window);
} 