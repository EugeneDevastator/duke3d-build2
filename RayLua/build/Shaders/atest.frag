#version 330

// Input
in vec2 fragTexCoord;
in vec4 fragColor;

// Uniforms
uniform sampler2D texture0;
uniform vec4 colDiffuse;
//uniform float alphaThreshold;

// Output
out vec4 finalColor;

void main()
{
    float alphaThreshold = 0.8;
    vec4 texelColor = texture(texture0, fragTexCoord);
    vec4 color = texelColor * colDiffuse * fragColor;

    // Alpha test - discard if below threshold
    if (color.a < alphaThreshold)
    discard;

    finalColor = color;
}
