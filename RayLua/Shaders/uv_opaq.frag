// uv_opaq.frag
#version 330

in vec4 fragColor;
in vec2 uvPosition;

out vec4 finalColor;

// Declare a uniform for the texture
uniform sampler2D textureSampler;

void main() {
    // Sample the texture using UV coordinates
    vec4 texColor = texture(textureSampler, fract((uvPosition)));

    // Output the sampled texture color
    finalColor = texColor;

    // Alternatively, you can blend with the original color visualization:
    // finalColor = texColor * fragColor;

    // Or keep your UV visualization for debugging:
    // vec2 uvColor = fract(uvPosition);
    // finalColor = vec4(uvColor.x, uvColor.y, 0.0, 1);
}
