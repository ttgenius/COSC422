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

out vec3 texWeights;
out vec2 texCoord;
out float lightFactor;
out float fogFactor;
out vec4 specularFactor;


void main()
{
    float xmin = -45, xmax = +45, zmin = 0, zmax = -100;
	float dmax = 5;
	float grassWithSnowLevel = 1;
	float fogGradient = 1.2;
	float ambient = 0.2;

	vec4 posn[3];

	for (int i = 0; i < gl_in.length(); i++) 
    {
        posn[i] = gl_in[i].gl_Position;
        if (posn[i].y < waterLevel){
            posn[i].y = waterLevel;
		}
    }

	vec3 u = posn[0].xyz - posn[2].xyz;
	vec3 v = posn[1].xyz - posn[2].xyz;
	vec4 normal = vec4(normalize(cross(u, v)), 0);
    
    for (int i=0; i< gl_in.length(); i++)
	{
		vec4 oldPos = gl_in[i].gl_Position;

        if (oldPos.y < waterLevel){  //water
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
		
		vec4 lightPosn = vec4(-50, 50, 60, 1.0);

		//diffuse
		vec4 posnEye = posn[i];
		vec4 normalEye = normal;
		vec4 lgtVec = normalize(lightPosn - posnEye); 
		float diffuse = max(dot(lgtVec, normalEye), 0);   
		
		//specular
		float shininess = 100.0;
		vec4 white = vec4(1.0);
		vec4 viewVec = normalize(vec4(-posnEye.xyz, 0)); 		
		vec4 halfVec = normalize(lgtVec + viewVec); 
		float specTerm = max(dot(halfVec, normalEye), 0);
		vec4 specularFactor = white * pow(specTerm, shininess) ;
		
		//water depth variation
		float depth = posn[i].y - oldPos.y;
        float depthFactor = depth / dmax;

		//sum light
		lightFactor = min(ambient + diffuse - depthFactor, 1.0);
		
		if (hasFog){
		    fogFactor = 1-exp(-pow(length(posn[i] * fogLevel), fogGradient));
            fogFactor = clamp(fogFactor, 0.0, 1.0);
        } else{
            fogFactor = 0.0;
		}
		
		texCoord.s = (posn[i].x - xmin) / (xmax - xmin);
		texCoord.t = (posn[i].z - zmin) / (zmax - zmin);

		gl_Position = mvpMatrix * posn[i];
		EmitVertex();	
	}
	EndPrimitive();
}