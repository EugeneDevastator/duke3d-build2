// uv_opaq.frag
#version 330

in vec4 fragColor;
in vec2 uvPosition;

out vec4 finalColor;

void main() {
    // Use UV position as color for visualization
    vec2 uvColor = fract(abs(uvPosition*10));
    finalColor = vec4(uvColor.x, uvColor.y, 0.2, 1);
}
