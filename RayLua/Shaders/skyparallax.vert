#version 330

// Vertex shader
in vec3 vertexPosition;
in vec3 vertexNormal;
in vec3 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform vec3 cameraPosition;
uniform vec3 cameraTarget;
uniform vec3 cameraUp;

out vec4 fragColor;
out vec3 viewDir;
out vec3 worldPos;

void main() {
    fragColor = vertexColor;
    worldPos = vertexPosition; // Pass world position to fragment
    gl_Position = mvp * vec4(vertexPosition, 1.0);
}

