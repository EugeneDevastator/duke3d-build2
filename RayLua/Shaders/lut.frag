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

    // Scale to LUT coordinate space
    vec3 scaledColor = color * (lutSize - 1.0);

    // Get integer and fractional parts
    vec3 baseColor = floor(scaledColor);
    vec3 fracColor = scaledColor - baseColor;

    // Calculate 2D coordinates for 3D LUT
    // Standard layout: blue slices arranged horizontally
    float slice0 = baseColor.b;
    float slice1 = min(slice0 + 1.0, lutSize - 1.0);

    // Calculate UV coordinates for both slices
    vec2 uv0 = vec2(
    (slice0 + baseColor.r / lutSize) / lutSize,
    baseColor.g / lutSize
    );

    vec2 uv1 = vec2(
    (slice1 + baseColor.r / lutSize) / lutSize,
    baseColor.g / lutSize
    );

    // Sample both slices and interpolate
    vec3 color0 = texture(lut, uv0).rgb;
    vec3 color1 = texture(lut, uv1).rgb;

    return mix(color0, color1, fracColor.b);
}

void main() {
    vec3 color = texture(texture0, fragTexCoord).rgb;
    vec3 lutColor = applyLUT(color, lutTexture);
    finalColor = vec4(lutColor, 1.0);
}
