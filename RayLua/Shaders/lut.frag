#version 330

in vec2 fragTexCoord;
uniform sampler2D texture0;
uniform sampler2D lutTexture;

out vec4 finalColor;

vec3 applyLUT(vec3 color, sampler2D lut) {
    float lutSize = 32.0;
    float texelSize = 1.0 / lutSize;
    float texelSizeX = 1.0 / (lutSize * lutSize);

    // Clamp input to valid range
    color = clamp(color, 0.0, 1.0);

    // Scale to LUT index space (0 to lutSize-1)
    vec3 scaledColor = color * (lutSize - 1.0);

    // Get nearest indices
    vec3 index = floor(scaledColor + 0.5);

    // Ensure indices are within bounds
    index = clamp(index, 0.0, lutSize - 1.0);

    // Calculate 2D coordinates with proper texel centering
    float x = (index.r + index.b * lutSize) / (lutSize * lutSize) + texelSizeX * 0.5;
    float y = index.g / lutSize + texelSize * 0.5;

    vec2 uv = vec2(x, y);

    return texture(lut, uv).rgb;
}

void main() {
    vec3 color = texture(texture0, fragTexCoord).rgb;
    vec3 lutColor = applyLUT(color, lutTexture);
    finalColor = vec4(lutColor, 1.0);
}
