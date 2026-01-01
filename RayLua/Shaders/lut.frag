#version 330

in vec2 fragTexCoord;
uniform sampler2D texture0;    // Your rendered scene
uniform sampler2D lutTexture;  // LUT texture (256x16)
uniform float lutIntensity;    // Blend factor

out vec4 finalColor;

vec3 applyLUT(vec3 color, sampler2D lut) {
    float lutSize = 16.0;

    // Clamp input color
   // color = clamp(color, 0.0, 1.0);

    // Calculate LUT coordinates
    float blueX = floor(( (color.b * 255.999) / lutSize)) / lutSize;  // horizontally lut is broken into 16 quads increasing in blue
    float r =  (color.r / lutSize);
    vec2 lutCoord = vec2(
    blueX + r, // r is increased per quad horisontally.
    color.g // g is along vertical axis.
    );
    return texture(lut, lutCoord).rgb;
}

void main() {
    vec3 color = texture(texture0, fragTexCoord).rgb;
    vec3 lutColor = applyLUT(color, lutTexture);
    finalColor = vec4(lutColor,1);//vec4(mix(color, lutColor, lutIntensity), 1.0);
   // finalColor = vec4(texture(lutTexture, fragTexCoord).rgb,1);//vec4(mix(color, lutColor, lutIntensity), 1.0);
}
