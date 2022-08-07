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


out vec3 texWeights;
out vec2 texCoord;
out vec4 lightColour;
out float fogColour;



void main()
{
    float xmin = -45, xmax = +45, zmin = 0, zmax = -100;
	float dmax = 5;
	float fogLevel = 0.02;
	float fogGradient = 1.3;

	vec4 posn[3];

	for (int i = 0; i < 3; i++) 
    {
        posn[i] = gl_in[i].gl_Position;
        if (posn[i].y < waterLevel){
            posn[i].y = waterLevel;
		}
    }

	vec3 u = posn[1].xyz - posn[0].xyz;
	vec3 v = posn[2].xyz - posn[0].xyz;
	vec4 normal = vec4(normalize(cross(u, v)), 0);
    
    for (int i=0; i< gl_in.length(); i++)
	{
		vec4 oldPos = gl_in[i].gl_Position;
		float ambient = 0.3;

        if (posn[i].y == waterLevel){
            texWeights = vec3(1, 0, 0);  
			ambient = 0.7;

        }else if (posn[i].y > snowLevel){
            texWeights = vec3(0, 1, 0);    
        }else{
            texWeights = vec3(0, 0, 1);
        }
		
		//lighting
		vec4 posnEye = mvMatrix * posn[i];
		vec4 normalEye = norMatrix * normal;
		vec4 lgtVec = normalize(lightPos - posnEye); 
		vec4 viewVec = normalize(vec4(-posnEye.xyz, 0)); 		
		vec4 halfVec = normalize(lgtVec + viewVec); 
			
		float diffuse = max(dot(lgtVec, normalEye), 0);   
		
		float shininess = 100.0;
		vec4 white = vec4(1.0);
		float specTerm = max(dot(halfVec, normalEye), 0);
		vec4 specular = white *  pow(specTerm, shininess) * texWeights.x ;
		
		//water depth variation
		float depth = posn[i].y - oldPos.y;
        float depthScale = depth / dmax;

		lightColour = vec4(min(ambient + diffuse + specular - depthScale, 1.0));

		if (hasFog){
		    fogColour = exp(-pow(length(posn[i] * fogLevel), fogGradient));
            fogColour = clamp(fogColour, 0.0, 1.0);
        } else{
            fogColour = 1.0;
		}
		
		texCoord.s = (posn[i].x - xmin) / (xmax - xmin);
		texCoord.t = (posn[i].z - zmin) / (zmax - zmin);

		gl_Position = mvpMatrix * posn[i];
		EmitVertex();	
	}
	EndPrimitive();
}

