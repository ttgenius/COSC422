#version 330

uniform int wireMode;
uniform sampler2D texSample[3];
uniform bool NPRMode;
uniform bool multiTexture;


flat in int isEdgeVertex;
in vec2 texCoords;
in float diffTerm;

out vec4 outputColor;

vec4 getTwoTone()
{
   vec4 twoToneColor;
   if (diffTerm < 0){
     twoToneColor = vec4(0.2, 0.2, 0.2, 1);   
   } else if (diffTerm > 0.7){
     twoToneColor = vec4(0, 1, 1, 1);
   }else{
     twoToneColor = vec4(0, 0.5, 0.5, 1);
   }
   return twoToneColor;
}


vec4 getPencilColor()
{
    vec4 penColor;
    if (diffTerm < 0.0){
            penColor = texture(texSample[2], texCoords);
    } else if (diffTerm > 0.7) {
             penColor = texture(texSample[0], texCoords);
    } else {
            if (multiTexture){
                if (diffTerm > 0.5){
                    penColor = mix(texture(texSample[1], texCoords), texture(texSample[0], texCoords), (diffTerm - 0.5) / 0.5);
                } else if (diffTerm < 0.3){
                    penColor = mix(texture(texSample[2], texCoords), texture(texSample[1], texCoords), (diffTerm - 0.2)/ 0.2);
                } else {
                    penColor = texture(texSample[1], texCoords);
                }
            } else {
                penColor = texture(texSample[1], texCoords);
            }     
    }
    return penColor;

}


void main() 
{
   if(wireMode == 1){    //Wireframe
       outputColor = vec4(0, 0, 1, 1);
   }
   else	{		//Fill + lighting
       if (isEdgeVertex == 1){    
         outputColor = vec4(0);
       }
         if (NPRMode){
           outputColor = getTwoTone();
         }
         else{
           outputColor = getPencilColor();
         }
         
   }
}
