// uv_opaq.frag
#version 330

in vec4 fragColor;
in vec2 uvPosition;
uniform int  useGradient;
out vec4 finalColor;

// Declare a uniform for the texture
uniform sampler2D textureSampler;

void main() {
    vec4 texColor = texture(textureSampler, fract((uvPosition)));
if (useGradient==1) {
    // Sample the texture using UV coordinates

    //texColor.a = 0.8f;
    // Output the sampled texture color

    // Convert original RGB to luminance (0.0 to 1.0)
    // Divide by 3.0 since max sum is 3.0
    float luminance = (texColor.r + texColor.g + texColor.b) / 3.0;
    luminance *= (fragColor.a);

    // easy contrast adjust with powering 1.
    //luminance = pow(luminance, 3);

    //inversion for pal 6, lookin hot
    //luminance = 1.0-luminance;

    // can also multiply lum.
    //luminance*=2;

    // Create gradient: 0 (black) -> fragColor -> 1 (white)
    vec3 result;
    if (luminance <= 0.5) {
        // 0 to 0.5: interpolate from black to fragColor
        result = fragColor.rgb * (luminance * 2.0);
    } else {
        // 0.5 to 1.0: interpolate from fragColor to white
        result = mix(fragColor.rgb, vec3(1.0), (luminance - 0.5) * 2.0);
    }

    finalColor = vec4(result, texColor.a * fragColor.a);
    } else {
        // Standard texture * color
        finalColor = vec4(texColor * fragColor);
    }
   // finalColor = vec4(result, texColor.a * fragColor.a);
    // Alternatively, you can blend with the original color visualization:
    // finalColor = texColor * fragColor;

    // Or keep your UV visualization for debugging:
   // vec2 uvColor = fract(uvPosition);
   // finalColor = vec4(uvColor.x, uvColor.y, 0.0, 1);
}
