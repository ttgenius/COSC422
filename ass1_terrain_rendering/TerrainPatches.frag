#version 330

uniform sampler2D waterTexture;
uniform sampler2D grassTexture;
uniform sampler2D snowTexture;

uniform bool isWireframe;
uniform bool hasFog;

in vec4 texWeights;
in vec2 texCoord;
in float lightFactor;
in float fogFactor;

out vec4 outputColor;

void main()
{
    vec4 waterTexColor = texture(waterTexture, texCoord) * texWeights.x;
    vec4 snowTexColor = texture(snowTexture, texCoord) * texWeights.y;
    vec4 grassTexColor = texture(grassTexture, texCoord) * texWeights.z;
    
  
    outputColor = vec4(lightFactor) * (waterTexColor + grassTexColor + snowTexColor);
    if (hasFog){
        vec4 baseColor = vec4(0.3);
        outputColor = mix(baseColor, outputColor, fogFactor);
    }
    
}
