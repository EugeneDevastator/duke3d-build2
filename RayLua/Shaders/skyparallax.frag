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
    vec2 uv;
    float fov = 1.4;//rads
    panX=0;
    panY=0;
    scaleX=6;
    scaleY=1;
    parallaxMode=0;

    // Calculate camera basis vectors
    vec3 forward = normalize(cameraTarget - cameraPosition);
    vec3 right = normalize(cross(forward, cameraUp));
    vec3 up = normalize(cross(right, forward));

    // Get NDC coordinates using derivatives (0 to 1 range)
    vec2 ndc = gl_FragCoord.xy * fwidth(gl_FragCoord.xy);
    ndc = fract(ndc); // This gives us 0-1 normalized coords

    // Alternative: project viewDir onto screen plane
    vec3 viewDir = normalize(worldPos - cameraPosition);
    float rightComp = dot(viewDir, right);
    float upComp = dot(viewDir, up);
    float forwardComp = dot(viewDir, forward);

    // Convert to screen space (-0.5 to 0.5)
    vec2 screenSpace = vec2(rightComp, upComp) / forwardComp;
    ndc = screenSpace + 0.5; // Convert to 0-1

    if (parallaxMode == 0) {
        // Your approach
        float hAngleDiff = mix(0,fov,1-ndc.x) - fov*0.5; // Scale by FOV

        float forwardHorizontal = atan(forward.x, forward.z);
        float worldAngle = forwardHorizontal + hAngleDiff;
        float ysample = -1*viewDir.y;
      //  ysample *= ysample*ysample;
      //  ysample = 1-ysample;
      //  ysample = mix(ysample,1,ndc.y);
        uv.x = (worldAngle / (2*3.14159265)) + 0.5;
        uv.y = ysample * 0.5 + 0.5;
    }  else if (parallaxMode == 2) // spherical
    {
        float angle = atan(viewDir.x, viewDir.z);
        uv.x = angle / (2.0 * 3.14159265) + 0.5;
        uv.y = viewDir.y * 0.5 + 0.5;
    }
    else {
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
