#version 330

in vec2 fragTexCoord;
in vec4 fragColor;

uniform sampler2D texture0;

out vec4 finalColor;

void main()
{
    // Use UV coordinates directly as colors
    // U -> Red channel, V -> Green channel
    vec3 uvColor = vec3(fragTexCoord.x, fragTexCoord.y, 0.5);

    // Optional: Add grid lines for better visualization
    vec2 grid = abs(fract(fragTexCoord * 8.0) - 0.5) / fwidth(fragTexCoord * 8.0);
    float line = min(grid.x, grid.y);
    float gridMask = 1.0 - min(line, 1.0);

    // Blend UV color with grid
    uvColor = mix(uvColor, vec3(1.0), gridMask * 0.3);

    finalColor = vec4(uvColor, fragColor.a);
}
