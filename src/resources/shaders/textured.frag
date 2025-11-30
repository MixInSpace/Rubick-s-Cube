#version 330 core
out vec4 FragColor;

in vec3 vertexColor;
in vec2 texCoord;

uniform sampler2D textureMap;
uniform bool useTexture;

void main() {
    if (useTexture) {
        FragColor = texture(textureMap, texCoord) * vec4(vertexColor, 1.0);
    } else {
        FragColor = vec4(vertexColor, 1.0);
    }
} 