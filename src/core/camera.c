#define _USE_MATH_DEFINES

#include "camera.h"
#include <math.h>
#include <stdio.h>

void camera_init(Camera* camera) {
    // Set default camera mode
    camera->mode = CAMERA_MODE_ORBIT;
    
    // Set default target to origin
    camera->target = (Vec3){0.0f, 0.0f, 0.0f};
    camera->up = (Vec3){0.0f, 1.0f, 0.0f};
    
    // Set default orbit parameters
    camera->radius = 5.0f;
    camera->theta = 45.0f;   // 45 degrees around Y axis
    camera->phi = 45.0f;     // 45 degrees from Y axis
    
    // Set default projection parameters
    camera->fov = 45.0f;     // 45 degrees field of view
    camera->nearPlane = 0.1f;
    camera->farPlane = 100.0f;
    
    // Set default camera interaction state
    camera->isDragging = false;
    camera->lastMouseX = 0.0;
    camera->lastMouseY = 0.0;
    
    // Set default camera settings
    camera->sensitivity = 0.3f;
    camera->zoomSpeed = 0.5f;
    camera->minRadius = 2.0f;
    camera->maxRadius = 15.0f;
    camera->minPhi = 5.0f;
    camera->maxPhi = 175.0f;
    
    // Calculate initial camera position
    camera_update(camera);
}

void camera_process_mouse_movement(Camera* camera, double xpos, double ypos) {
    if (!camera->isDragging) {
        return;
    }
    
    double xoffset = xpos - camera->lastMouseX;
    double yoffset = camera->lastMouseY - ypos; // Reversed: y ranges bottom to top
    
    xoffset *= camera->sensitivity;
    yoffset *= camera->sensitivity;
    
    // Update camera orbit angles
    camera->theta -= xoffset; // Moving mouse left increases theta (counter-clockwise)
    camera->phi -= yoffset;   // Moving mouse up decreases phi (moves up from equator)
    
    // Limit phi to avoid gimbal lock
    if (camera->phi < camera->minPhi) camera->phi = camera->minPhi;
    if (camera->phi > camera->maxPhi) camera->phi = camera->maxPhi;
    
    // Normalize theta to keep it in range [0, 360]
    while (camera->theta > 360.0f) camera->theta -= 360.0f;
    while (camera->theta < 0.0f) camera->theta += 360.0f;
    
    camera->lastMouseX = xpos;
    camera->lastMouseY = ypos;
    
    // Update camera position based on new angles
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

void camera_process_scroll(Camera* camera, double yoffset) {
    // Zoom in/out with mouse scroll
    camera->radius -= yoffset * camera->zoomSpeed;
    
    // Limit zoom range
    if (camera->radius < camera->minRadius) camera->radius = camera->minRadius;
    if (camera->radius > camera->maxRadius) camera->radius = camera->maxRadius;
    
    // Update camera position based on new radius
    camera_update(camera);
}

void camera_set_fov(Camera* camera, float fov) {
    // Clamp FOV to reasonable values (10 to 120 degrees)
    if (fov < 5.0f) fov = 5.0f;
    if (fov > 120.0f) fov = 120.0f;
    
    camera->fov = fov;
}

void camera_update(Camera* camera) {
    // Convert from spherical to Cartesian coordinates
    float thetaRad = camera->theta * M_PI / 180.0f;
    float phiRad = camera->phi * M_PI / 180.0f;
    
    camera->position.x = camera->radius * sinf(phiRad) * cosf(thetaRad);
    camera->position.y = camera->radius * cosf(phiRad);
    camera->position.z = camera->radius * sinf(phiRad) * sinf(thetaRad);
    
    // Add target position for centered orbit
    camera->position.x += camera->target.x;
    camera->position.y += camera->target.y;
    camera->position.z += camera->target.z;
}

Vec3 camera_get_position(Camera* camera) {
    return camera->position;
}

Vec3 camera_get_target(Camera* camera) {
    return camera->target;
}

Vec3 camera_get_up(Camera* camera) {
    return camera->up;
}

Mat4 camera_get_view_matrix(Camera* camera) {
    return mat4_look_at(camera->position, camera->target, camera->up);
}

Mat4 camera_get_projection_matrix(Camera* camera, float aspectRatio) {
    // Convert FOV from degrees to radians
    float fovRadians = camera->fov * M_PI / 180.0f;
    
    return mat4_perspective(fovRadians, aspectRatio, camera->nearPlane, camera->farPlane);
} 