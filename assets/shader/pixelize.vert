#version 330 core
// x, y, z
layout(location = 0) in vec3 Position_in;
// x, y, z
layout(location = 1) in vec3 Normal_in;
// u, v
layout(location = 2) in vec2 TextureCoordinate_in;
// Hint: Gouraud shading calculates per vertex color, interpolate in fs
// You may want to add some out here
out vec3 rawPosition;
out vec2 TextureCoordinate;
out float lightIntensity;

// Uniform blocks
// https://www.khronos.org/opengl/wiki/Interface_Block_(GLSL)

layout (std140) uniform model {
  // Model matrix
  mat4 modelMatrix;
  // mat4(inverse(transpose(mat3(modelMatrix)))), precalculate using CPU for efficiency
  mat4 normalMatrix;
};

layout (std140) uniform camera {
  // Camera's projection * view matrix
  mat4 viewProjectionMatrix;
  // Position of the camera
  vec4 viewPosition;
};

layout (std140) uniform light {
  // Light's projection * view matrix
  // Hint: If you want to implement shadow, you may use this.
  mat4 lightSpaceMatrix;
  // Position or direction of the light
  vec4 lightVector;
  // inner cutoff, outer cutoff, isSpotlight, isDirectionalLight
  vec4 coefficients;
};

uniform float ambient;
uniform float ks;
uniform float kd;

void main() {
  TextureCoordinate = TextureCoordinate_in;
  rawPosition = mat3(modelMatrix) * Position_in;

  gl_Position = viewProjectionMatrix * modelMatrix * vec4(Position_in, 1.0);

  float diffuse, specular;
  vec3 lightDir  = normalize(lightVector.xyz);
  vec3 viewDir   = normalize(viewPosition.xyz - rawPosition);
  vec3 normalDir = normalize(mat3(normalMatrix) * Normal_in);

  if (coefficients[3] == 1){
    diffuse  = kd * max(dot(normalDir, lightDir), 0);
    specular = ks * pow(max(dot(normalDir, normalize(lightDir + viewDir)), 0), 8);
    lightIntensity = ambient + 0.65 * (diffuse + specular);
  }
  else{
    if (coefficients[2] == 1){
      float cameraDist  = distance(rawPosition, viewPosition.xyz);
      float attenuation = 1 / (1 + 0.014 * cameraDist + 0.007 * pow(cameraDist, 2));
      float intensity   = clamp((dot(-lightDir, viewDir) - coefficients.y) / (coefficients.x - coefficients.y), 0, 1);

      diffuse  = kd * max(dot(normalDir, viewDir), 0);
      specular = ks * pow(max(dot(normalDir, viewDir), 0), 8);
      lightIntensity = ambient + attenuation * intensity * (diffuse + specular);
    }
    else{
      float cameraDist  = distance(rawPosition, lightVector.xyz);
      float attenuation = 1 / (1 + 0.027 * cameraDist + 0.0028 * pow(cameraDist, 2));

      lightDir = normalize(lightVector.xyz - rawPosition);
      diffuse  = kd * max(dot(normalDir, lightDir), 0);
      specular = ks * pow(max(dot(normalDir, normalize(lightDir + viewDir)), 0), 8);
      lightIntensity = ambient + attenuation * (diffuse + specular);
    }
  }
}
