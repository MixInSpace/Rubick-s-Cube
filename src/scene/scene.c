#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../core/window.h"
#include "../resources/embedded_shaders.h"
#include "scene.h"

// Определение типа элемента по позиции
static bool is_corner_piece(int x, int y, int z) {
    return (x == 0 || x == 2) && (y == 0 || y == 2) && (z == 0 || z == 2);
}

static bool is_edge_piece(int x, int y, int z) {
    int extremeCount = 0;
    if (x == 0 || x == 2) extremeCount++;
    if (y == 0 || y == 2) extremeCount++;
    if (z == 0 || z == 2) extremeCount++;
    return extremeCount == 2;
}

static bool is_center_piece(int x, int y, int z) {
    int extremeCount = 0;
    if (x == 0 || x == 2) extremeCount++;
    if (y == 0 || y == 2) extremeCount++;
    if (z == 0 || z == 2) extremeCount++;
    return extremeCount == 1;
}

// RGB из char
static RGBColor get_rgb_from_cube_color(char colorChar) {
    RGBColor rgb;
    
    switch (colorChar) {
        case CUBE_COLOR_WHITE:
            rgb.r = 1.0f; rgb.g = 1.0f; rgb.b = 1.0f;
            break;
        case CUBE_COLOR_YELLOW:
            rgb.r = 1.0f; rgb.g = 1.0f; rgb.b = 0.0f;
            break;
        case CUBE_COLOR_RED:
            rgb.r = 1.0f; rgb.g = 0.0f; rgb.b = 0.0f;
            break;
        case CUBE_COLOR_ORANGE:
            rgb.r = 1.0f; rgb.g = 0.5f; rgb.b = 0.0f;
            break;
        case CUBE_COLOR_BLUE:
            rgb.r = 0.0f; rgb.g = 0.0f; rgb.b = 1.0f;
            break;
        case CUBE_COLOR_GREEN:
            rgb.r = 0.0f; rgb.g = 0.8f; rgb.b = 0.0f;
            break;
        default:
            // Для неизвестных цветов серый
            rgb.r = 0.5f; rgb.g = 0.5f; rgb.b = 0.5f;
    }
    
    return rgb;
}

// char из RGB
static char rgb_to_char(RGBColor color) {
    const float tolerance = 0.1f;
    
    if (fabs(color.r - 1.0f) < tolerance && fabs(color.g - 1.0f) < tolerance && fabs(color.b - 1.0f) < tolerance) {
        return 'W';
    }
    else if (fabs(color.r - 1.0f) < tolerance && fabs(color.g - 1.0f) < tolerance && fabs(color.b - 0.0f) < tolerance) {
        return 'Y';
    }
    else if (fabs(color.r - 1.0f) < tolerance && fabs(color.g - 0.0f) < tolerance && fabs(color.b - 0.0f) < tolerance) {
        return 'R';
    }
    else if (fabs(color.r - 1.0f) < tolerance && fabs(color.g - 0.5f) < tolerance && fabs(color.b - 0.0f) < tolerance) {
        return 'O';
    }
    else if (fabs(color.r - 0.0f) < tolerance && fabs(color.g - 0.0f) < tolerance && fabs(color.b - 1.0f) < tolerance) {
        return 'B';
    }
    else if (fabs(color.r - 0.0f) < tolerance && fabs(color.g - 0.8f) < tolerance && fabs(color.b - 0.0f) < tolerance) {
        return 'G';
    }
    else {
        return '?';
    }
}

static void create_textured_rubiks_cube_mesh(Scene* scene) {
    // Размер кажого куба
    float size = 0.3f; 
    // Промежуток между кубами
    float gap = 0.02f;
    
    // Размер каждого куба + промежуток между ними
    float step = size + gap;
    
    // НАчальный оффсет, чтобы центровать куб
    float offset = -step;
    
    int index = 0;
    
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                // Determine which faces should be visible based on position
                unsigned int visibleFaces = 0;
                
                // Записываем в виде битовой маски
                // Передняя грань видна для z=2
                if (z == 2) visibleFaces |= FACE_FRONT;
                
                // Задняя для z = 0
                if (z == 0) visibleFaces |= FACE_BACK;
                
                if (y == 2) visibleFaces |= FACE_TOP;
                
                if (y == 0) visibleFaces |= FACE_BOTTOM;
                
                if (x == 2) visibleFaces |= FACE_RIGHT;
                
                if (x == 0) visibleFaces |= FACE_LEFT;
                
                // создаем меш куба с такими текстурами
                scene->cubes[index] = create_custom_colored_cube(visibleFaces, scene->cubeColors, x, y, z);
                
                // Если это центральный кубик, он получает текстуру центрального кубика
                if (is_center_piece(x, y, z)) {
                    scene->cubes[index].textureType = TEXTURE_STICKER_CENTER;
                } else {
                    scene->cubes[index].textureType = TEXTURE_STICKER;
                }
                
                // Сохраняем позиции кубов
                scene->positions[index][0] = offset + x * step; // X position
                scene->positions[index][1] = offset + y * step; // Y position
                scene->positions[index][2] = offset + z * step; // Z position
                
                index++;
            }
        }
    }
}

// Инициализация кубика рубика
bool scene_init_rubiks(Scene* scene) {
    scene->numCubes = 27; // 3x3x3
    
    // Инициализация параметров вращения
    scene->isRotating = false;
    scene->rotationAngle = 0.0f;
    scene->rotationTarget = 0.0f;
    scene->rotatingLayer = 0;
    scene->rotationAxis = 'y';
    scene->speedMultiplier = 1.0f; // Стандартная скороость по умолчанию
    scene->colorMode = false;
    
    // Инициализация параметров последовательности ходов
    scene_init_move_queue(scene);
    

    // Инициализация куба стандартными цветами
    // White (U)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_TOP][i] = (RGBColor){1.0f, 1.0f, 1.0f};
    }
    // Red (F)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_FRONT][i] = (RGBColor){1.0f, 0.0f, 0.0f};
    }
    // Blue (R)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_RIGHT][i] = (RGBColor){0.0f, 0.0f, 1.0f};
    }
    // Orange (B)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_BACK][i] = (RGBColor){1.0f, 0.5f, 0.0f};
    }
    // Green (L)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_LEFT][i] = (RGBColor){0.0f, 0.8f, 0.0f};
    }
    // Yellow (D)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_BOTTOM][i] = (RGBColor){1.0f, 1.0f, 0.0f};
    }

    // Инициализируем шейдеры 
    if (!shader_init_from_source(&scene->shader, TEXTURED_VERTEX_SHADER, TEXTURED_FRAGMENT_SHADER)) {
        fprintf(stderr, "Failed to initialize shader\n");
        return false;
    }

    // Выделяем память на 27 кубов
    scene->cubes = (Mesh*)malloc(scene->numCubes * sizeof(Mesh));
    if (!scene->cubes) {
        fprintf(stderr, "Failed to allocate memory for Rubik's cube meshes\n");
    }

    // Создаем 3х3 структуру кубиков
    create_textured_rubiks_cube_mesh(scene);
    
    // Включение anti-aliasing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    
    return true;
}

// Обновление поворота в фрейме
static void update_rotation(Scene* scene, float deltaTime) {
    if (!scene->isRotating) {
        return;
    }

    float baseRotationSpeed = 270.0f;
    float rotationSpeed = baseRotationSpeed * scene->speedMultiplier;
    
    scene->rotationAngle += rotationSpeed * deltaTime;
    
    // Если вращение завершено
    if (scene->rotationAngle >= scene->rotationTarget) {
        scene->isRotating = false;
        scene->rotationAngle = 0.0f;
        
        // Применяем вращение к цветам на кубе
        for (int i = 0; i < scene->rotationRepetitions; i++) {
            rotate_face_colors(scene->cubeColors, scene->rotatingFace, scene->rotationDirection);

            // Пересобираем куб
            for (int i = 0; i < scene->numCubes; i++) {
                mesh_destroy(&scene->cubes[i]);
            }

            create_textured_rubiks_cube_mesh(scene);(scene, scene->rotatingFace, scene->rotationDirection);
        }
    }
}

void scene_update(Scene* scene, Window* window, float deltaTime) {
    // Если куб вращается обновляем угол вращения
    update_rotation(scene, deltaTime);
    
    // Если сейчас не вращается и не в browse моде, обновляем последовательность ходов в очереди
    if (!scene->isRotating && !scene->browseMode) {
        scene_process_move_queue(scene);
    }

    // Если в режжиме раскраски обновляем движения камеры
    if (scene->colorMode) {
        camera_update_movement(window_get_camera(window), deltaTime);
    }
}

// Функция которая перерисовывает сцену
void scene_render(Scene* scene, Window* window) {
    // Очищаем экран
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Темный фон
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Используем шейдер
    shader_use(&scene->shader);
    
    Camera* camera = window_get_camera(window);
    
    // Получаем матрицы вида и проекции из камеры
    Mat4 view = camera_get_view_matrix(camera);
    float aspectRatio = (float)window->width / (float)window->height;
    Mat4 projection = camera_get_projection_matrix(camera, aspectRatio);
    
    // Устанавливаем эти матрицы в шейдеры
    shader_set_mat4(&scene->shader, "view", view);
    shader_set_mat4(&scene->shader, "projection", projection);
    shader_set_int(&scene->shader, "textureMap", 0);
    shader_set_bool(&scene->shader, "useTexture", true);
    
    // Рендерим Куб
    for (int i = 0; i < scene->numCubes; i++) {
        // Обходим все x, y, z
        int x = i % 3;              // 0, 1, 2, 0, 1, 2, ...
        int y = (i / 3) % 3;        // 0, 0, 0, 1, 1, 1, ...
        int z = i / 9;              // 0, 0, 0, 0, 0, 0, ...
        
        // Проверка на является ли этот куб частью слоя который сейчас вращается
        bool isRotatingCube = false;
        if (scene->isRotating) {
            if (scene->rotationAxis == 'x') {
                isRotatingCube = (z == scene->rotatingLayer);
            } else if (scene->rotationAxis == 'y') {
                isRotatingCube = (y == scene->rotatingLayer);
            } else if (scene->rotationAxis == 'z') {
                isRotatingCube = (x == scene->rotatingLayer);
            }
        }
        
        // Создаем матрицу преобразований для этого куба
        Mat4 model = mat4_identity();
        
        // Если этот куб сейчас вращается, преобразуем матрцу, учитывая смещение и вращение куба относительно центра
        if (isRotatingCube) {
            // Угол вращения в радианах
            float angleRad = scene->rotationAngle * (float)scene->rotationDirection * -(float)M_PI / 180.0f;
            
            // Применяем вращение на основе оси вразения
            if (scene->rotationAxis == 'x') {
                Mat4 rotationMatrix = mat4_rotation_x(angleRad);
                
                // Создаем вектор на основе текущего, поворачиваем и сдвигаем (вращение вокруг центра)
                Vec3 pos = {
                    scene->positions[i][0],
                    scene->positions[i][1],
                    scene->positions[i][2]
                };
                
                Vec3 rotated;
                rotated.x = pos.x;
                rotated.y = pos.y * cosf(angleRad) - pos.z * sinf(angleRad);
                rotated.z = pos.y * sinf(angleRad) + pos.z * cosf(angleRad);
                
                // Создаем матрицу на основе новых координат
                model = mat4_translation(rotated.x, rotated.y, rotated.z);
                
                // Применяем то же вращение к углу поворота самого куба
                model = mat4_multiply(model, rotationMatrix);
                
            } else if (scene->rotationAxis == 'y') {
                Mat4 rotationMatrix = mat4_rotation_y(angleRad);
                
                Vec3 pos = {
                    scene->positions[i][0],
                    scene->positions[i][1],
                    scene->positions[i][2]
                };
                
                Vec3 rotated;
                rotated.x = pos.x * cosf(angleRad) + pos.z * sinf(angleRad);
                rotated.y = pos.y;
                rotated.z = -pos.x * sinf(angleRad) + pos.z * cosf(angleRad);
                
                model = mat4_translation(rotated.x, rotated.y, rotated.z);
                
                model = mat4_multiply(model, rotationMatrix);
                
            } else { // 'z'
                Mat4 rotationMatrix = mat4_rotation_z(angleRad);
                
                Vec3 pos = {
                    scene->positions[i][0],
                    scene->positions[i][1],
                    scene->positions[i][2]
                };
                
                Vec3 rotated;
                rotated.x = pos.x * cosf(angleRad) - pos.y * sinf(angleRad);
                rotated.y = pos.x * sinf(angleRad) + pos.y * cosf(angleRad);
                rotated.z = pos.z;
                
                model = mat4_translation(rotated.x, rotated.y, rotated.z);
                
                model = mat4_multiply(model, rotationMatrix);
            }
        } 
        // Если текущий куб не вращается, просто ставим текущую позицию в model
        else {
            model = mat4_translation(
                scene->positions[i][0],
                scene->positions[i][1],
                scene->positions[i][2]
            );
        }
        
        // Уменьшаем каждый куб
        model = mat4_scale_vec3(model, (Vec3){0.3f, 0.3f, 0.3f});
        
        // Подставляем координаты и вращение куба в шейдер
        shader_set_mat4(&scene->shader, "model", model);
        
        // Рисуем этот куб на экран
        mesh_draw(&scene->cubes[i]);
    }
}

void scene_destroy(Scene* scene) {
    // Освобождаем ресурсы
    shader_destroy(&scene->shader);
    

    // Освобождаем последовательность ходов
    scene_destroy_move_queue(scene);
    
    // Освобождаем каждый куб из грида
    for (int i = 0; i < scene->numCubes; i++) {
        mesh_destroy(&scene->cubes[i]);
    }
    
    free(scene->cubes);
    scene->cubes = NULL;
    scene->numCubes = 0;
}

// Задать позицию кубика рубика из строчки
bool scene_set_cube_state_from_string(Scene* scene, const char* state) {
    if (!scene || !state) {
        return false;
    }
    
    // Проверка на длину
    size_t len = strlen(state);
    if (len != 54) {
        fprintf(stderr, "Invalid cube state string length: %zu (expected 54)\n", len);
        return false;
    }
    
    fprintf(stderr, "Setting cube state from string: %s\n", state);
    
    const int stringToFaceMap[6] = {
        FACE_IDX_TOP,
        FACE_IDX_FRONT,
        FACE_IDX_RIGHT,
        FACE_IDX_BACK,
        FACE_IDX_LEFT,
        FACE_IDX_BOTTOM,
    };
    
    // Убираем все старые кубики
    for (int i = 0; i < scene->numCubes; i++) {
        mesh_destroy(&scene->cubes[i]);
    }
    
    for (int stringFace = 0; stringFace < 6; stringFace++) {
        int faceIndex = stringToFaceMap[stringFace];
        
        for (int pos = 0; pos < 9; pos++) {
            char colorChar = state[stringFace * 9 + pos];
            
            // to_uppercase
            colorChar = (colorChar >= 'a' && colorChar <= 'z') ? 
                        (char)(colorChar - 'a' + 'A') : colorChar;
            
            scene->cubeColors[faceIndex][pos] = get_rgb_from_cube_color(colorChar);
        }
    }
    
    create_textured_rubiks_cube_mesh(scene);
    
    return true;
}

// Получить позицию кубика в срочку
char* scene_get_cube_state_as_string(RGBColor (*cubeColors)[9]) {
    if (!cubeColors) {
        return NULL;
    }
    
    char* state = (char*)malloc(55 * sizeof(char));
    if (!state) {
        return NULL;
    }
    

    const int faceToStringMap[6] = {
        FACE_IDX_TOP,
        FACE_IDX_FRONT,   
        FACE_IDX_RIGHT,   
        FACE_IDX_BACK,         
        FACE_IDX_LEFT,
        FACE_IDX_BOTTOM
    };
    
    int stringIndex = 0;
    for (int stringFace = 0; stringFace < 6; stringFace++) {
        int faceIndex = faceToStringMap[stringFace];
        
        for (int pos = 0; pos < 9; pos++) {
            RGBColor color = cubeColors[faceIndex][pos];
            state[stringIndex] = rgb_to_char(color);
            stringIndex++;
        }
    }
    
    state[54] = '\0';
    
    return state;
}

RGBColor* scene_get_cube_colors(Scene* scene) {
    return (RGBColor*)scene->cubeColors;
}

/* Операции с вращением */
bool scene_is_rotating(Scene* scene) {
    return scene->isRotating;
}

// Начать анимацию вращения
void scene_start_rotation(Scene* scene, FaceIndex face, RotationDirection direction, int repetitions) {
    if (scene->isRotating) {
        return; // Не начинать анимацию вращения если одна уже в процессе
    }
    
    scene->isRotating = true;
    scene->rotationAngle = 0.0f;
    scene->rotationTarget = 90.0f * repetitions;
    scene->rotatingFace = face;
    scene->rotationRepetitions = repetitions;
    
    // Получаем слой кубов и направление вращание для каждой комбинации вращения и face
    switch (face) {
        case FACE_IDX_TOP:
            scene->rotationAxis = 'y';
            scene->rotatingLayer = 2;
            scene->rotationDirection = direction;
            break;
        case FACE_IDX_BOTTOM:
            scene->rotationAxis = 'y';
            scene->rotatingLayer = 0;
            scene->rotationDirection = -direction;
            break;
        case FACE_IDX_RIGHT:
            scene->rotationAxis = 'x';
            scene->rotatingLayer = 2;
            scene->rotationDirection = direction; 
            break;
        case FACE_IDX_LEFT:
            scene->rotationAxis = 'x';
            scene->rotatingLayer = 0; 
            scene->rotationDirection = -direction;
            break;
        case FACE_IDX_FRONT:
            scene->rotationAxis = 'z';
            scene->rotatingLayer = 2; 
            scene->rotationDirection = direction; 
            break;
        case FACE_IDX_BACK:
            scene->rotationAxis = 'z';
            scene->rotatingLayer = 0;
            scene->rotationDirection = -direction; 
            break;
    }
}

// Повернуть цвета на face в направлении direction и соседние элементы к ним
void rotate_face_colors(RGBColor (*cubeColors)[9], FaceIndex face, RotationDirection direction) {
    RGBColor tempColors[6][9];
    for (int f = 0; f < 6; f++) {
        for (int p = 0; p < 9; p++) {
            tempColors[f][p] = cubeColors[f][p];
        }
    }

    if (face == FACE_IDX_TOP || face == FACE_IDX_BACK || face == FACE_IDX_RIGHT) direction = -direction;

    // Поворот цветов на лицевой стороне
    if (direction == ROTATE_CLOCKWISE) {  
        cubeColors[face][0] = tempColors[face][6];
        cubeColors[face][2] = tempColors[face][0];
        cubeColors[face][8] = tempColors[face][2];
        cubeColors[face][6] = tempColors[face][8];
        
        cubeColors[face][1] = tempColors[face][3];
        cubeColors[face][5] = tempColors[face][1];
        cubeColors[face][7] = tempColors[face][5];
        cubeColors[face][3] = tempColors[face][7];
    } else {       
        cubeColors[face][0] = tempColors[face][2];
        cubeColors[face][2] = tempColors[face][8];
        cubeColors[face][8] = tempColors[face][6];
        cubeColors[face][6] = tempColors[face][0];
        
        cubeColors[face][1] = tempColors[face][5];
        cubeColors[face][5] = tempColors[face][7];
        cubeColors[face][7] = tempColors[face][3];
        cubeColors[face][3] = tempColors[face][1];
    }
    
    if (face == FACE_IDX_TOP || face == FACE_IDX_BOTTOM || face == FACE_IDX_LEFT || face == FACE_IDX_RIGHT) direction = -direction;

    switch (face) {
        case FACE_IDX_TOP:
            if (direction == ROTATE_CLOCKWISE) {
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_RIGHT][2-i] = tempColors[FACE_IDX_BACK][i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_BACK][2-i] = tempColors[FACE_IDX_LEFT][i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_LEFT][2-i] = tempColors[FACE_IDX_FRONT][i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_FRONT][2-i] = tempColors[FACE_IDX_RIGHT][i];
                }
            } else {
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_LEFT][2-i] = tempColors[FACE_IDX_BACK][i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_BACK][2-i] = tempColors[FACE_IDX_RIGHT][i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_RIGHT][2-i] = tempColors[FACE_IDX_FRONT][i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_FRONT][2-i] = tempColors[FACE_IDX_LEFT][i];
                }
            }
            break;
        case FACE_IDX_BOTTOM:
            if (direction == ROTATE_CLOCKWISE) {
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_LEFT][2-i+6] = tempColors[FACE_IDX_BACK][6+i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_BACK][2-i+6] = tempColors[FACE_IDX_RIGHT][6+i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_RIGHT][2-i+6] = tempColors[FACE_IDX_FRONT][6+i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_FRONT][2-i+6] = tempColors[FACE_IDX_LEFT][6+i];
                }
            } else {
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_RIGHT][2-i+6] = tempColors[FACE_IDX_BACK][6+i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_BACK][2-i+6] = tempColors[FACE_IDX_LEFT][6+i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_LEFT][2-i+6] = tempColors[FACE_IDX_FRONT][6+i];
                }
                for (int i = 0; i < 3; i++) {
                    cubeColors[FACE_IDX_FRONT][2-i+6] = tempColors[FACE_IDX_RIGHT][6+i];
                }
            }
            break;
        case FACE_IDX_FRONT:
            if (direction == ROTATE_CLOCKWISE) {
                cubeColors[FACE_IDX_RIGHT][8] = tempColors[FACE_IDX_TOP][2];
                cubeColors[FACE_IDX_RIGHT][5] = tempColors[FACE_IDX_TOP][1];
                cubeColors[FACE_IDX_RIGHT][2] = tempColors[FACE_IDX_TOP][0];
                
                cubeColors[FACE_IDX_BOTTOM][6] = tempColors[FACE_IDX_RIGHT][8];
                cubeColors[FACE_IDX_BOTTOM][7] = tempColors[FACE_IDX_RIGHT][5];
                cubeColors[FACE_IDX_BOTTOM][8] = tempColors[FACE_IDX_RIGHT][2];
                
                cubeColors[FACE_IDX_LEFT][6] = tempColors[FACE_IDX_BOTTOM][8];
                cubeColors[FACE_IDX_LEFT][3] = tempColors[FACE_IDX_BOTTOM][7];
                cubeColors[FACE_IDX_LEFT][0] = tempColors[FACE_IDX_BOTTOM][6];
                
                cubeColors[FACE_IDX_TOP][2] = tempColors[FACE_IDX_LEFT][0];
                cubeColors[FACE_IDX_TOP][1] = tempColors[FACE_IDX_LEFT][3];
                cubeColors[FACE_IDX_TOP][0] = tempColors[FACE_IDX_LEFT][6];
            } else {
                cubeColors[FACE_IDX_LEFT][0] = tempColors[FACE_IDX_TOP][2];
                cubeColors[FACE_IDX_LEFT][3] = tempColors[FACE_IDX_TOP][1];
                cubeColors[FACE_IDX_LEFT][6] = tempColors[FACE_IDX_TOP][0];
                
                cubeColors[FACE_IDX_BOTTOM][6] = tempColors[FACE_IDX_LEFT][0];
                cubeColors[FACE_IDX_BOTTOM][7] = tempColors[FACE_IDX_LEFT][3];
                cubeColors[FACE_IDX_BOTTOM][8] = tempColors[FACE_IDX_LEFT][6];
                
                cubeColors[FACE_IDX_RIGHT][2] = tempColors[FACE_IDX_BOTTOM][8];
                cubeColors[FACE_IDX_RIGHT][5] = tempColors[FACE_IDX_BOTTOM][7];
                cubeColors[FACE_IDX_RIGHT][8] = tempColors[FACE_IDX_BOTTOM][6];
                
                cubeColors[FACE_IDX_TOP][2] = tempColors[FACE_IDX_RIGHT][8];
                cubeColors[FACE_IDX_TOP][1] = tempColors[FACE_IDX_RIGHT][5];
                cubeColors[FACE_IDX_TOP][0] = tempColors[FACE_IDX_RIGHT][2];
            }
            break;
        case FACE_IDX_BACK:
            if (direction == ROTATE_CLOCKWISE) {                
                cubeColors[FACE_IDX_LEFT][2] = tempColors[FACE_IDX_TOP][8];
                cubeColors[FACE_IDX_LEFT][5] = tempColors[FACE_IDX_TOP][7];
                cubeColors[FACE_IDX_LEFT][8] = tempColors[FACE_IDX_TOP][6];

                cubeColors[FACE_IDX_BOTTOM][0] = tempColors[FACE_IDX_LEFT][2];
                cubeColors[FACE_IDX_BOTTOM][1] = tempColors[FACE_IDX_LEFT][5];
                cubeColors[FACE_IDX_BOTTOM][2] = tempColors[FACE_IDX_LEFT][8];
                
                cubeColors[FACE_IDX_RIGHT][6] = tempColors[FACE_IDX_BOTTOM][0];
                cubeColors[FACE_IDX_RIGHT][3] = tempColors[FACE_IDX_BOTTOM][1];
                cubeColors[FACE_IDX_RIGHT][0] = tempColors[FACE_IDX_BOTTOM][2];
                
                cubeColors[FACE_IDX_TOP][6] = tempColors[FACE_IDX_RIGHT][0];
                cubeColors[FACE_IDX_TOP][7] = tempColors[FACE_IDX_RIGHT][3];
                cubeColors[FACE_IDX_TOP][8] = tempColors[FACE_IDX_RIGHT][6];
            } else {
                cubeColors[FACE_IDX_RIGHT][0] = tempColors[FACE_IDX_TOP][6];
                cubeColors[FACE_IDX_RIGHT][3] = tempColors[FACE_IDX_TOP][7];
                cubeColors[FACE_IDX_RIGHT][6] = tempColors[FACE_IDX_TOP][8];
                
                cubeColors[FACE_IDX_BOTTOM][2] = tempColors[FACE_IDX_RIGHT][0];
                cubeColors[FACE_IDX_BOTTOM][1] = tempColors[FACE_IDX_RIGHT][3];
                cubeColors[FACE_IDX_BOTTOM][0] = tempColors[FACE_IDX_RIGHT][6];
                
                cubeColors[FACE_IDX_LEFT][8] = tempColors[FACE_IDX_BOTTOM][2];
                cubeColors[FACE_IDX_LEFT][5] = tempColors[FACE_IDX_BOTTOM][1];
                cubeColors[FACE_IDX_LEFT][2] = tempColors[FACE_IDX_BOTTOM][0];
            
                cubeColors[FACE_IDX_TOP][8] = tempColors[FACE_IDX_LEFT][2];
                cubeColors[FACE_IDX_TOP][7] = tempColors[FACE_IDX_LEFT][5];
                cubeColors[FACE_IDX_TOP][6] = tempColors[FACE_IDX_LEFT][8];
            }
            break;
        case FACE_IDX_LEFT:
            if (direction == ROTATE_CLOCKWISE) {
                cubeColors[FACE_IDX_FRONT][0] = tempColors[FACE_IDX_TOP][6];
                cubeColors[FACE_IDX_FRONT][3] = tempColors[FACE_IDX_TOP][3];
                cubeColors[FACE_IDX_FRONT][6] = tempColors[FACE_IDX_TOP][0];
                
                cubeColors[FACE_IDX_BOTTOM][0] = tempColors[FACE_IDX_FRONT][6];
                cubeColors[FACE_IDX_BOTTOM][3] = tempColors[FACE_IDX_FRONT][3];
                cubeColors[FACE_IDX_BOTTOM][6] = tempColors[FACE_IDX_FRONT][0];
                
                cubeColors[FACE_IDX_BACK][2] = tempColors[FACE_IDX_BOTTOM][0];
                cubeColors[FACE_IDX_BACK][5] = tempColors[FACE_IDX_BOTTOM][3];
                cubeColors[FACE_IDX_BACK][8] = tempColors[FACE_IDX_BOTTOM][6];
                
                cubeColors[FACE_IDX_TOP][0] = tempColors[FACE_IDX_BACK][2];
                cubeColors[FACE_IDX_TOP][3] = tempColors[FACE_IDX_BACK][5];
                cubeColors[FACE_IDX_TOP][6] = tempColors[FACE_IDX_BACK][8];
            } else {
                cubeColors[FACE_IDX_BACK][8] = tempColors[FACE_IDX_TOP][6];
                cubeColors[FACE_IDX_BACK][5] = tempColors[FACE_IDX_TOP][3];
                cubeColors[FACE_IDX_BACK][2] = tempColors[FACE_IDX_TOP][0];
                
                cubeColors[FACE_IDX_BOTTOM][0] = tempColors[FACE_IDX_BACK][2];
                cubeColors[FACE_IDX_BOTTOM][3] = tempColors[FACE_IDX_BACK][5];
                cubeColors[FACE_IDX_BOTTOM][6] = tempColors[FACE_IDX_BACK][8];
                
                cubeColors[FACE_IDX_FRONT][0] = tempColors[FACE_IDX_BOTTOM][6];
                cubeColors[FACE_IDX_FRONT][3] = tempColors[FACE_IDX_BOTTOM][3];
                cubeColors[FACE_IDX_FRONT][6] = tempColors[FACE_IDX_BOTTOM][0];
                
                cubeColors[FACE_IDX_TOP][0] = tempColors[FACE_IDX_FRONT][6];
                cubeColors[FACE_IDX_TOP][3] = tempColors[FACE_IDX_FRONT][3];
                cubeColors[FACE_IDX_TOP][6] = tempColors[FACE_IDX_FRONT][0];
            }
            break;
        case FACE_IDX_RIGHT:
            if (direction == ROTATE_CLOCKWISE) {                
                cubeColors[FACE_IDX_BACK][6] = tempColors[FACE_IDX_TOP][8];
                cubeColors[FACE_IDX_BACK][3] = tempColors[FACE_IDX_TOP][5];
                cubeColors[FACE_IDX_BACK][0] = tempColors[FACE_IDX_TOP][2];
                
                cubeColors[FACE_IDX_BOTTOM][2] = tempColors[FACE_IDX_BACK][0];
                cubeColors[FACE_IDX_BOTTOM][5] = tempColors[FACE_IDX_BACK][3];
                cubeColors[FACE_IDX_BOTTOM][8] = tempColors[FACE_IDX_BACK][6];
                
                cubeColors[FACE_IDX_FRONT][2] = tempColors[FACE_IDX_BOTTOM][8];
                cubeColors[FACE_IDX_FRONT][5] = tempColors[FACE_IDX_BOTTOM][5];
                cubeColors[FACE_IDX_FRONT][8] = tempColors[FACE_IDX_BOTTOM][2];
                
                cubeColors[FACE_IDX_TOP][2] = tempColors[FACE_IDX_FRONT][8];
                cubeColors[FACE_IDX_TOP][5] = tempColors[FACE_IDX_FRONT][5];
                cubeColors[FACE_IDX_TOP][8] = tempColors[FACE_IDX_FRONT][2];

            } else {
                cubeColors[FACE_IDX_FRONT][2] = tempColors[FACE_IDX_TOP][8];
                cubeColors[FACE_IDX_FRONT][5] = tempColors[FACE_IDX_TOP][5];
                cubeColors[FACE_IDX_FRONT][8] = tempColors[FACE_IDX_TOP][2];
                
                cubeColors[FACE_IDX_BOTTOM][2] = tempColors[FACE_IDX_FRONT][8];
                cubeColors[FACE_IDX_BOTTOM][5] = tempColors[FACE_IDX_FRONT][5];
                cubeColors[FACE_IDX_BOTTOM][8] = tempColors[FACE_IDX_FRONT][2];
                
                cubeColors[FACE_IDX_BACK][6] = tempColors[FACE_IDX_BOTTOM][8];
                cubeColors[FACE_IDX_BACK][3] = tempColors[FACE_IDX_BOTTOM][5];
                cubeColors[FACE_IDX_BACK][0] = tempColors[FACE_IDX_BOTTOM][2];
                
                cubeColors[FACE_IDX_TOP][8] = tempColors[FACE_IDX_BACK][6];
                cubeColors[FACE_IDX_TOP][5] = tempColors[FACE_IDX_BACK][3];
                cubeColors[FACE_IDX_TOP][2] = tempColors[FACE_IDX_BACK][0];
            }
            break;
    }
}


// Move sequence management functions

void scene_init_move_queue(Scene* scene) {
    scene->moveQueue = NULL;
    scene->moveQueueSize = 0;
    scene->moveQueueCapacity = 0;
    scene->currentMoveIndex = 0;
    scene->processingSequence = false;
    scene->originalSpeedBeforeSequence = 1.0f;
    
    // Initialize browse mode
    scene->browseMode = false;
    scene->browseIndex = 0;
}

void scene_destroy_move_queue(Scene* scene) {
    if (scene->moveQueue) {
        // Освобождаем каждую строку хода
        for (int i = 0; i < scene->moveQueueSize; i++) {
            if (scene->moveQueue[i]) {
                free(scene->moveQueue[i]);
            }
        }
        free(scene->moveQueue);
        scene->moveQueue = NULL;
    }
    scene->moveQueueSize = 0;
    scene->moveQueueCapacity = 0;
    scene->currentMoveIndex = 0;
    scene->processingSequence = false;
    
    scene->browseMode = false;
    scene->browseIndex = 0;
}

void scene_add_move_to_queue(Scene* scene, const char* move) {
    if (!scene || !move) return;
    
    // Увеличиваем очередь если необходимо 
    if (scene->moveQueueSize >= scene->moveQueueCapacity) {
        int newCapacity = scene->moveQueueCapacity == 0 ? 16 : scene->moveQueueCapacity * 2;
        char** newQueue = (char**)realloc(scene->moveQueue, newCapacity * sizeof(char*));
        if (!newQueue) {
            fprintf(stderr, "Failed to expand move queue\n");
            return;
        }
        scene->moveQueue = newQueue;
        scene->moveQueueCapacity = newCapacity;
    }
    
    // Копируем строчку ходов
    size_t moveLen = strlen(move);
    scene->moveQueue[scene->moveQueueSize] = (char*)malloc((moveLen + 1) * sizeof(char));
    if (!scene->moveQueue[scene->moveQueueSize]) {
        fprintf(stderr, "Failed to allocate memory for move string\n");
        return;
    }
    strcpy(scene->moveQueue[scene->moveQueueSize], move);
    scene->moveQueueSize++;
}

// Функция чтобы извлечь direction и face из строчки хода
static bool parse_move_string(const char* moveStr, FaceIndex* face, RotationDirection* direction, int* repetitions) {
    if (!moveStr || strlen(moveStr) == 0) return false;
    
    *repetitions = 1;
    *direction = ROTATE_CLOCKWISE;
    
    char faceChar = moveStr[0];
    switch (faceChar) {
        case 'U': case 'u': *face = FACE_IDX_TOP; break;
        case 'D': case 'd': *face = FACE_IDX_BOTTOM; break;
        case 'F': case 'f': *face = FACE_IDX_FRONT; break;
        case 'B': case 'b': *face = FACE_IDX_BACK; break;
        case 'R': case 'r': *face = FACE_IDX_RIGHT; break;
        case 'L': case 'l': *face = FACE_IDX_LEFT; break;
        default: return false;
    }
    
    for (size_t i = 1; i < strlen(moveStr); i++) {
        char modifier = moveStr[i];
        if (modifier == '\'' || modifier == '\'') {
            *direction = ROTATE_COUNTERCLOCKWISE;
        } else if (modifier == '2') {
            *repetitions = 2;
        }
    }
    
    return true;
}

// Функция чтобы получить обратный ход ( для возврата в режиме по ходам)
static bool get_inverse_move(const char* moveStr, FaceIndex* face, RotationDirection* direction, int* repetitions) {
    if (!parse_move_string(moveStr, face, direction, repetitions)) {
        return false;
    }
    
    *direction = (*direction == ROTATE_CLOCKWISE) ? ROTATE_COUNTERCLOCKWISE : ROTATE_CLOCKWISE;
    
    return true;
}

// Обрабюотка очереди ходов
void scene_process_move_queue(Scene* scene) {
    if (!scene || !scene->processingSequence || scene->currentMoveIndex >= scene->moveQueueSize) {
        return;
    }
    
    // Если сей1час аниманиця поворота не играет, начинаем поворот следующего 
    if (!scene->isRotating) {
        const char* moveStr = scene->moveQueue[scene->currentMoveIndex];
        FaceIndex face;
        RotationDirection direction;
        int repetitions;
        
        if (parse_move_string(moveStr, &face, &direction, &repetitions)) {
            printf("Executing move %d/%d: %s\n", 
                   scene->currentMoveIndex + 1, scene->moveQueueSize, moveStr);
            
            // Запускаем анимацию
            scene_start_rotation(scene, face, direction, repetitions);
            scene->currentMoveIndex++;
            
            // Если ходы кончились очищаем очередь и сбрасываем скорость
            if (scene->currentMoveIndex >= scene->moveQueueSize) {
                printf("Move sequence completed! Restoring speed to %.1fx\n", scene->originalSpeedBeforeSequence);
                scene_set_speed_multiplier(scene, scene->originalSpeedBeforeSequence);
                scene->processingSequence = false;
                scene->currentMoveIndex = 0;
                scene->moveQueueSize = 0; 
            }
        } else {
            fprintf(stderr, "Invalid move string: %s\n", moveStr);
            scene->currentMoveIndex++;
        }
    }
}

bool scene_is_processing_sequence(Scene* scene) {
    return scene ? scene->processingSequence : false;
}

// Функция чтобы сразу проиграть всю цепочку ходов
void apply_move_sequence(Scene* scene, char** moveSequence) {
    
    if (scene->processingSequence || scene->isRotating) {
        printf("Cannot start new move sequence: one is already in progress\n");
        return;
    }
    
    scene->originalSpeedBeforeSequence = scene->speedMultiplier;
    
    scene_destroy_move_queue(scene);
    scene_init_move_queue(scene);
    
    int moveCount = 0;
    while (moveSequence[moveCount] != NULL) {
        moveCount++;
    }
    
    if (moveCount == 0) {
        printf("Empty move sequence provided\n");
        scene_set_speed_multiplier(scene, scene->originalSpeedBeforeSequence);
        return;
    }
    
    printf("Starting move sequence with %d moves at %.1fx speed: ", moveCount, scene->speedMultiplier);
    for (int i = 0; i < moveCount; i++) {
        scene_add_move_to_queue(scene, moveSequence[i]);
        printf("%s ", moveSequence[i]);
    }
    printf("\n");
    
    scene->processingSequence = true;
    scene->currentMoveIndex = 0;
}


// Browse mode functions

void scene_enter_browse_mode(Scene* scene) {
    if (!scene || scene->moveQueueSize == 0) {
        printf("Cannot enter browse mode: no moves in queue\n");
        return;
    }
    
    scene->processingSequence = false;
    
    scene->browseMode = true;
    scene->browseIndex = 0;
    
    printf("Entered browse mode with %d moves. Use arrow keys to navigate, C to exit.\n", scene->moveQueueSize);
    printf("Ready to apply move [%d/%d]: %s\n", scene->browseIndex + 1, scene->moveQueueSize, 
           scene->moveQueue[scene->browseIndex]);
}

void scene_exit_browse_mode(Scene* scene) {
    if (!scene || !scene->browseMode) {
        return;
    }
    
    printf("Exiting browse mode\n");
    scene->browseMode = false;
    scene->browseIndex = 0;
}

void scene_browse_next(Scene* scene) {
    if (!scene || !scene->browseMode || scene->moveQueueSize == 0) {
        return;
    }
    
    if (scene->browseIndex >= scene->moveQueueSize) {
        printf("Already at the end of sequence\n");
        return;
    }
    
    const char* currentMoveStr = scene->moveQueue[scene->browseIndex];
    FaceIndex face;
    RotationDirection direction;
    int repetitions;
    
    if (parse_move_string(currentMoveStr, &face, &direction, &repetitions)) {
        printf("Applying move [%d/%d]: %s\n", 
               scene->browseIndex + 1, scene->moveQueueSize, currentMoveStr);
        
        scene_start_rotation(scene, face, direction, repetitions);
    }
    
    scene->browseIndex++;
    
    if (scene->browseIndex < scene->moveQueueSize) {
        printf("Now at move [%d/%d]: %s\n", scene->browseIndex + 1, scene->moveQueueSize, 
               scene->moveQueue[scene->browseIndex]);
    } else {
        printf("Reached the end of sequence (all moves applied)\n");
    }
}

void scene_browse_previous(Scene* scene) {
    if (!scene || !scene->browseMode || scene->moveQueueSize == 0) {
        return;
    }
    
    if (scene->browseIndex <= 0) {
        printf("Already at the beginning of sequence - no moves to undo\n");
        return;
    }
    
    scene->browseIndex--;
    
    const char* moveToUndo = scene->moveQueue[scene->browseIndex];
    FaceIndex face;
    RotationDirection direction;
    int repetitions;
    
    if (get_inverse_move(moveToUndo, &face, &direction, &repetitions)) {
        printf("Undoing move [%d/%d]: %s (applying inverse)\n", 
               scene->browseIndex + 1, scene->moveQueueSize, moveToUndo);
        
        scene_start_rotation(scene, face, direction, repetitions);
    }
    
    printf("Now at move [%d/%d]: %s\n", scene->browseIndex + 1, scene->moveQueueSize, 
           scene->moveQueue[scene->browseIndex]);
}

bool scene_is_in_browse_mode(Scene* scene) {
    return scene ? scene->browseMode : false;
}

// Color mode functions

static void move_camera_to_color_face(Window* window, FaceIndex face) {
    printf("Moving camera to face %d\n", face);
    switch (face) {
        case FACE_IDX_TOP:
            camera_move_to(window_get_camera(window), 90.0f, 1.0f, 1.0f);
            break;
        case FACE_IDX_FRONT:
            camera_move_to(window_get_camera(window), 90.0f, 90.0f, 1.0f);
            break;
        case FACE_IDX_RIGHT:
            camera_move_to(window_get_camera(window), 0.0f, 90.0f, 1.0f);
            break;
        case FACE_IDX_BACK:
            camera_move_to(window_get_camera(window), -90.0f, 90.0f, 1.0f);
            break;
        case FACE_IDX_LEFT:
            camera_move_to(window_get_camera(window), -180.0f, 90.0f, 1.0f);
            break;
        case FACE_IDX_BOTTOM:
            camera_move_to(window_get_camera(window), -180.0f, 179.0f, 1.0f);
            break;
    }
}

static int get_cell_index_on_face(FaceIndex face, int cellIndex) {
    int top_cells[] = {6, 7, 8, 3, 4, 5, 0, 1, 2};
    int side_cells[] = {2, 1, 0, 5, 4, 3, 8, 7, 6};
    int bottom_cells[] = {0, 3, 6, 1, 4, 7, 2, 5, 8};
    switch (face) {
        case FACE_IDX_TOP:
            return top_cells[cellIndex]; 
        case FACE_IDX_FRONT:
            return cellIndex;
        case FACE_IDX_RIGHT:
            return side_cells[cellIndex];
        case FACE_IDX_BACK:
            return cellIndex;
        case FACE_IDX_LEFT:
            return side_cells[cellIndex];
        case FACE_IDX_BOTTOM:
            return bottom_cells[cellIndex];
    }
}

bool scene_is_in_color_mode(Scene* scene) {
    return scene ? scene->colorMode : false;
}

void scene_enter_color_mode(Scene* scene, Window* window) {

    scene_set_cube_state_from_string(scene, "NNNNWNNNNNNNNRNNNNNNNNBNNNNNNNNONNNNNNNNGNNNNNNNNYNNNN");

    scene->colorMode = true;
    scene->colorFace = FACE_IDX_TOP;
    scene->cellIndex = 0;

    move_camera_to_color_face(window, scene->colorFace);
}

void scene_next_color_face(Scene* scene, Window* window) {
    if (!scene || !scene->colorMode) {
        return;
    }

    if (window->camera.isMoving) {
        return;
    }

    scene->cellIndex = 0;
    scene->colorFace = (scene->colorFace + 1) % 6;
    move_camera_to_color_face(window, scene->colorFace);
}

void scene_previous_color_face(Scene* scene, Window* window) {
    if (!scene || !scene->colorMode) {
        return;
    }

    if (window->camera.isMoving) {
        return;
    }

    scene->cellIndex = 0;
    scene->colorFace = (scene->colorFace - 1 + 6) % 6;
    move_camera_to_color_face(window, scene->colorFace);
}

void scene_exit_color_mode(Scene* scene, Window* window) {
    if (!scene || !scene->colorMode) {
        return;
    }
    scene->colorMode = false;
}

void scene_set_color_for_current_cell(Scene* scene, Window* window, char color) {
    if (!scene || !scene->colorMode) {
        return;
    }

    int index = get_cell_index_on_face(scene->colorFace, scene->cellIndex);
    scene->cubeColors[scene->colorFace][index] = get_rgb_from_cube_color(color);
    scene->cellIndex==3? scene->cellIndex=5 : scene->cellIndex == 8? scene->cellIndex=0 : scene->cellIndex++;
    for (int i = 0; i < scene->numCubes; i++) {
        mesh_destroy(&scene->cubes[i]);
    }
    create_textured_rubiks_cube_mesh(scene);
}

// Animation speed control functions

void scene_set_speed_multiplier(Scene* scene, float multiplier) {
    if (scene && multiplier > 0.0f) {
        scene->speedMultiplier = multiplier;
        printf("Animation speed set to %.1fx\n", multiplier);
    }
}

float scene_get_speed_multiplier(Scene* scene) {
    return scene ? scene->speedMultiplier : 1.0f;
}
