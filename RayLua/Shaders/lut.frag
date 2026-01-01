// Fragment shader for LUT (save as lut.fs)
#version 330

in vec2 fragTexCoord;
uniform sampler2D texture0;    // Your rendered scene
uniform sampler2D lutTexture;  // LUT texture (256x16 or 512x512)
uniform float lutIntensity;    // Blend factor

out vec4 finalColor;

vec3 applyLUT(vec3 color, sampler2D lut) {
    float lutSize = 16.0; // For 256x16 LUT

    float scale = (lutSize - 1.0) / lutSize;
    float offset = 1.0 / (2.0 * lutSize);

    color = clamp(color, 0.0, 1.0);

    float blue = color.b * scale + offset;
    vec2 quad1;
    quad1.y = floor(floor(blue * lutSize) / lutSize);
    quad1.x = floor(blue * lutSize) - (quad1.y * lutSize);

    vec2 quad2;
    quad2.y = floor(ceil(blue * lutSize) / lutSize);
    quad2.x = ceil(blue * lutSize) - (quad2.y * lutSize);

    vec2 texPos1;
    texPos1.x = (quad1.x * scale) + offset + (scale * color.r);
    texPos1.y = (quad1.y * scale) + offset + (scale * color.g);

    vec2 texPos2;
    texPos2.x = (quad2.x * scale) + offset + (scale * color.r);
    texPos2.y = (quad2.y * scale) + offset + (scale * color.g);

    vec3 newColor1 = texture(lut, texPos1).rgb;
    vec3 newColor2 = texture(lut, texPos2).rgb;

    vec3 newColor = mix(newColor1, newColor2, fract(blue * lutSize));
    return newColor;
}

void main() {
    vec3 color = texture(texture0, fragTexCoord).rgb;
    vec3 lutColor = applyLUT(color, lutTexture);
    finalColor = vec4(mix(color, lutColor, lutIntensity), 1.0);
}
