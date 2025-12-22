#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;  // Add this

uniform vec3 lightPosition;
uniform float lightRange;

out vec4 finalColor;

void main()
{
    float distance = length(lightPosition - fragPosition);
    float attenuation = 1.0 - clamp(distance/22, 0.0, 1.0);
    float intensity = attenuation * attenuation;

    finalColor = vec4(fragColor.rgb*5, intensity);
}
