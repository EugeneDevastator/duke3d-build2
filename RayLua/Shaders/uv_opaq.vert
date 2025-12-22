 // uv_opaq.vert
#version 330

in vec3 vertexPosition;
in vec3 vertexTexCoord;
in vec4 vertexColor;

uniform mat4 mvp;
uniform vec3 worldOrigin;
uniform vec3 worldU;
uniform vec3 worldV;

out vec4 fragColor;
out vec3 uvPosition;

void main() {
    // Transform UV coordinates to world space
    // vertexTexCoord encodes xyz in same world space, so we need to get 2d out of
    // vertexPosition, worldorigin, worldU and worldV vectors;
    vec3 worldPos = vertexPosition;  // Assuming this is your world space vertex position
    vec3 localPos = worldPos - worldOrigin;

    // Project onto UV plane using dot products
    float u = dot(localPos, normalize(worldU)) / length(worldU);
    float v = dot(localPos, normalize(worldV)) / length(worldV);

    uvPosition = vec2(u, v);

    gl_Position = mvp * vec4(vertexPosition, 1.0);
}
