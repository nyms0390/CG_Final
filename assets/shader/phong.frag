#version 330 core
layout(location = 0) out vec4 FragColor;

in vec2 TextureCoordinate;
in vec3 rawPosition;
in vec4 position;
in vec3 normal;

uniform sampler2D diffuseTexture;
uniform samplerCube diffuseCubeTexture;
// precomputed shadow
// Hint: You may want to uncomment this to use shader map texture.
// uniform sampler2DShadow shadowMap;
uniform int isCube;

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

float ambient = 0.1;
float ks = 0.75;
float kd = 0.75;

float pointLightFactor();
float dirLightFactor();
float spotLightFactor();

void main() {
    vec4 diffuseTextureColor = texture(diffuseTexture, TextureCoordinate);
    vec4 diffuseCubeTextureColor = texture(diffuseCubeTexture, rawPosition);
    vec3 color = isCube == 1 ? diffuseCubeTextureColor.rgb : diffuseTextureColor.rgb;
    
    bool isSpotLight = bool(coefficients.z);
    bool isDirLight = bool(coefficients.w);
    
    if (isSpotLight) color *= spotLightFactor();
    else if (isDirLight) color *= dirLightFactor();
    else color *= pointLightFactor();
    
    FragColor = vec4(color, 1.0);
}

float pointLightFactor() {
    // Diffuse
    vec3 lightDir = normalize(lightVector.xyz - position.xyz);
    float diffuse = kd * max(dot(normal, lightDir), 0.0);
    // Specular
    vec3 viewDir = normalize(viewPosition.xyz - position.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specular = ks * pow(max(dot(normal, halfwayDir), 0.0), 8.0);
    // Attenuation
    float dist = length(lightVector.xyz - position.xyz);
    float attenuation = 1.0f / (1 + 0.027 * dist + 0.0028 * dist * dist);
    
    return ambient + attenuation * (diffuse + specular);
}

float dirLightFactor() {
    // Diffuse
    vec3 lightDir = normalize(lightVector.xyz);
    float diffuse = kd * max(dot(normal, lightDir), 0.0);
    // Specular
    vec3 viewDir = normalize(viewPosition.xyz - position.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specular = ks * pow(max(dot(normal, halfwayDir), 0.0), 8.0);
    // Attenuation
    float attenuation = 0.65;
    
    return ambient + attenuation * (diffuse + specular);
}

float spotLightFactor() {
    // Diffuse
    vec3 lightDir = normalize(viewPosition.xyz - position.xyz);
    float diffuse = kd * max(dot(normal, lightDir), 0.0);
    // Specular
    vec3 viewDir = normalize(viewPosition.xyz - position.xyz);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specular = ks * pow(max(dot(normal, halfwayDir), 0.0), 8.0);
    
    float theta = dot(lightDir, normalize(-lightVector.xyz));
    float epsilon = coefficients.x - coefficients.y;
    float intensity = clamp((theta - coefficients.y) / epsilon, 0.0, 1.0);
    diffuse *= intensity;
    specular *= intensity;
    
    // Attenuation
    float dist = length(viewPosition.xyz - position.xyz);
    float attenuation = 1.0f / (1 + 0.014 * dist + 0.007 * dist * dist);
    
    return ambient + attenuation * (diffuse + specular);
}
