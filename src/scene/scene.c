#define _USE_MATH_DEFINES

#include <GL/glew.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "../core/window.h"
#include "../resources/embedded_shaders.h"
#include "scene.h"

// Helper functions to identify cube types based on position
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

// Get the RGB values for a given cube color character
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
            // Default to gray for unknown colors
            rgb.r = 0.5f; rgb.g = 0.5f; rgb.b = 0.5f;
    }
    
    return rgb;
}

// Helper function to convert RGB color back to character
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

// Function to get the position mapping from a face index and position within that face
// Returns the index of the cube in the 3x3x3 grid (0-26)
static int get_cube_index_from_face_position(int faceIndex, int position) {
    // Convert from the 3x3 face grid (0-8) to the position in the 3D layout
    // For each face, we need a mapping from the face position (0-8) to the cube index (0-26)
    
    // Map face positions (for 3x3 grid on each face):
    // 0 1 2
    // 3 4 5
    // 6 7 8
    
    // Convert to coordinates based on face
    int x, y, z;
    
    switch (faceIndex) {
        case FACE_IDX_TOP: // U - Top face (y=2)
            y = 2;
            x = position % 3;
            z = 2 - (position / 3);
            break;
            
        case FACE_IDX_BOTTOM: // D - Bottom face (y=0)
            y = 0;
            x = position % 3;
            z = position / 3;
            break;
            
        case FACE_IDX_FRONT: // F - Front face (z=2)
            z = 2;
            x = position % 3;
            y = 2 - (position / 3);
            break;
            
        case FACE_IDX_BACK: // B - Back face (z=0)
            z = 0;
            x = 2 - (position % 3);
            y = 2 - (position / 3);
            break;
            
        case FACE_IDX_LEFT: // L - Left face (x=0)
            x = 0;
            z = 2 - (position % 3);
            y = 2 - (position / 3);
            break;
            
        case FACE_IDX_RIGHT: // R - Right face (x=2)
            x = 2;
            z = position % 3;
            y = 2 - (position / 3);
            break;
            
        default:
            // Invalid face index
            return -1;
    }
    
    // Calculate 3D index from x,y,z coordinates
    return x + y * 3 + z * 9;
}

// Function to determine which cube face in the 3D model corresponds to a given 2D face position
static CubeFace get_cube_face_from_face_index(int faceIndex) {
    switch (faceIndex) {
        case FACE_IDX_TOP: return FACE_TOP;
        case FACE_IDX_BOTTOM: return FACE_BOTTOM;
        case FACE_IDX_FRONT: return FACE_FRONT;
        case FACE_IDX_BACK: return FACE_BACK;
        case FACE_IDX_LEFT: return FACE_LEFT;
        case FACE_IDX_RIGHT: return FACE_RIGHT;
        default: return 0;
    }
}

bool scene_init(Scene* scene) {
    scene->id = 1;
    scene->isRubiksCube = false;
    
    // Initialize shader using embedded shader source
    if (!shader_init_from_source(&scene->shader, DEFAULT_VERTEX_SHADER, DEFAULT_FRAGMENT_SHADER)) {
        fprintf(stderr, "Failed to initialize shader\n");
        return false;
    }
    
    // Create a single cube mesh
    scene->cube = create_cube_mesh();
    
    // Enable depth testing and set up anti-aliasing friendly settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);  // Ensure MSAA is enabled
    glEnable(GL_LINE_SMOOTH);  // Smooth lines
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH); // Smooth polygons
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    
    return true;
}

bool scene_init_rubiks(Scene* scene) {
    scene->id = 2;
    scene->isRubiksCube = true;
    scene->numCubes = 27; // 3x3x3 grid
    
    // Initialize rotation state
    scene->isRotating = false;
    scene->rotationAngle = 0.0f;
    scene->rotationTarget = 0.0f;
    scene->rotatingLayer = 0;
    scene->rotationAxis = 'y';
    
    // Initialize move queue
    scene_init_move_queue(scene);
    
    // Initialize the default colors for each face
    // White for top (U)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_TOP][i] = (RGBColor){1.0f, 1.0f, 1.0f};
    }
    
    // Red for front (F)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_FRONT][i] = (RGBColor){1.0f, 0.0f, 0.0f};
    }
    
    // Blue for right (R)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_RIGHT][i] = (RGBColor){0.0f, 0.0f, 1.0f};
    }
    
    // Orange for back (B)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_BACK][i] = (RGBColor){1.0f, 0.5f, 0.0f};
    }
    
    // Green for left (L)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_LEFT][i] = (RGBColor){0.0f, 0.8f, 0.0f};
    }
    
    // Yellow for bottom (D)
    for (int i = 0; i < 9; i++) {
        scene->cubeColors[FACE_IDX_BOTTOM][i] = (RGBColor){1.0f, 1.0f, 0.0f};
    }
    
    // Initialize shader with textured shaders instead of default ones
    if (!shader_init_from_source(&scene->shader, TEXTURED_VERTEX_SHADER, TEXTURED_FRAGMENT_SHADER)) {
        fprintf(stderr, "Failed to initialize shader\n");
        return false;
    }
    
    // Create the cubes
    scene->cubes = (Mesh*)malloc(scene->numCubes * sizeof(Mesh));
    if (!scene->cubes) {
        fprintf(stderr, "Failed to allocate memory for Rubik's cube meshes\n");
        return false;
    }
    
    // The size of one cube (not scaled)
    float cubeSize = 0.3f;

    float gap = 0.02f;
    
    // Calculate the total step between cube centers (cube size + gap)
    float step = cubeSize + gap;
    
    // Starting offset to center the 3x3x3 grid at origin
    // For a 3×3×3 grid starting at the origin, we need to offset by -step
    float offset = -step;
    
    int index = 0;
    
    // Create a 3x3x3 grid of cubes and store their positions
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                // Determine which faces should be visible based on position
                unsigned int visibleFaces = 0;
                
                // Front face is visible for cubes at z=2 (front layer)
                if (z == 2) visibleFaces |= FACE_FRONT;
                
                // Back face is visible for cubes at z=0 (back layer)
                if (z == 0) visibleFaces |= FACE_BACK;
                
                // Top face is visible for cubes at y=2 (top layer)
                if (y == 2) visibleFaces |= FACE_TOP;
                
                // Bottom face is visible for cubes at y=0 (bottom layer)
                if (y == 0) visibleFaces |= FACE_BOTTOM;
                
                // Right face is visible for cubes at x=2 (right layer)
                if (x == 2) visibleFaces |= FACE_RIGHT;
                
                // Left face is visible for cubes at x=0 (left layer)
                if (x == 0) visibleFaces |= FACE_LEFT;
                
                // Create a cube with only the visible faces colored normally
                // (internal faces will be black)
                scene->cubes[index] = create_textured_cube_mesh(visibleFaces);
                
                // Assign texture based on cube type: center pieces get center texture, others get regular texture
                if (is_center_piece(x, y, z)) {
                    scene->cubes[index].textureType = TEXTURE_STICKER_CENTER;
                } else {
                    scene->cubes[index].textureType = TEXTURE_STICKER;
                }
                
                // Store position for this cube
                scene->positions[index][0] = offset + x * step; // X position
                scene->positions[index][1] = offset + y * step; // Y position
                scene->positions[index][2] = offset + z * step; // Z position
                
                index++;
            }
        }
    }
    
    // Enable depth testing and anti-aliasing settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    
    // Enable alpha blending for transparency
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    return true;
}

// Update rotation animation
static void update_rotation(Scene* scene, float deltaTime) {
    if (!scene->isRotating) {
        return;
    }
    
    // Rotation speed (degrees per second)
    float rotationSpeed = 270.0f; // 270 degrees per second (1/3 second per rotation)
    
    // Update rotation angle
    scene->rotationAngle += rotationSpeed * deltaTime;
    
    // If rotation is complete
    if (scene->rotationAngle >= scene->rotationTarget) {
        scene->isRotating = false;
        scene->rotationAngle = 0.0f;
        
        // Apply the rotation to the cube state the correct number of times
        for (int i = 0; i < scene->rotationRepetitions; i++) {
            scene_rotate_face(scene, scene->rotatingFace, scene->rotationDirection);
        }
    }
}

void scene_update(Scene* scene, float deltaTime) {
    // Update rotation animation if in progress
    update_rotation(scene, deltaTime);
    
    // Process move queue if not currently rotating
    if (scene->isRubiksCube && !scene->isRotating) {
        scene_process_move_queue(scene);
    }
}

void scene_render(Scene* scene, Window* window) {
    // Clear the screen
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  // Dark background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // Use shader
    shader_use(&scene->shader);
    
    // Get camera from window
    Camera* camera = window_get_camera(window);
    
    // Get view and projection matrices from camera
    Mat4 view = camera_get_view_matrix(camera);
    float aspectRatio = (float)window->width / (float)window->height;
    Mat4 projection = camera_get_projection_matrix(camera, aspectRatio);
    
    // Set view and projection uniforms (these are the same for all cubes)
    shader_set_mat4(&scene->shader, "view", view);
    shader_set_mat4(&scene->shader, "projection", projection);
    
    // Set texture sampler uniform to use texture unit 0
    shader_set_int(&scene->shader, "textureMap", 0);
    // Enable textures
    shader_set_bool(&scene->shader, "useTexture", true);
    
    // Render the Rubik's cube grid
    for (int i = 0; i < scene->numCubes; i++) {
        // Get cube position in grid (0-2 for each dimension)
        int x = i % 3;              // 0, 1, 2, 0, 1, 2, ...
        int y = (i / 3) % 3;        // 0, 0, 0, 1, 1, 1, ...
        int z = i / 9;              // 0, 0, 0, 0, 0, 0, ...
        
        // Check if this cube is part of the rotating layer based on the face
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
        
        // Create model matrix for this cube
        Mat4 model = mat4_identity();
        
        if (isRotatingCube) {
            // Calculate rotation angle in radians
            float angleRad = scene->rotationAngle * (float)scene->rotationDirection * -(float)M_PI / 180.0f;
            
            // Apply rotation based on axis
            if (scene->rotationAxis == 'x') {
                // Model matrix: First create a rotation matrix
                Mat4 rotationMatrix = mat4_rotation_x(angleRad);
                
                // Apply rotation to the position 
                Vec3 pos = {
                    scene->positions[i][0],
                    scene->positions[i][1],
                    scene->positions[i][2]
                };
                
                // Rotate position around origin
                Vec3 rotated;
                rotated.x = pos.x;
                rotated.y = pos.y * cosf(angleRad) - pos.z * sinf(angleRad);
                rotated.z = pos.y * sinf(angleRad) + pos.z * cosf(angleRad);
                
                // Create model matrix with rotated position
                model = mat4_translation(rotated.x, rotated.y, rotated.z);
                
                // Apply same rotation to the cube orientation
                model = mat4_multiply(model, rotationMatrix);
                
            } else if (scene->rotationAxis == 'y') {
                // Model matrix: First create a rotation matrix
                Mat4 rotationMatrix = mat4_rotation_y(angleRad);
                
                // Apply rotation to the position 
                Vec3 pos = {
                    scene->positions[i][0],
                    scene->positions[i][1],
                    scene->positions[i][2]
                };
                
                // Rotate position around origin
                Vec3 rotated;
                rotated.x = pos.x * cosf(angleRad) + pos.z * sinf(angleRad);
                rotated.y = pos.y;
                rotated.z = -pos.x * sinf(angleRad) + pos.z * cosf(angleRad);
                
                // Create model matrix with rotated position
                model = mat4_translation(rotated.x, rotated.y, rotated.z);
                
                // Apply same rotation to the cube orientation
                model = mat4_multiply(model, rotationMatrix);
                
            } else { // 'z'
                // Model matrix: First create a rotation matrix
                Mat4 rotationMatrix = mat4_rotation_z(angleRad);
                
                // Apply rotation to the position 
                Vec3 pos = {
                    scene->positions[i][0],
                    scene->positions[i][1],
                    scene->positions[i][2]
                };
                
                // Rotate position around origin
                Vec3 rotated;
                rotated.x = pos.x * cosf(angleRad) - pos.y * sinf(angleRad);
                rotated.y = pos.x * sinf(angleRad) + pos.y * cosf(angleRad);
                rotated.z = pos.z;
                
                // Create model matrix with rotated position
                model = mat4_translation(rotated.x, rotated.y, rotated.z);
                
                // Apply same rotation to the cube orientation
                model = mat4_multiply(model, rotationMatrix);
            }
        } else {
            // Standard positioning for non-rotating cubes
            model = mat4_translation(
                scene->positions[i][0],
                scene->positions[i][1],
                scene->positions[i][2]
            );
        }
        
        // Scale the cube (reduce size to create the gap effect)
        model = mat4_scale_vec3(model, (Vec3){0.3f, 0.3f, 0.3f});
        
        // Set the model matrix uniform for this cube
        shader_set_mat4(&scene->shader, "model", model);
        
        // Draw this cube
        mesh_draw(&scene->cubes[i]);
    }
}

void scene_destroy(Scene* scene) {
    // Free resources
    shader_destroy(&scene->shader);
    
    if (scene->isRubiksCube) {
        // Free move queue
        scene_destroy_move_queue(scene);
        
        // Free all the cubes in the grid
        for (int i = 0; i < scene->numCubes; i++) {
            mesh_destroy(&scene->cubes[i]);
        }
        
        // Free the array
        free(scene->cubes);
        scene->cubes = NULL;
        scene->numCubes = 0;
    } else {
        // Free the single cube
        mesh_destroy(&scene->cube);
    }
}

// Implementation of scene_set_cube_state_from_string function
bool scene_set_cube_state_from_string(Scene* scene, const char* state) {
    if (!scene || !state || !scene->isRubiksCube) {
        return false;
    }
    
    // Check if the string is valid (should be 54 characters - 9 per face * 6 faces)
    size_t len = strlen(state);
    if (len != 54) {
        fprintf(stderr, "Invalid cube state string length: %zu (expected 54)\n", len);
        return false;
    }
    
    fprintf(stderr, "Setting cube state from string: %s\n", state);
    
    // Define how the string maps to face indices
    // The order in the string is: U, R, F, D, L, B
    // But our internal face indices are: 
    // FACE_IDX_TOP(0), FACE_IDX_FRONT(1), FACE_IDX_RIGHT(2), FACE_IDX_BACK(3), FACE_IDX_LEFT(4), FACE_IDX_BOTTOM(5)
    const int stringToFaceMap[6] = {
        FACE_IDX_TOP,     // U - First 9 chars (0-8) -> Top face
        FACE_IDX_RIGHT,   // R - Next 9 chars (9-17) -> Right face
        FACE_IDX_FRONT,   // F - Next 9 chars (18-26) -> Front face
        FACE_IDX_BOTTOM,  // D - Next 9 chars (27-35) -> Bottom face
        FACE_IDX_LEFT,    // L - Next 9 chars (36-44) -> Left face
        FACE_IDX_BACK     // B - Last 9 chars (45-53) -> Back face
    };
    
    // We need to recreate all the cubes with new colors
    // First, free the existing cubes
    for (int i = 0; i < scene->numCubes; i++) {
        mesh_destroy(&scene->cubes[i]);
    }
    
    // Now we'll create new cubes with the updated colors
    int index = 0;
    
    // Size of one cube (not scaled)
    float cubeSize = 0.3f;
    float gap = 0.02f;
    float step = cubeSize + gap;
    float offset = -step;
    
    // Store color data for each face
    for (int stringFace = 0; stringFace < 6; stringFace++) {
        int faceIndex = stringToFaceMap[stringFace];
        
        fprintf(stderr, "Processing face %d (string section %d): ", faceIndex, stringFace);
        for (int pos = 0; pos < 9; pos++) {
            // Get the character for this position
            char colorChar = state[stringFace * 9 + pos];
            fprintf(stderr, "%c", colorChar);
            
            // Convert to uppercase for consistency
            colorChar = (colorChar >= 'a' && colorChar <= 'z') ? 
                        (char)(colorChar - 'a' + 'A') : colorChar;
            
            // Store the RGB color in the scene
            scene->cubeColors[faceIndex][pos] = get_rgb_from_cube_color(colorChar);
        }
        fprintf(stderr, "\n");
    }
    
    // Create a 3x3x3 grid of cubes with updated colors
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                // Determine which faces should be visible based on position
                unsigned int visibleFaces = 0;
                
                // Front face is visible for cubes at z=2 (front layer)
                if (z == 2) visibleFaces |= FACE_FRONT;
                
                // Back face is visible for cubes at z=0 (back layer)
                if (z == 0) visibleFaces |= FACE_BACK;
                
                // Top face is visible for cubes at y=2 (top layer)
                if (y == 2) visibleFaces |= FACE_TOP;
                
                // Bottom face is visible for cubes at y=0 (bottom layer)
                if (y == 0) visibleFaces |= FACE_BOTTOM;
                
                // Right face is visible for cubes at x=2 (right layer)
                if (x == 2) visibleFaces |= FACE_RIGHT;
                
                // Left face is visible for cubes at x=0 (left layer)
                if (x == 0) visibleFaces |= FACE_LEFT;
                
                // Create a cube with custom colors for visible faces
                scene->cubes[index] = create_custom_colored_cube(visibleFaces, scene, x, y, z);
                
                // Store position for this cube
                scene->positions[index][0] = offset + x * step;
                scene->positions[index][1] = offset + y * step;
                scene->positions[index][2] = offset + z * step;
                
                index++;
            }
        }
    }
    
    return true;
}

char* scene_get_cube_state_as_string(Scene* scene) {
    if (!scene || !scene->isRubiksCube) {
        return NULL;
    }
    
    char* state = (char*)malloc(55 * sizeof(char));
    if (!state) {
        return NULL;
    }
    

    const int faceToStringMap[6] = {
        FACE_IDX_TOP,     // U - First 9 chars (0-8)
        FACE_IDX_RIGHT,   // R - Next 9 chars (9-17)
        FACE_IDX_FRONT,   // F - Next 9 chars (18-26)
        FACE_IDX_BOTTOM,  // D - Next 9 chars (27-35)
        FACE_IDX_LEFT,    // L - Next 9 chars (36-44)
        FACE_IDX_BACK     // B - Last 9 chars (45-53)
    };
    
    int stringIndex = 0;
    for (int stringFace = 0; stringFace < 6; stringFace++) {
        int faceIndex = faceToStringMap[stringFace];
        
        for (int pos = 0; pos < 9; pos++) {
            RGBColor color = scene->cubeColors[faceIndex][pos];
            state[stringIndex] = rgb_to_char(color);
            stringIndex++;
        }
    }
    
    state[54] = '\0';
    
    return state;
}

RGBColor* scene_get_cube_colors(Scene* scene) {
    return scene->cubeColors;
}

// Helper function to create a cube with custom colors based on its position and the stored cube colors
Mesh create_custom_colored_cube(unsigned int visibleFaces, Scene* scene, int x, int y, int z) {
    Mesh mesh;
    mesh_init(&mesh);
    
    // Default color for non-visible faces
    float black[3] = {0.05f, 0.05f, 0.05f};
    
    // Determine colors for each face based on position and stored color data
    RGBColor frontColor = (visibleFaces & FACE_FRONT) ? 
        scene->cubeColors[FACE_IDX_FRONT][x + (2-y)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor backColor = (visibleFaces & FACE_BACK) ? 
        scene->cubeColors[FACE_IDX_BACK][(2-x) + (2-y)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor topColor = (visibleFaces & FACE_TOP) ? 
        scene->cubeColors[FACE_IDX_TOP][x + (2-z)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor bottomColor = (visibleFaces & FACE_BOTTOM) ? 
        scene->cubeColors[FACE_IDX_BOTTOM][x + z*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor rightColor = (visibleFaces & FACE_RIGHT) ? 
        scene->cubeColors[FACE_IDX_RIGHT][z + (2-y)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor leftColor = (visibleFaces & FACE_LEFT) ? 
        scene->cubeColors[FACE_IDX_LEFT][(2-z) + (2-y)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    // Cube vertex data: positions (x,y,z), colors (r,g,b), and texture coordinates (s,t)
    float vertices[] = {
        // Front face
        -0.5f, -0.5f,  0.5f,  frontColor.r, frontColor.g, frontColor.b,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  frontColor.r, frontColor.g, frontColor.b,  1.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  frontColor.r, frontColor.g, frontColor.b,  1.0f, 1.0f,
        -0.5f,  0.5f,  0.5f,  frontColor.r, frontColor.g, frontColor.b,  0.0f, 1.0f,
        
        // Back face
        -0.5f, -0.5f, -0.5f,  backColor.r, backColor.g, backColor.b,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  backColor.r, backColor.g, backColor.b,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  backColor.r, backColor.g, backColor.b,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  backColor.r, backColor.g, backColor.b,  0.0f, 1.0f,
        
        // Top face
        -0.5f,  0.5f,  0.5f,  topColor.r, topColor.g, topColor.b,  0.0f, 0.0f,
         0.5f,  0.5f,  0.5f,  topColor.r, topColor.g, topColor.b,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  topColor.r, topColor.g, topColor.b,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  topColor.r, topColor.g, topColor.b,  0.0f, 1.0f,
        
        // Bottom face
        -0.5f, -0.5f,  0.5f,  bottomColor.r, bottomColor.g, bottomColor.b,  0.0f, 0.0f,
         0.5f, -0.5f,  0.5f,  bottomColor.r, bottomColor.g, bottomColor.b,  1.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  bottomColor.r, bottomColor.g, bottomColor.b,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f,  bottomColor.r, bottomColor.g, bottomColor.b,  0.0f, 1.0f,
        
        // Right face
         0.5f, -0.5f,  0.5f,  rightColor.r, rightColor.g, rightColor.b,  0.0f, 0.0f,
         0.5f, -0.5f, -0.5f,  rightColor.r, rightColor.g, rightColor.b,  1.0f, 0.0f,
         0.5f,  0.5f, -0.5f,  rightColor.r, rightColor.g, rightColor.b,  1.0f, 1.0f,
         0.5f,  0.5f,  0.5f,  rightColor.r, rightColor.g, rightColor.b,  0.0f, 1.0f,
        
        // Left face
        -0.5f, -0.5f, -0.5f,  leftColor.r, leftColor.g, leftColor.b,  0.0f, 0.0f,
        -0.5f, -0.5f,  0.5f,  leftColor.r, leftColor.g, leftColor.b,  1.0f, 0.0f,
        -0.5f,  0.5f,  0.5f,  leftColor.r, leftColor.g, leftColor.b,  1.0f, 1.0f,
        -0.5f,  0.5f, -0.5f,  leftColor.r, leftColor.g, leftColor.b,  0.0f, 1.0f
    };
    
    // Cube indices (triangles)
    unsigned int indices[] = {
        // Front face
        0, 1, 2,
        2, 3, 0,
        
        // Back face
        4, 5, 6,
        6, 7, 4,
        
        // Top face
        8, 9, 10,
        10, 11, 8,
        
        // Bottom face
        12, 13, 14,
        14, 15, 12,
        
        // Right face
        16, 17, 18,
        18, 19, 16,
        
        // Left face
        20, 21, 22,
        22, 23, 20
    };
    
    mesh.vertexCount = 24;  // 6 faces, 4 vertices per face
    mesh.indexCount = 36;   // 12 triangles, 3 indices each
    mesh.drawMode = GL_TRIANGLES;
    mesh.hasTexture = true;
    
    // Assign texture based on cube type: center pieces get center texture, others get regular texture
    if (is_center_piece(x, y, z)) {
        mesh.textureType = TEXTURE_STICKER_CENTER;
    } else {
        mesh.textureType = TEXTURE_STICKER;
    }
    
    // Bind the Vertex Array Object first
    glBindVertexArray(mesh.VAO);
    
    // Bind and set vertex buffer(s)
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Bind and set element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Unbind to prevent accidental modification
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return mesh;
}

// Helper function to check if rotation is in progress
bool scene_is_rotating(Scene* scene) {
    return scene->isRotating;
}

// Start a face rotation animation
void scene_start_rotation(Scene* scene, FaceIndex face, RotationDirection direction, int repetitions) {
    if (scene->isRotating) {
        return; // Don't start a new rotation if one is in progress
    }
    
    scene->isRotating = true;
    scene->rotationAngle = 0.0f;
    scene->rotationTarget = 90.0f * repetitions; // Calculate animation angle from repetitions
    scene->rotatingFace = face;
    scene->rotationRepetitions = repetitions;
    
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

// Helper function to get cube indices for a specific layer
static void get_layer_cubes(int layer, char axis, int* indices, int* count) {
    *count = 0;
    
    for (int i = 0; i < 27; i++) {
        int x = i % 3;              // 0, 1, 2, 0, 1, 2, ...
        int y = (i / 3) % 3;        // 0, 0, 0, 1, 1, 1, ...
        int z = i / 9;              // 0, 0, 0, 0, 0, 0, ...
        
        // Check if this cube belongs to the specified layer
        if ((axis == 'x' && x == layer) ||
            (axis == 'y' && y == layer) ||
            (axis == 'z' && z == layer)) {
            indices[*count] = i;
            (*count)++;
        }
    }
}

// Rotate the colors on a face (and adjacent faces)
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

// The main function to rotate a face
void scene_rotate_face(Scene* scene, FaceIndex face, RotationDirection direction) {
    if (!scene->isRubiksCube) {
        return;
    }
    
    // Step 1: Rotate the colors on the faces
    rotate_face_colors(scene->cubeColors, face, direction);
    
    // Step 2: Rebuild the cubes to apply the new colors
    // First, free the existing cubes
    for (int i = 0; i < scene->numCubes; i++) {
        mesh_destroy(&scene->cubes[i]);
    }
    
    // Now recreate cubes with the updated colors
    float cubeSize = 0.3f;
    float gap = 0.02f;
    float step = cubeSize + gap;
    float offset = -step;
    
    int index = 0;
    for (int x = 0; x < 3; x++) {
        for (int y = 0; y < 3; y++) {
            for (int z = 0; z < 3; z++) {
                // Determine which faces should be visible based on position
                unsigned int visibleFaces = 0;
                
                // Front face is visible for cubes at z=2 (front layer)
                if (z == 2) visibleFaces |= FACE_FRONT;
                
                // Back face is visible for cubes at z=0 (back layer)
                if (z == 0) visibleFaces |= FACE_BACK;
                
                // Top face is visible for cubes at y=2 (top layer)
                if (y == 2) visibleFaces |= FACE_TOP;
                
                // Bottom face is visible for cubes at y=0 (bottom layer)
                if (y == 0) visibleFaces |= FACE_BOTTOM;
                
                // Right face is visible for cubes at x=2 (right layer)
                if (x == 2) visibleFaces |= FACE_RIGHT;
                
                // Left face is visible for cubes at x=0 (left layer)
                if (x == 0) visibleFaces |= FACE_LEFT;
                
                // Create a cube with custom colors for visible faces
                scene->cubes[index] = create_custom_colored_cube(visibleFaces, scene, x, y, z);
                
                // Store position for this cube
                scene->positions[index][0] = offset + x * step;
                scene->positions[index][1] = offset + y * step;
                scene->positions[index][2] = offset + z * step;
                
                index++;
            }
        }
    }
}

// Move sequence management functions

void scene_init_move_queue(Scene* scene) {
    scene->moveQueue = NULL;
    scene->moveQueueSize = 0;
    scene->moveQueueCapacity = 0;
    scene->currentMoveIndex = 0;
    scene->processingSequence = false;
}

void scene_destroy_move_queue(Scene* scene) {
    if (scene->moveQueue) {
        // Free all move strings
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
}

void scene_add_move_to_queue(Scene* scene, const char* move) {
    if (!scene || !move) return;
    
    // Expand queue if needed
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
    
    // Copy the move string
    size_t moveLen = strlen(move);
    scene->moveQueue[scene->moveQueueSize] = (char*)malloc((moveLen + 1) * sizeof(char));
    if (!scene->moveQueue[scene->moveQueueSize]) {
        fprintf(stderr, "Failed to allocate memory for move string\n");
        return;
    }
    strcpy(scene->moveQueue[scene->moveQueueSize], move);
    scene->moveQueueSize++;
}

// Helper function to parse a move string and extract face and direction
static bool parse_move_string(const char* moveStr, FaceIndex* face, RotationDirection* direction, int* repetitions) {
    if (!moveStr || strlen(moveStr) == 0) return false;
    
    *repetitions = 1; // Default to single turn
    *direction = ROTATE_CLOCKWISE; // Default direction
    
    // Parse face
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
    
    // Parse modifiers (prime and number)
    for (size_t i = 1; i < strlen(moveStr); i++) {
        char modifier = moveStr[i];
        if (modifier == '\'' || modifier == '\'') {
            *direction = ROTATE_COUNTERCLOCKWISE;
        } else if (modifier == '2') {
            *repetitions = 2;
        } else if (modifier == '3') {
            *repetitions = 3;
        }
    }
    
    return true;
}

void scene_process_move_queue(Scene* scene) {
    if (!scene || !scene->processingSequence || scene->currentMoveIndex >= scene->moveQueueSize) {
        return;
    }
    
    // Start the next move if not currently rotating
    if (!scene->isRotating) {
        const char* moveStr = scene->moveQueue[scene->currentMoveIndex];
        FaceIndex face;
        RotationDirection direction;
        int repetitions;
        
        if (parse_move_string(moveStr, &face, &direction, &repetitions)) {
            printf("Executing move %d/%d: %s\n", 
                   scene->currentMoveIndex + 1, scene->moveQueueSize, moveStr);
            
            // For 3 repetitions, it's equivalent to 1 counter-clockwise
            if (repetitions == 3) {
                direction = ROTATE_COUNTERCLOCKWISE;
                repetitions = 1;
            }
            
            scene_start_rotation(scene, face, direction, repetitions);
            scene->currentMoveIndex++;
            
            // Check if sequence is complete
            if (scene->currentMoveIndex >= scene->moveQueueSize) {
                printf("Move sequence completed!\n");
                scene->processingSequence = false;
                scene->currentMoveIndex = 0;
                scene->moveQueueSize = 0; // Clear the queue
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

// Function to apply a sequence of moves (called from window.c)
void apply_move_sequence(Scene* scene, char** moveSequence) {
    if (!scene || !scene->isRubiksCube || !moveSequence) {
        return;
    }
    
    // Don't start a new sequence if one is already in progress
    if (scene->processingSequence || scene->isRotating) {
        printf("Cannot start new move sequence: one is already in progress\n");
        return;
    }
    
    // Clear any existing queue
    scene_destroy_move_queue(scene);
    scene_init_move_queue(scene);
    
    // Count moves and add them to queue
    int moveCount = 0;
    while (moveSequence[moveCount] != NULL) {
        moveCount++;
    }
    
    if (moveCount == 0) {
        printf("Empty move sequence provided\n");
        return;
    }
    
    printf("Starting move sequence with %d moves: ", moveCount);
    for (int i = 0; i < moveCount; i++) {
        scene_add_move_to_queue(scene, moveSequence[i]);
        printf("%s ", moveSequence[i]);
    }
    printf("\n");
    
    // Start processing the sequence
    scene->processingSequence = true;
    scene->currentMoveIndex = 0;
}
