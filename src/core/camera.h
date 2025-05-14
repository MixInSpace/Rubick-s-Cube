#ifndef CAMERA_H
#define CAMERA_H

#include <stdbool.h>
#include "../math/vec3.h"
#include "../math/mat4.h"

// Defines camera modes (possible future expansion)
typedef enum {
    CAMERA_MODE_ORBIT,   // Orbits around a target point
    CAMERA_MODE_FREE     // Free-flying camera (not implemented yet)
} CameraMode;

// Camera structure containing all camera data
typedef struct {
    CameraMode mode;
    
    // Position and target
    Vec3 position;
    Vec3 target;
    Vec3 up;
    
    // Orbit camera parameters
    float radius;     // Distance from camera to target
    float theta;      // Horizontal angle in degrees (around Y-axis)
    float phi;        // Vertical angle in degrees (from Y-axis)
    
    // Projection parameters
    float fov;        // Field of view in degrees
    float nearPlane;  // Near clipping plane
    float farPlane;   // Far clipping plane
    
    // Camera interaction state
    bool isDragging;
    double lastMouseX;
    double lastMouseY;
    
    // Camera settings
    float sensitivity;    // Mouse sensitivity
    float zoomSpeed;      // Zoom speed with scroll wheel
    float minRadius;      // Minimum zoom distance
    float maxRadius;      // Maximum zoom distance
    float minPhi;         // Minimum vertical angle (degrees)
    float maxPhi;         // Maximum vertical angle (degrees)
} Camera;

// Initialize camera with default settings
void camera_init(Camera* camera);

// Process mouse movement for camera rotation
void camera_process_mouse_movement(Camera* camera, double xpos, double ypos);

// Start camera drag operation
void camera_start_drag(Camera* camera, double xpos, double ypos);

// End camera drag operation
void camera_end_drag(Camera* camera);

// Process mouse scroll for camera zoom
void camera_process_scroll(Camera* camera, double yoffset);

// Set camera field of view (in degrees)
void camera_set_fov(Camera* camera, float fov);

// Update camera position based on current orbit parameters
void camera_update(Camera* camera);

// Get camera position
Vec3 camera_get_position(Camera* camera);

// Get camera target (look-at point)
Vec3 camera_get_target(Camera* camera);

// Get camera up vector
Vec3 camera_get_up(Camera* camera);

// Get view matrix for the camera
Mat4 camera_get_view_matrix(Camera* camera);

// Get projection matrix for the camera
Mat4 camera_get_projection_matrix(Camera* camera, float aspectRatio);

#endif /* CAMERA_H */ 