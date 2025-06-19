#ifndef SCENE_H
#define SCENE_H

#include <stdbool.h>
#include "../renderer/shader.h"
#include "../renderer/mesh.h"
#include "../math/mat4.h"
#include "../types.h"
#include "../core/window.h"
// Color mapping for Rubik's cube faces
typedef enum {
    CUBE_COLOR_WHITE = 'W',   // Top face (U)
    CUBE_COLOR_YELLOW = 'Y',  // Bottom face (D)
    CUBE_COLOR_RED = 'R',     // Front face (F)
    CUBE_COLOR_ORANGE = 'O',  // Back face (B)
    CUBE_COLOR_BLUE = 'B',    // Right face (R)
    CUBE_COLOR_GREEN = 'G'    // Left face (L)
} CubeColor;

// Face indices for a Rubik's cube
typedef enum {
    FACE_IDX_TOP = 0,    // U - White
    FACE_IDX_FRONT = 1,  // F - Red
    FACE_IDX_RIGHT = 2,  // R - Blue 
    FACE_IDX_BACK = 3,   // B - Orange
    FACE_IDX_LEFT = 4,   // L - Green
    FACE_IDX_BOTTOM = 5  // D - Yellow
} FaceIndex;

// Rotation direction
typedef enum {
    ROTATE_CLOCKWISE = 1,
    ROTATE_COUNTERCLOCKWISE = -1,
    ROTATE_180 = 2
} RotationDirection;

struct Scene {
    Shader shader;
    
    Mesh* cubes;
    int numCubes;
    float positions[27][3]; // Координаты кубов
    RGBColor cubeColors[6][9]; // Цвета на гранях
    
    // Анимация вращения
    bool isRotating;
    float rotationAngle;
    float rotationTarget;
    FaceIndex rotatingFace;
    RotationDirection rotationDirection;
    int rotatingLayer;     // 0, 1, 2
    char rotationAxis;     // 'x', 'y', or 'z'
    int rotationRepetitions; 
    float speedMultiplier;   
    
    // Последовательность ходов
    char** moveQueue;      // Массив ходов
    int moveQueueSize;     // Число ходов в очереди
    int moveQueueCapacity; // Максимальная вместимость 
    int currentMoveIndex;  // Индекс текущего хода
    bool processingSequence;
    float originalSpeedBeforeSequence;
    
    // Browse mode 
    bool browseMode; 
    int browseIndex;

    bool colorMode;
    int colorFace;  // Текущая грань
    int cellIndex;  // Текущая ячейка
};

void scene_update(Scene* scene, Window* window, float deltaTime);
void scene_render(Scene* scene, Window* window);
void scene_destroy(Scene* scene);

bool scene_init_rubiks(Scene* scene);

bool scene_set_cube_state_from_string(Scene* scene, const char* state);


char* scene_get_cube_state_as_string(RGBColor (*cubeColors)[9]);
RGBColor* scene_get_cube_colors(Scene* scene);
void rotate_face_colors(RGBColor (*cubeColors)[9], FaceIndex face, RotationDirection direction);


void scene_start_rotation(Scene* scene, FaceIndex face, RotationDirection direction, int repetitions);

bool scene_is_rotating(Scene* scene);

void apply_move_sequence(Scene* scene, char** moveSequence);

// Последовательность ходов
void scene_init_move_queue(Scene* scene);
void scene_destroy_move_queue(Scene* scene);
void scene_add_move_to_queue(Scene* scene, const char* move);
void scene_process_move_queue(Scene* scene);
bool scene_is_processing_sequence(Scene* scene);

// Browse mode 
void scene_enter_browse_mode(Scene* scene);
void scene_exit_browse_mode(Scene* scene);
void scene_browse_next(Scene* scene);
void scene_browse_previous(Scene* scene);
bool scene_is_in_browse_mode(Scene* scene);

// Раскраска
void scene_enter_color_mode(Scene* scene, Window* window);
void scene_exit_color_mode(Scene* scene, Window* window);
bool scene_is_in_color_mode(Scene* scene);
void scene_next_color_face(Scene* scene, Window* window);
void scene_previous_color_face(Scene* scene, Window* window);
void scene_set_color_for_current_cell(Scene* scene, Window* window, char color);

// Animation speed control
void scene_set_speed_multiplier(Scene* scene, float multiplier);
float scene_get_speed_multiplier(Scene* scene);

#endif /* SCENE_H */