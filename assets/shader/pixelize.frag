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

  vec3 color;

  if (isCube == 0){
    vec2  texSize = textureSize(diffuseTexture, 0).xy;
    float wx = 100.0 / texSize.x;
    float wy = 100.0 / texSize.y;
    vec2  newCoordinate = vec2(wx * floor(TextureCoordinate.x / wx), 
                               wy * floor(TextureCoordinate.y / wy));
    color = texture(diffuseTexture, newCoordinate).rgb;

  }else{
    int   maxAbsInd;
    float maxAbs, tempAbs;

    maxAbs = -1.0;
    for (int i = 0; i < 3; i++){
      tempAbs = abs(rawPosition[i]);
      if (tempAbs > maxAbs) { 
        maxAbs = tempAbs;
        maxAbsInd = i;
      }
    }

    float texSize = textureSize(diffuseCubeTexture, 0).x;
    float w = 10.0 / texSize;

    vec3  scaledPosition = rawPosition / maxAbs;
    float scaledMaxAbs   = scaledPosition[maxAbsInd];

    scaledPosition = w * floor(scaledPosition / w);
    scaledPosition[maxAbsInd] = scaledMaxAbs;

    color = texture(diffuseCubeTexture, scaledPosition).rgb;
  }

  FragColor = vec4(color, 1.0) * lightIntensity;
}
