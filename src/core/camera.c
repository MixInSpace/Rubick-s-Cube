#define _USE_MATH_DEFINES

#include "camera.h"
#include <math.h>
#include <stdio.h>

void camera_init(Camera* camera) {
    // Положение объекта вращения и направления вверх
    camera->up = (Vec3){0.0f, 1.0f, 0.0f};
    
    // Начальное положение
    camera->radius = 5.0f;
    camera->theta = 45.0f;   // 45 degrees around Y axis
    camera->phi = 45.0f;     // 45 degrees from Y axis
    
    // Параметры проекции
    camera->fov = 45.0f;
    camera->nearPlane = 0.1f;
    camera->farPlane = 100.0f;
    
    // Параметры интеракции с камерой
    camera->isDragging = false;
    camera->lastMouseX = 0.0;
    camera->lastMouseY = 0.0;
    
    camera->sensitivity = 0.3f;
    camera->zoomSpeed = 0.5f;
    camera->minRadius = 2.0f;
    camera->maxRadius = 15.0f;
    camera->minPhi = 5.0f;
    camera->maxPhi = 175.0f;
    
    camera_update(camera);
}

// Обработка движения мыши
void camera_process_mouse_movement(Camera* camera, double xpos, double ypos) {
    if (!camera->isDragging) {
        return;
    }
    
    // dx, dy
    double xoffset = xpos - camera->lastMouseX;
    double yoffset = camera->lastMouseY - ypos;
    
    xoffset *= camera->sensitivity;
    yoffset *= camera->sensitivity;
    
    // Обновление углов полжения камеры на dx, dy
    camera->theta -= xoffset; 
    camera->phi -= yoffset; 
    
    // Ограничение вращения phi для того чтобы избежать gimbal lock
    if (camera->phi < camera->minPhi) camera->phi = camera->minPhi;
    if (camera->phi > camera->maxPhi) camera->phi = camera->maxPhi;
    
    // Нормализация theta до [0.0, 360.0]
    while (camera->theta > 360.0f) camera->theta -= 360.0f;
    while (camera->theta < 0.0f) camera->theta += 360.0f;
    
    camera->lastMouseX = xpos;
    camera->lastMouseY = ypos;
    
    // Обновить позицию камеры на основании новых углов
    camera_update(camera);
}

void camera_start_drag(Camera* camera, double xpos, double ypos) {
    camera->isDragging = true;
    camera->lastMouseX = xpos;
    camera->lastMouseY = ypos;
}

void camera_end_drag(Camera* camera) {
    camera->isDragging = false;
}

// Обработка скролла
void camera_process_scroll(Camera* camera, double yoffset) {
    // Приближение 
    camera->radius -= yoffset * camera->zoomSpeed;
    
    // Ограничение зума
    if (camera->radius < camera->minRadius) camera->radius = camera->minRadius;
    if (camera->radius > camera->maxRadius) camera->radius = camera->maxRadius;
    
    // Обновление позиции камеры на новом радиусе
    camera_update(camera);
}

void camera_update(Camera* camera) {
    // Преобразование из сферических в декартовы
    float thetaRad = camera->theta * M_PI / 180.0f;
    float phiRad = camera->phi * M_PI / 180.0f;
    
    camera->position.x = camera->radius * sinf(phiRad) * cosf(thetaRad);
    camera->position.y = camera->radius * cosf(phiRad);
    camera->position.z = camera->radius * sinf(phiRad) * sinf(thetaRad);
}


// Проекции для шейдеров
Mat4 camera_get_view_matrix(Camera* camera) {
    return mat4_look_at(camera->position, (Vec3){0.0f, 0.0f, 0.0f}, camera->up);
}

Mat4 camera_get_projection_matrix(Camera* camera, float aspectRatio) {
    // FOV deg to rad
    float fovRadians = camera->fov * M_PI / 180.0f;
    
    return mat4_perspective(fovRadians, aspectRatio, camera->nearPlane, camera->farPlane);
}

// Перемещение камеры до позиции за n секунд
void camera_move_to(Camera* camera, float targetTheta, float targetPhi, float duration) {
    camera->moveStartTheta = camera->theta;
    camera->moveEndTheta = targetTheta;
    camera->moveStartPhi = camera->phi;
    camera->moveEndPhi = targetPhi;
    camera->moveTimer = 0.0f;
    camera->moveDuration = duration > 0.0f ? duration : 0.01f;
    camera->isMoving = true;
}

// Вызывается каждый фрейм чтобы апдейтить позицию камеры
void camera_update_movement(Camera* camera, float deltaTime) {
    if (!camera->isMoving) return;
    camera->moveTimer += deltaTime;
    float t = camera->moveTimer / camera->moveDuration;
    if (t >= 1.0f) {
        camera->theta = camera->moveEndTheta;
        camera->phi = camera->moveEndPhi;
        camera->isMoving = false;
    } else {
        camera->theta = camera->moveStartTheta + t * (camera->moveEndTheta - camera->moveStartTheta);
        camera->phi = camera->moveStartPhi + t * (camera->moveEndPhi - camera->moveStartPhi);
    }
    camera_update(camera);
}

bool camera_is_moving(Camera* camera) {
    return camera->isMoving;
} 