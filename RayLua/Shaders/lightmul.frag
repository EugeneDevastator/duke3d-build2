#version 330

in vec2 fragTexCoord;
uniform sampler2D albedoTexture;
uniform sampler2D lightTexture;
out vec4 finalColor;

void main() {
    vec3 albedo = texture(albedoTexture, fragTexCoord).rgb;
    vec3 light = texture(lightTexture, fragTexCoord).rgb;
    // Proper ambient + diffuse lighting
    vec3 ambient = albedo * 0.2;
    float blend = 0.4;
    vec3 diffuse_foggy = light; // sum
    vec3 diffuse_real = albedo * light; // mult
    vec3 diffuse = mix(diffuse_real,diffuse_foggy,blend);

    finalColor = vec4(ambient + diffuse, 1.0);
}

