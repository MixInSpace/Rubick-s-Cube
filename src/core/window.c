#define _USE_MATH_DEFINES

#include "window.h"
#include "application.h"
#include "../scene/scene.h"
#include "../solver/cube_solver.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>

static void handle_move_sequence(struct Application* app, char** moveSequence, bool browseMode, float speed);
static char** generate_random_move_sequence(int length);
static char* generate_random_move(char excludeFace);
static void framebuffer_size_callback(GLFWwindow* handle, int width, int height);

// Функция для изменения размеров окна
static void framebuffer_size_callback(GLFWwindow* handle, int width, int height) {
    glViewport(0, 0, width, height);
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    window->width = width;
    window->height = height;
}

static char** generate_random_move_sequence(int length) {
    char** moveSequence = malloc((length + 1) * sizeof(char*));
    char lastFace = '\0';
    
    for (int i = 0; i < length; i++) {
        moveSequence[i] = generate_random_move(lastFace);
        lastFace = moveSequence[i][0];
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

// Функция для обработки последовательности ходов
static void handle_move_sequence(struct Application* app, char** moveSequence, bool browseMode, float speed) {
    // Если загружена в режиме по ходам
    if (browseMode) {
        // Удалить существующую очередь ходов и инициализировать новую
        scene_destroy_move_queue(&app->scene);
        scene_init_move_queue(&app->scene);
        
        // Добавить ходы в очередь
        int i = 0;
        while (moveSequence[i] != NULL) {
            scene_add_move_to_queue(&app->scene, moveSequence[i]);
            i++;
        }
        
        scene_enter_browse_mode(&app->scene);
    } else {
        // Выполнить последовательность напрямую с указанной скоростью
        scene_set_speed_multiplier(&app->scene, speed);
        apply_move_sequence(&app->scene, moveSequence);
    }
}

// Функция для обработки нажатий клавиш
static void key_callback(GLFWwindow* handle, int key, int scancode, int action, int mods) {
    if (action != GLFW_PRESS) return;
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    
    // Получить ссылку на камеру
    Camera* camera = &window->camera;
    
    struct Application* app = window->app;
    if (app && !scene_is_rotating(&app->scene)) {
        // Обрабока нажатий в специальных режимах
        if (scene_is_in_browse_mode(&app->scene)) {
            // Навигация в режиме просмотра
            if (key == GLFW_KEY_RIGHT) {
                scene_browse_next(&app->scene);
            } 
            else if (key == GLFW_KEY_LEFT) {
                scene_browse_previous(&app->scene);
            } 
            else if (key == GLFW_KEY_C) {
                scene_exit_browse_mode(&app->scene);
            }
            return; // Не обрабатывать другие клавиши в режиме просмотра
        } else if (scene_is_in_color_mode(&app->scene)) {
            // Навигация в режиме цвета
            if (key == GLFW_KEY_RIGHT) {
                scene_next_color_face(&app->scene, window);
            } 
            else if (key == GLFW_KEY_LEFT) {
                scene_previous_color_face(&app->scene, window);
            } 
            // Установить цвет для текущей ячейки
            else if (key == GLFW_KEY_1) {
                scene_set_color_for_current_cell(&app->scene, window, 'W');
            }
            else if (key == GLFW_KEY_2) {
                scene_set_color_for_current_cell(&app->scene, window, 'R');
            }
            else if (key == GLFW_KEY_3) {
                scene_set_color_for_current_cell(&app->scene, window, 'B');
            }
            else if (key == GLFW_KEY_4) {
                scene_set_color_for_current_cell(&app->scene, window, 'O');
            }
            else if (key == GLFW_KEY_5) {
                scene_set_color_for_current_cell(&app->scene, window, 'G');
            }
            else if (key == GLFW_KEY_6) {
                scene_set_color_for_current_cell(&app->scene, window, 'Y');
            }
            // Выйти из режима раскраски можно только если куб валидный (решается)
            else if (key == GLFW_KEY_C) {
                char* cubeString = scene_get_cube_state_as_string(app->scene.cubeColors);
                
                bool incomplete = false;

                for (int i = 0; cubeString[i] != '\0'; i++) {
                    char c = cubeString[i];
                    if (c == '?'){
                        incomplete = true;
                        break;
                    }
                }

                if (incomplete) {
                    printf("Coloring not complete\n");
                    
                    const char* solidColors = "WWWWWWWWWRRRRRRRRRBBBBBBBBBOOOOOOOOOGGGGGGGGGYYYYYYYYY";
                    scene_set_cube_state_from_string(&app->scene, solidColors);
                    scene_exit_color_mode(&app->scene, window);
                } else {
                    bool isSolved = false;
                    char** moveSequence = cube_solver_solve(&app->scene, &isSolved);
                    if (isSolved) {
                        scene_exit_color_mode(&app->scene, window);
                    }
                    else {
                        printf("Not valid cube state\n");
                    }
                }
            }
            return; // Не обрабатывать другие клавиши в режиме цвета
        }
        
        // Обрабока нажатий в режиме вращения
        // Если shift зажат, то вращение против часовой стрелки, иначе по часовой стрелке
        int direction = (mods & GLFW_MOD_SHIFT) ? -1 : 1; 
        
        // Верхняя грань (Up - U)
        if (key == GLFW_KEY_U) {
            scene_start_rotation(&app->scene, FACE_IDX_TOP, direction, 1);
            printf("Rotating top face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }
        // Нижняя грань (Down - D)
        if (key == GLFW_KEY_D) {
            scene_start_rotation(&app->scene, FACE_IDX_BOTTOM, direction, 1);
            printf("Rotating bottom face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }
        // Передняя грань (Front - F)
        if (key == GLFW_KEY_F) {
            scene_start_rotation(&app->scene, FACE_IDX_FRONT, direction, 1);
            printf("Rotating front face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }
        // Задняя грань (Back - B)
        if (key == GLFW_KEY_B) {
            scene_start_rotation(&app->scene, FACE_IDX_BACK, direction, 1);
            printf("Rotating back face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }
        // Левая грань (Left - L)
        if (key == GLFW_KEY_L) {
            scene_start_rotation(&app->scene, FACE_IDX_LEFT, direction, 1);
            printf("Rotating left face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }        
        // Правая грань (Right - R)
        if (key == GLFW_KEY_R) {
            scene_start_rotation(&app->scene, FACE_IDX_RIGHT, direction, 1);
            printf("Rotating right face %s\n", 
                   direction == 1 ? "clockwise" : "counter-clockwise");
        }


        // Сброс куба в исходное состояние
        if (key == GLFW_KEY_O) {
            const char* solidColors = "WWWWWWWWWRRRRRRRRRBBBBBBBBBOOOOOOOOOGGGGGGGGGYYYYYYYYY";
            scene_set_cube_state_from_string(&app->scene, solidColors);
        }


        // Если зажат shift, то последовательности воспроизводятся в режиме просмотра по ходам
        // Случайная последовательность ходов
        if (key == GLFW_KEY_M) {
            char** moveSequence = generate_random_move_sequence(30);
            handle_move_sequence(app, moveSequence, mods & GLFW_MOD_SHIFT, 7.0f);
        }

        // Решение куба
        if (key == GLFW_KEY_S) {
            bool isSolved = false;
            char** moveSequence = cube_solver_solve(&app->scene, &isSolved);
            
            if (isSolved) {
                handle_move_sequence(app, moveSequence, mods & GLFW_MOD_SHIFT, 3.0f);
            }
            else {
                printf("Not solved\n");
                
                handle_move_sequence(app, moveSequence, mods & GLFW_MOD_SHIFT, 3.0f);
            }
        }

        // Примеры последовательностей ходов (Узоры)
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
        if (key == GLFW_KEY_3) { 
            //U2 F2 R2 U' L2 D B R' B R' B R' D' L2 U'
            char* moveSequence[] = {"B", "U", "D", "F", "D'", "B", "U2", "R'", "L'", "F", "D", "U'", "L'", "R", "B", "D'", "L'", "U2", "R", "B2", "U", "F'", "D'", "R2", "F'", "L'", "D2", "L", "R", "U'", NULL};            
            handle_move_sequence(app, moveSequence, mods & GLFW_MOD_SHIFT, 3.0f);
        }
        // Режим раскраски
        if (key == GLFW_KEY_C) {
            scene_enter_color_mode(&app->scene, window);
        }
    }
}

// Обрабока нажатий мыши
static void mouse_button_callback(GLFWwindow* handle, int button, int action, int mods) {
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    struct Application* app = window->app;

    // Отключить вращение мышью в режиме раскраски 
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS && !scene_is_in_color_mode(&app->scene)) {
            camera_start_drag(&window->camera, window->mouseX, window->mouseY);
        } else if (action == GLFW_RELEASE) {
            camera_end_drag(&window->camera);
        }
    }
}

// Обрабока движения мыши
static void cursor_pos_callback(GLFWwindow* handle, double xpos, double ypos) {
    Window* window = (Window*)glfwGetWindowUserPointer(handle);

    // Отключить движения мышью в режиме раскраски
    if (!scene_is_in_color_mode(&window->app->scene)){
        // Обновление текущей позиции мыши
        window->mouseX = xpos;
        window->mouseY = ypos;
        
        // Обрабока движения мыши для камеры
        camera_process_mouse_movement(&window->camera, xpos, ypos);
    }
}

// Обработка колесика мыши
static void scroll_callback(GLFWwindow* handle, double xoffset, double yoffset) {
    Window* window = (Window*)glfwGetWindowUserPointer(handle);
    struct Application* app = window->app;
    // Process scroll for camera zoom
    camera_process_scroll(&window->camera, yoffset);
}

// Инициализация окна 
bool window_init(Window* window, int width, int height, const char* title) {
    if (!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return false;
    }

    // Request OpenGL 3.3 core profile
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Включить MSAA (Multi-Sample Anti-Aliasing) 4x
    glfwWindowHint(GLFW_SAMPLES, 4);

    window->handle = glfwCreateWindow(width, height, title, NULL, NULL);
    window->width = width;
    window->height = height;
    window->title = title;
    window->shouldClose = false;
    window->mouseX = 0.0;
    window->mouseY = 0.0;
    
    // Инициализация камеры
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
    
    // VSync
    glfwSwapInterval(1);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return false;
    }

    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));

    glViewport(0, 0, width, height);
    
    // Включить MSAA
    glEnable(GL_MULTISAMPLE);
    
    return true;
}

// Функция которая меняет текущий буфер окна со следующим обновляя viewport
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

Camera* window_get_camera(Window* window) {
    return &window->camera;
}