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
    vec4 waterTexColor = texture(waterTexture, texCoord)  * texWeights.x;
    vec4 snowTexColor = texture(snowTexture, texCoord) * texWeights.y;
    vec4 grassTexColor = texture(grassTexture, texCoord) * texWeights.z;
     
    outputColor = lightFactor * (grassTexColor + snowTexColor + waterTexColor);

    if (hasFog){
        vec4 fogColor = vec4(255/242, 255/248, 255/247, 1);
        outputColor = mix(outputColor, fogColor, fogFactor);
    }
    
}
