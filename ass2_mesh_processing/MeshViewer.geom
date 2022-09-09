#version 400

layout (triangles_adjacency) in;
layout (triangle_strip , max_vertices = 27) out;


uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat4 norMatrix;
uniform vec4 lightPos;

uniform vec2 creaseEdges;
uniform vec2 silhoutteEdges;

uniform float creaseThreshold;
float T = cos(creaseThreshold * 3.14159  / 180.0);

in vec3 unitNormal[];

flat out int isEdgeVertex;
out float diffTerm;  // l.n
out vec2 texCoords;


vec4 faceNormal;
float thresh;

vec4 getFaceNormal(int i1, int i2, int i3){
    vec3 v1 = gl_in[i1].gl_Position.xyz - gl_in[i3].gl_Position.xyz;
    vec3 v2 = gl_in[i2].gl_Position.xyz - gl_in[i3].gl_Position.xyz;

    return vec4(normalize(cross(v1, v2)), 0);
}



void drawSilhoutteEdge(vec4 a, vec4 b, vec4 n1, vec4 n2){
    vec4 v = normalize(n1 + n2);
    vec4 p1, p2, q1, q2;
    float d1 = silhoutteEdges[0];
    float d2 = silhoutteEdges[1];

/*
    if (enableOverlap == 1){
        vec4 a_to_b = 3 * normalize(b - a);
        vec4 b_to_a = 3 * normalize(a - b);
        p1 = (a + b_to_a) + d1 * v;
        p2 = (a + b_to_a) + d2 * v;
        q1 = (b + a_to_b) + d1 * v;
        q2 = (b + a_to_b) + d2 * v;
    } else {
*/
        p1 = a + d1 * v;
        p2 = a + d2 * v;
        q1 = b + d1 * v;
        q2 = b + d2 * v;
   // }

    isEdgeVertex = 1;
    
    gl_Position = mvpMatrix * p1;
    EmitVertex();
    
    gl_Position = mvpMatrix * p2;
    EmitVertex();
   
    gl_Position = mvpMatrix * q1;
    EmitVertex();
    
    gl_Position = mvpMatrix * q2;
    EmitVertex();
    
    EndPrimitive();
}



void drawCreaseEdge(vec4 a, vec4 b, vec4 n1, vec4 n2){
    vec4 u = normalize(b - a);
    vec4 v = normalize(n1 + n2);
    vec4 w = vec4(normalize(cross(vec3(u.xyz), vec3(v.xyz))), 0);

    float d1 = creaseEdges[0];
    float d2 = creaseEdges[1];

    vec4 p1 = a + d1 * v + d2 * w;
    vec4 p2 = a + d1 * v - d2 * w;
    vec4 q1 = b + d1 * v + d2 * w;
    vec4 q2 = b + d1 * v - d2 * w;
    
    isEdgeVertex = 1;
    
    gl_Position = mvpMatrix * p1;
    EmitVertex();
    
    gl_Position = mvpMatrix * p2;
    EmitVertex();
    
    gl_Position = mvpMatrix * q1;
    EmitVertex();
    
    gl_Position = mvpMatrix * q2;
    EmitVertex();
    
    EndPrimitive();
}


void drawEdge(int index){
    vec4 adjFaceNormal = getFaceNormal(index, (index + 1) % 6, (index + 2) % 6);
    //if (enableSil == 1) {
        if ((mvMatrix * faceNormal).z > 0 && (mvMatrix * adjFaceNormal).z < 0){
            drawSilhoutteEdge(gl_in[index].gl_Position, gl_in[(index + 2) % 6].gl_Position, faceNormal, adjFaceNormal);
        }
   // }

   // if (enableCrease == 1) {
        if (dot(faceNormal, adjFaceNormal) < T){
            drawCreaseEdge(gl_in[index].gl_Position, gl_in[(index + 2) % 6].gl_Position, faceNormal, adjFaceNormal);
        }
   // }
}


void main()
{
	faceNormal = getFaceNormal(0, 2, 4);
    for (int i = 0; i < gl_in.length(); i += 2) //for each vertex in main triangle (every 2nd)
	{
	  vec4 posn = gl_in[i].gl_Position;
	  vec4 posnEye = mvMatrix * posn;
	  vec3 lgtVec = normalize(lightPos.xyz - posnEye.xyz); 

	  diffTerm = max(dot(lgtVec, unitNormal[i]), 0.2);

	  if (i == 0){
			texCoords = vec2(0.0, 0.0);
	  }
	  else if (i == 2) {
			texCoords = vec2(1.0, 0.5);
	  } 
	  else if (i == 4) {
			texCoords = vec2(0.0, 1.0);
	  }
	 
	  isEdgeVertex = 0;

	  gl_Position = mvpMatrix * posn;
	  EmitVertex();
	}
	EndPrimitive();

    for(int i = 0; i < gl_in.length(); i+=2)
    {
      
        drawEdge(i);
        
    }
}