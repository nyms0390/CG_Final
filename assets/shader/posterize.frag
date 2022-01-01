#version 330 core
layout(location = 0) out vec4 FragColor;

// Data from vertex shader
in vec3 rawPosition;
in vec2 TextureCoordinate;
in float lightIntensity;

// Texture of object
uniform sampler2D diffuseTexture;
uniform samplerCube diffuseCubeTexture;
uniform int isCube;

void main() {
  vec4 diffuseTextureColor = texture(diffuseTexture, TextureCoordinate);
  vec4 diffuseCubeTextureColor = texture(diffuseCubeTexture, rawPosition);
  vec3 color = isCube == 1 ? diffuseCubeTextureColor.rgb : diffuseTextureColor.rgb;

  float nLevel   = 5;
  float grayVal  = max(color.r, max(color.g, color.b));

  float lowerVal  = floor(grayVal * nLevel) / nLevel;
  float lowerDiff = grayVal - lowerVal;

  float upperVal  = ceil(grayVal * nLevel) / nLevel;
  float upperDiff = upperVal - grayVal;

  float level = lowerDiff <= upperDiff ? lowerVal : upperVal;
  FragColor = vec4(color * level / grayVal, 1.0) * lightIntensity;
}
