#version 330

uniform sampler2D waterTexture;
uniform sampler2D grassTexture;
uniform sampler2D snowTexture;

uniform bool isWireframe;

in vec4 texWeights;
in vec2 texCoord;
in vec4 lightColour;
in float fogColour;

void main()
{
    vec4 waterTexColor = texture(waterTexture, texCoord) * texWeights.x;
    vec4 snowTexColor = texture(snowTexture, texCoord) * texWeights.y;
    vec4 grassTexColor = texture(grassTexture, texCoord) * texWeights.z;
    
    if (isWireframe){
        gl_FragColor = vec4(0, 0, 0, 1);
    }else{
        gl_FragColor = lightColour *(waterTexColor + grassTexColor + snowTexColor);
        //vec4 baseColor = vec4(0.3);
        //gl_FragColor = mix(baseColor, lightColour *(waterTexColor + grassTexColor + snowTexColor), fogColour);
    }
}
