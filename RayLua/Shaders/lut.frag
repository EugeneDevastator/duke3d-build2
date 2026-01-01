#version 330

in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform sampler2D lutTexture;
uniform float lutIntensity;

out vec4 finalColor;

vec3 applyLUT(vec3 color, sampler2D lut) {
    float lutSize = 16.0;

    // Clamp input to valid range
    color = clamp(color, 0.0, 1.0);

    // Scale to LUT coordinate space and floor everything for nearest sampling
    vec3 scaledColor = floor(color * (lutSize - 1.0));

    // Calculate 2D coordinates for 3D LUT - single slice only
    vec2 uv = vec2(
    (scaledColor.b + scaledColor.r / lutSize) / lutSize,
    scaledColor.g / lutSize
    );

    return texture(lut, uv).rgb;
}

void main() {
    vec3 color = texture(texture0, fragTexCoord).rgb;
    vec3 lutColor = applyLUT(color, lutTexture);
    finalColor = vec4(lutColor, 1.0);
}
