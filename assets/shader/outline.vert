#version 330 core
layout(location = 0) in vec3 Position_in;
layout(location = 1) in vec3 Normal_in;
layout(location = 2) in vec2 TextureCoordinate_in;

out vec2 TextureCoordinate;
out vec3 rawPosition;
out vec4 position;
out vec3 normal;

layout (std140) uniform model {
    // Model matrix
    mat4 modelMatrix;
    // inverse(transpose(model)), precalculate using CPU for efficiency
    mat4 normalMatrix;
};

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

void main() {
    TextureCoordinate = TextureCoordinate_in;
    rawPosition = mat3(modelMatrix) * Position_in;
    position = modelMatrix * vec4(Position_in, 1.0);
    mat3 normalMatrix = mat3(transpose(inverse(modelMatrix)));
    normal = normalize(normalMatrix * Normal_in);
    
    gl_Position = viewProjectionMatrix * modelMatrix * vec4(Position_in, 1.0);
}
