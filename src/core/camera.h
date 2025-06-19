#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include "../math/vec3.h"
#include "../math/mat4.h"

typedef struct {
    // Координаты камеры и вектора вверх
    Vec3 position;
    Vec3 up;
    
    // Параметры орбиты
    float radius;     // Растояние до цели
    float theta;      // Горизонтальный угол
    float phi;        // Вертикальный угол
    
    // Параметры проекции
    float fov;        // FOV в градусах
    float nearPlane;  // Растояние после которого объекты не отображатся
    float farPlane;   // Растояние после которого объекты не отображатся
    
    // Состояния интеракции с камерой
    bool isDragging;
    double lastMouseX;
    double lastMouseY;
    
    // Настройки камеры
    float sensitivity;    // Чувсвтительность мыши
    float zoomSpeed;      // Скорость зума
    float minRadius;      // Минимальный зум
    float maxRadius;      // Максимальный зум
    float minPhi;         // Минимальный вертикальный угол
    float maxPhi;         // Максимальный вертикальный угол

    // Для движения по таймеру
    float moveStartTheta, moveEndTheta;
    float moveStartPhi, moveEndPhi;
    float moveTimer;      // Прошедшее время
    float moveDuration;   // Общее время
    bool isMoving;        
} Camera;

void camera_init (Camera* camera);


void camera_process_mouse_movement (Camera* camera, double xpos, double ypos);
void camera_start_drag             (Camera* camera, double xpos, double ypos);
void camera_end_drag               (Camera* camera);
void camera_process_scroll         (Camera* camera, double yoffset);


void camera_update (Camera* camera);


Mat4 camera_get_view_matrix       (Camera* camera);
Mat4 camera_get_projection_matrix (Camera* camera, float aspectRatio);


void camera_move_to         (Camera* camera, float targetTheta, float targetPhi, float duration);
void camera_update_movement (Camera* camera, float deltaTime);
bool camera_is_moving       (Camera* camera);

#endif /* CAMERA_H */ 