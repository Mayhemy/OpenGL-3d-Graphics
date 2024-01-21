#version 420 core

out vec4 FragColor;
in vec2 TexCoord;
in vec3 WorldPos;
uniform vec3 color;
uniform vec3 viewPos;

layout (binding = 0) uniform sampler2D normalTexture;

float FogFactor(float d) {
    const float FogMax = 750.0;
    if (d>=FogMax) return 1;
    return 1 - (FogMax - d) / (FogMax);
}

void main() {

    // normal
    vec3 normalMap = texture2D(normalTexture, TexCoord).rgb;
    normalMap = vec3(normalMap.x, normalMap.z, normalMap.y);
    vec3 normal = normalize(normalMap * 2.0 - 1.0); 
    
    // ambient light
    vec3 terrainColor = vec3(0.31, 0.20, 0.08);
    float ambientStrength = 0.05;
    vec3 ambient = ambientStrength * terrainColor;
  	
    // diffuse color
    vec3 lightDir = normalize(vec3(0, 1, 2));
    float ndotl = clamp(dot(normal, lightDir), 0.0, 1.0);
    vec3 diffuse = ndotl * terrainColor;    
    vec3 lighting = ambient + diffuse;
  
    // fog
    float d = distance(viewPos, WorldPos);
    float alpha = FogFactor(d);
    vec3 FogColor = vec3(0.09, 0.11, 0.09);

    // final coor
    FragColor.rgb = mix(lighting, FogColor, alpha);
    FragColor.a = 1.0;

    // FragColor.rgb = vec3(normal);
}

