#version 400

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

uniform mat4 mvpMatrix;
uniform mat4 mvMatrix;
uniform mat4 norMatrix;
uniform vec4 lightPos;
uniform float waterLevel;
uniform float snowLevel;
uniform bool hasFog;
uniform float fogLevel;
uniform int waterWaveTick;

out vec3 texWeights;
out vec2 texCoord;
out float lightFactor;
out float fogFactor;


float getWaterWavaHeight(vec4 position)
{
    float d = waterLevel - position.y;
    float m = 0.2;  // wave height
    float waterFrequency = 1.0; 
    float y = m * sin(waterFrequency * (d - float(waterWaveTick)*0.1));

    return y;
}


void main()
{
    float xmin = -45, xmax = +45, zmin = 0, zmax = -100;
	float dmax = 5;
	float grassWithSnowLevel = 1;
	float fogGradient = 1.2;
	
	vec4 newPositions[3];

	for (int i = 0; i < gl_in.length(); i++) 
    {
        newPositions[i] = gl_in[i].gl_Position;
        if (newPositions[i].y < waterLevel){
            float waterWavaHeight = getWaterWavaHeight(newPositions[i]);
			newPositions[i].y = waterLevel + waterWavaHeight;
		}
    }
	
	//face noraml of a triangle
	vec3 u = newPositions[0].xyz - newPositions[2].xyz;
	vec3 v = newPositions[1].xyz - newPositions[2].xyz;
	vec4 normal = vec4(normalize(cross(u, v)), 0);
    
    for (int i=0; i< gl_in.length(); i++)
	{
		vec4 oldPos = gl_in[i].gl_Position; //unmodified water position

        //pass texWeights to frag shader for selecting correct texture
		if (oldPos.y < waterLevel){               //water, use oldPos because can't do float compare newPos == waterLevel  
            texWeights = vec3(1.0, 0.0, 0.0);
		}
	    else if (oldPos.y > snowLevel){
            texWeights = vec3(0.0, 1.0, 0.0);    //snow
        }
		else if (oldPos.y > (snowLevel - grassWithSnowLevel)){              //grass with snow
		    float grassWeight = (snowLevel - oldPos.y)/grassWithSnowLevel;
			float snowWeight = 1-grassWeight;
		    texWeights = vec3(0.0, snowWeight, grassWeight);
		}
		else{
            texWeights = vec3(0.0, 0.0, 1.0);  //grass
        }
		
		//lighting calculations
		vec4 lightPosn = vec4(-50, 50, 60, 1.0);

		//ambient
		float ambient = 0.2;
		
		//diffuse
		vec4 posnEye = newPositions[i];
		vec4 normalEye = normal;
		vec4 lgtVec = normalize(lightPosn - posnEye); 
		float diffuse = max(dot(lgtVec, normalEye), 0);   
		
		//specular
		float shininess = 100.0;
		vec4 white = vec4(1.0);
		vec4 viewVec = normalize(vec4(-posnEye.xyz, 0)); 		
		vec4 halfVec = normalize(lgtVec + viewVec); 
		float specular = max(dot(halfVec, normalEye), 0) * texWeights.x;
		
		//water depth variation
		float depth = newPositions[i].y - oldPos.y;
        float depthFactor = depth / dmax;

		//sum light, output to frag shader
		lightFactor = min(ambient + diffuse + specular - depthFactor, 1.0);
		
		
		//fog factor for mix in frag
		if (hasFog){
		    fogFactor = 1 - exp(-pow(length(newPositions[i] * fogLevel), fogGradient));
            fogFactor = clamp(fogFactor, 0.0, 1.0);
        } else{
            fogFactor = 0.0; //no fog
		}
		
		texCoord.s = (newPositions[i].x - xmin) / (xmax - xmin);
		texCoord.t = (newPositions[i].z - zmin) / (zmax - zmin);

		gl_Position = mvpMatrix * newPositions[i];
		EmitVertex();	
	}
	EndPrimitive();
}