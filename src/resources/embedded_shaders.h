#ifndef EMBEDDED_SHADERS_H
#define EMBEDDED_SHADERS_H

// Default vertex shader embedded as a string constant
static const char* DEFAULT_VERTEX_SHADER = 
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"out vec3 vertexColor;\n"
"\n"
"void main() {\n"
"    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"    vertexColor = aColor;\n"
"}\n";

// Default fragment shader embedded as a string constant
static const char* DEFAULT_FRAGMENT_SHADER = 
"#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"in vec3 vertexColor;\n"
"\n"
"void main() {\n"
"    FragColor = vec4(vertexColor, 1.0);\n"
"}\n";

// Textured vertex shader embedded as a string constant
static const char* TEXTURED_VERTEX_SHADER = 
"#version 330 core\n"
"layout (location = 0) in vec3 aPos;\n"
"layout (location = 1) in vec3 aColor;\n"
"layout (location = 2) in vec2 aTexCoord;\n"
"\n"
"uniform mat4 model;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"\n"
"out vec3 vertexColor;\n"
"out vec2 texCoord;\n"
"\n"
"void main() {\n"
"    gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
"    vertexColor = aColor;\n"
"    texCoord = aTexCoord;\n"
"}\n";

// Textured fragment shader embedded as a string constant
static const char* TEXTURED_FRAGMENT_SHADER = 
"#version 330 core\n"
"out vec4 FragColor;\n"
"\n"
"in vec3 vertexColor;\n"
"in vec2 texCoord;\n"
"\n"
"uniform sampler2D textureMap;\n"
"uniform bool useTexture;\n"
"\n"
"void main() {\n"
"    if (useTexture) {\n"
"        FragColor = texture(textureMap, texCoord) * vec4(vertexColor, 1.0);\n"
"    } else {\n"
"        FragColor = vec4(vertexColor, 1.0);\n"
"    }\n"
"}\n";

#endif /* EMBEDDED_SHADERS_H */ 