#version 330 core
layout(location = 0) out vec4 FragColor;

in vec2 TextureCoordinate;
in vec3 rawPosition;
in vec4 position;
in vec3 normal;

uniform sampler2D diffuseTexture;
uniform samplerCube diffuseCubeTexture;
uniform int isCube;
uniform float ambient;
uniform float ks;
uniform float kd;

layout (std140) uniform camera {
    // Projection * View matrix
    mat4 viewProjectionMatrix;
    // Position of the camera
    vec4 viewPosition;
};

layout (std140) uniform light {
    // Projection * View matrix
    mat4 lightSpaceMatrix;
    // Position or direction of the light
    vec4 lightVector;
    // inner cutoff, outer cutoff, isSpotlight, isDirectionalLight
    vec4 coefficients;
};

float pointLightFactor();

void main() {
    vec4 diffuseTextureColor = texture(diffuseTexture, TextureCoordinate);
    vec4 diffuseCubeTextureColor = texture(diffuseCubeTexture, rawPosition);
    vec3 baseColor = isCube == 1 ? diffuseCubeTextureColor.rgb : diffuseTextureColor.rgb;
    
    float factor = pointLightFactor();
    vec3 color = baseColor * factor;
    FragColor = vec4(color, 1);
}

float pointLightFactor() {
    // Diffuse
    vec3 lightDir = normalize(lightVector.xyz - position.xyz);
    float diffuse = 0.75 * max(dot(normal, lightDir), 0.0);
    // Specular
    vec3 viewDir = normalize(viewPosition.xyz - position.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specular = 0.75 * pow(max(dot(normal, halfwayDir), 0.0), 8.0);
    // Attenuation
    float dist = length(lightVector.xyz - position.xyz);
    float attenuation = 1.0f / (1 + 0.027 * dist + 0.0028 * dist * dist);

    return 0.1 + attenuation * (diffuse + specular);
}
