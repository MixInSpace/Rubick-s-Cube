#ifndef APPLICATION_H
#define APPLICATION_H

#include "window.h"
#include "../scene/scene.h"
#include <stdbool.h>

typedef struct Application {
    Window window;
    Scene scene;
    bool running;
    float lastFrame;
    float deltaTime;
    float targetFPS;
    float frameTimeTarget;
    bool needsRedraw;
} Application;

bool application_init(Application* app);
void application_run(Application* app);
void application_cleanup(Application* app);

#endif /* APPLICATION_H */ 