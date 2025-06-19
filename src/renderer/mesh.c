#include "mesh.h"
#include "texture.h"
#include "../scene/scene.h"
#include <stdio.h>
#include <stdlib.h>


// Инициализация 
bool mesh_init(Mesh* mesh) {
    // Создаем VAO, VBO, EBO 
    glGenVertexArrays(1, &mesh->VAO);
    glGenBuffers(1, &mesh->VBO);
    glGenBuffers(1, &mesh->EBO);
    
    mesh->hasTexture = true;
    mesh->textureType = TEXTURE_STICKER;
    
    return true;
}

void mesh_bind(Mesh* mesh) {
    glBindVertexArray(mesh->VAO);
}

void mesh_unbind(void) {
    glBindVertexArray(0);
}

void mesh_draw(Mesh* mesh) {
    mesh_bind(mesh);
    
    // Если у меша есть текстура, то подключаемся
    if (mesh->hasTexture) {
        Texture* texture = texture_get(mesh->textureType);
        if (texture) {
            texture_bind(texture, 0);
        }
    }
    
    if (mesh->indexCount > 0) {
        glDrawElements(mesh->drawMode, mesh->indexCount, GL_UNSIGNED_INT, 0);
    } else {
        glDrawArrays(mesh->drawMode, 0, mesh->vertexCount);
    }
    
    // Отключить текстуру после подключения
    if (mesh->hasTexture) {
        texture_unbind(0);
    }
    
    mesh_unbind();
}

void mesh_destroy(Mesh* mesh) {
    glDeleteVertexArrays(1, &mesh->VAO);
    glDeleteBuffers(1, &mesh->VBO);
    glDeleteBuffers(1, &mesh->EBO);
    
    mesh->vertexCount = 0;
    mesh->indexCount = 0;
}   

Mesh create_custom_colored_cube(unsigned int visibleFaces, RGBColor (*cubeColors)[9], int x, int y, int z) {
    Mesh mesh;
    mesh_init(&mesh);
    
    // Черный для невидимых граней
    float black[3] = {0.05f, 0.05f, 0.05f};
    
    // Определение цвета на основании видимых граней и цветов из сохраненных
    RGBColor frontColor = (visibleFaces & FACE_FRONT) ? 
        cubeColors[FACE_IDX_FRONT][x + (2-y)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor backColor = (visibleFaces & FACE_BACK) ? 
        cubeColors[FACE_IDX_BACK][(2-x) + (2-y)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor topColor = (visibleFaces & FACE_TOP) ? 
        cubeColors[FACE_IDX_TOP][x + (2-z)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor bottomColor = (visibleFaces & FACE_BOTTOM) ? 
        cubeColors[FACE_IDX_BOTTOM][x + z*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor rightColor = (visibleFaces & FACE_RIGHT) ? 
        cubeColors[FACE_IDX_RIGHT][z + (2-y)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    RGBColor leftColor = (visibleFaces & FACE_LEFT) ? 
        cubeColors[FACE_IDX_LEFT][(2-z) + (2-y)*3] : 
        (RGBColor){black[0], black[1], black[2]};
    
    // Координаты (x,y,z), Цвет (r,g,b), Углы текстуры
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
    
    
    // Треугольные полигоны
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
    
    mesh.vertexCount = 24;  // 6 граней, 4 вершины на грань
    mesh.indexCount = 36;   // 12 полигонов, по 2 на грань
    mesh.drawMode = GL_TRIANGLES;
    mesh.hasTexture = true;
    
    // Вид текстуры будет определен во время создания куба в сцене
    mesh.textureType = TEXTURE_STICKER; 
    
    // Подключаемся к буферу VAO
    glBindVertexArray(mesh.VAO);
    
    // Подключаемся и задаем позиции вершин
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    
    // Подключаемся и задаем позиции полигонов
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    
    // Конфигурация позиции координат в нашей структуре
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    // Вклочаем layout (location = 0) для использования в шейдерах для обращения к координатам
    glEnableVertexAttribArray(0);
    
    // Конфигурация позиции цвета в нашей структуре
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // Конфигурация позиции текстуры в нашей структуре
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // Отключение от буферов
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    
    return mesh;
}