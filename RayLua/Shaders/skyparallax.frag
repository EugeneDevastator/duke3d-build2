#version 330

// Fragment shader
in vec4 fragColor;
in vec3 worldPos;
uniform vec3 cameraPosition;
uniform vec3 cameraTarget;
uniform vec3 cameraUp;
uniform sampler2D textureSampler;

 int parallaxMode; // 0 = cylinder, 1 = linear
 float panX;
 float panY;
 float scaleX;
 float scaleY;
uniform int useGradient;

out vec4 finalColor;

void main() {
    // Calculate view direction from camera to current pixel
    vec3 viewDir = normalize(worldPos - cameraPosition);
    panX=0;
    panY=0;
    scaleX=1;
    scaleY=1;
    vec2 uv;

    if (parallaxMode == 0) {
        // Cylinder mode - full 360 degree wrap
        float angle = atan(viewDir.x, viewDir.z);
        uv.x = angle / (2.0 * 3.14159265) + 0.5;
        uv.y = viewDir.y * 0.5 + 0.5;
    } else {
        // Linear mode - no vertical curvature
        uv.x = viewDir.x * 0.5 + 0.5;
        uv.y = viewDir.y * 0.5 + 0.5;
    }

    // Apply pan and scale
    uv.x = uv.x * scaleX + panX;
    uv.y = uv.y * scaleY + panY;

    vec4 texColor = texture(textureSampler, fract(uv));

    if (useGradient == 1) {
        float luminance = (texColor.r + texColor.g + texColor.b) / 3.0;
        luminance *= fragColor.a;

        vec3 result;
        if (luminance <= 0.5) {
            result = fragColor.rgb * (luminance * 2.0);
        } else {
            result = mix(fragColor.rgb, vec3(1.0), (luminance - 0.5) * 2.0);
        }

        finalColor = vec4(result, texColor.a * fragColor.a);
    } else {
        finalColor = vec4(texColor * fragColor);
    }
   // finalColor = vec4(uv,0,1);
}
