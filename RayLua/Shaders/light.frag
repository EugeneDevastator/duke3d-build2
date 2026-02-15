#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;

uniform vec3 lightPosition;
uniform vec3 lightForwardVec;
uniform float lightRange;

out vec4 finalColor;

void main()
{
    // pillar func. - just alter distance calculations.
    //  lp.y=0;    fp.y=0;    float distance = length(lp - fp);
    //  lp.z=0;    fp.z=0;    float distance = length(lp - fp);
    // Hardcoded spot light parameters
    vec3 spotDirection = normalize(lightForwardVec);
    float arcAngle = 360.0;       // full cone angle in degrees
    float fadeAngle = 0.0;       // fade border in degrees

    // Direction from light to fragment
    vec3 lightToFrag = normalize(fragPosition - lightPosition);

    // Calculate angle between spot direction and light-to-fragment vector
    float cosAngle = dot(spotDirection, lightToFrag);
    float angle = degrees(acos(cosAngle));

    // Distance attenuation
    float distance = length(lightPosition - fragPosition);
    float distanceAttenuation = 1.0 - clamp(distance / (fragColor.a * 100.0), 0.0, 1.0);

    // Spot light cone calculation
    float halfConeAngle = arcAngle * 0.5;
    float spotAttenuation = 0.0;

    if (angle <= halfConeAngle) {
        if (angle <= halfConeAngle - fadeAngle) {
            spotAttenuation = 1.0;  // Full intensity inside cone
        } else {
            // Smooth fade at cone edge
            float fadeDistance = angle - (halfConeAngle - fadeAngle);
            spotAttenuation = 1.0 - (fadeDistance / fadeAngle);
        }
    }

    float intensity = distanceAttenuation * distanceAttenuation * spotAttenuation;

    finalColor = vec4(fragColor.rgb, intensity);
}
