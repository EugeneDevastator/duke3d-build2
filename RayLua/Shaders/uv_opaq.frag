// uv_opaq.frag
#version 330

in vec4 fragColor;
in vec3 uvPosition;

out vec4 finalColor;

void main() {
    // Use UV position as color for visualization
    vec3 uvColor = normalize(abs(uvPosition)) * 0.8 + 0.2;
    finalColor = vec4(uvColor * fragColor.rgb, fragColor.a);
}
