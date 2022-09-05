#version 400

layout (triangles_adjacency) in;
layout (triangle_strip , max_vertices = 27) out;


uniform mat4 mvMatrix;
uniform mat4 mvpMatrix;
uniform mat4 norMatrix;
uniform vec4 lightPos;

out float diffTerm;  // l.n
out vec2 tex_coords;
out float normals[4];  // 4 faces to compare from