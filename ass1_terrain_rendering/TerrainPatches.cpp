//  ========================================================================
//  COSC363: Computer Graphics (2022);  University of Canterbury.
//  FILE NAME: TerrainPatches.cpp
//
//	The program generates and loads the mesh data for a terrain floor (100 verts, 81 elems).
//  This program requires the following files:
//         TerrainPatches.vert, TerrainPatches.frag
//         TerrainPatches.cont, TerrainPatches.eval
//  ========================================================================

#include <iostream>
#include <fstream>
#include <sstream>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "loadTGA.h" 
using namespace std;

GLuint vaoID;
GLuint texID[4];
GLuint theProgram;
GLuint mvpMatrixLoc, eyeLoc, mvMatrixLoc, norMatrixLoc, lightLoc;
float  eye_x = 0.0, eye_y = 40.0, eye_z = 30.0;      //Initial camera position
float look_x = 0.0, look_y = 0.0, look_z = -60.0;    //"Look-at" point along -z direction
float toRad = 3.14159265/180.0;     //Conversion from degrees to rad

float verts[100*3];       //10x10 grid (100 vertices)
GLushort elems[81*4];     //Element array for 9x9 = 81 quad patches

glm::mat4 projView;

float angle = 0;
float rotation = 0.1; //degree
float eye_x_max = 180.0, eye_x_min = -180.0;
float eye_z_max = 130.0, eye_z_min = -225.0;

GLuint fixCrackingLoc;
bool fixCracking = true;

GLuint waterLevelLoc;
float waterLevel = 2.0;

GLuint snowLevelLoc;
float snowLevel = 6;

GLuint wireframeLoc;
bool isWireframe = false;

GLuint fogLoc;
bool hasFog = false;

GLuint fogLevelLoc;
float fogLevel = 0.02;


//Generate vertex and element data for the terrain floor
void generateData()
{
	int indx, start;

	//verts array (100 vertices on a 10x10 grid)
	for (int j = 0; j < 10; j++)   //z-direction
	{
		for (int i = 0; i < 10; i++)  //x-direction
		{
			indx = 10 * j + i;
			verts[3 * indx] = 10 * i - 45;		//x
			verts[3 * indx + 1] = 0;			//y
			verts[3 * indx + 2] = -10 * j;		//z
		}
	}

	//elems array
	indx = 0;
	for (int j = 0; j < 9; j++)
	{
		for (int i = 0; i < 9; i++)
		{
			start = 10 * j + i;
			elems[indx] = start;
			elems[indx + 1] = start + 1;
			elems[indx + 2] = start + 11;
			elems[indx + 3] = start + 10;
			indx += 4;
		}
	}
}

//Loads height map
void loadTexture(string terrain_file)
{
	
    glGenTextures(4, texID);
	glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texID[0]);
	loadTGA(terrain_file);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, texID[1]);
	loadTGA("SurfaceTextures/Water.tga");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, texID[2]);
	loadTGA("SurfaceTextures/Grass.tga");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texID[3]);
	loadTGA("SurfaceTextures/Snow.tga");
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
}

//Loads a shader file and returns the reference to a shader object
GLuint loadShader(GLenum shaderType, string filename)
{
	ifstream shaderFile(filename.c_str());
	if(!shaderFile.good()) cout << "Error opening shader file." << endl;
	stringstream shaderData;
	shaderData << shaderFile.rdbuf();
	shaderFile.close();
	string shaderStr = shaderData.str();
	const char* shaderTxt = shaderStr.c_str();

	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderTxt, NULL);
	glCompileShader(shader);
	GLint status;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);
		const char *strShaderType = NULL;
		cerr <<  "Compile failure in shader: " << strInfoLog << endl;
		delete[] strInfoLog;
	}
	return shader;
}

//Initialise the shader program, create and load buffer data
void initialise()
{
//--------Load terrain height map-----------
	loadTexture("./HeightMaps/MtCook.tga");
//--------Load shaders----------------------
	GLuint shaderv = loadShader(GL_VERTEX_SHADER, "TerrainPatches.vert");
	GLuint shaderc = loadShader(GL_TESS_CONTROL_SHADER, "TerrainPatches.cont");
	GLuint shadere = loadShader(GL_TESS_EVALUATION_SHADER, "TerrainPatches.eval");
	GLuint shaderg = loadShader(GL_GEOMETRY_SHADER, "TerrainPatches.geom");
	GLuint shaderf = loadShader(GL_FRAGMENT_SHADER, "TerrainPatches.frag");

	GLuint program = glCreateProgram();
	glAttachShader(program, shaderv);
	glAttachShader(program, shaderc);
	glAttachShader(program, shadere);
	glAttachShader(program, shaderf);
	glAttachShader(program, shaderg);


	glLinkProgram(program);

	GLint status;
	glGetProgramiv (program, GL_LINK_STATUS, &status);
	if (status == GL_FALSE)
	{
		GLint infoLogLength;
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
		GLchar *strInfoLog = new GLchar[infoLogLength + 1];
		glGetProgramInfoLog(program, infoLogLength, NULL, strInfoLog);
		fprintf(stderr, "Linker failure: %s\n", strInfoLog);
		delete[] strInfoLog;
	}
	glUseProgram(program);

	mvpMatrixLoc = glGetUniformLocation(program, "mvpMatrix");
	mvMatrixLoc = glGetUniformLocation(program, "mvMatrix");
	norMatrixLoc = glGetUniformLocation(program, "norMatrix");
	eyeLoc = glGetUniformLocation(program, "eyePos");
	fixCrackingLoc = glGetUniformLocation(program, "fixCracking");
	wireframeLoc = glGetUniformLocation(program, "isWireframe");
	fogLoc = glGetUniformLocation(program, "hasFog");
	fogLevelLoc = glGetUniformLocation(program, "fogLevel");
	waterLevelLoc = glGetUniformLocation(program, "waterLevel");
	snowLevelLoc = glGetUniformLocation(program, "snowLevel");
	lightLoc = glGetUniformLocation(program, "ligtPos");

	GLuint texLoc = glGetUniformLocation(program, "heightMap");
	glUniform1i(texLoc, 0);

	GLuint waterTexLoc = glGetUniformLocation(program, "waterTexture");
	glUniform1i(waterTexLoc, 1);

	GLuint grassTexLoc = glGetUniformLocation(program, "grassTexture");
	glUniform1i(grassTexLoc, 2);

	GLuint snowTexLoc = glGetUniformLocation(program, "snowTexture");
	glUniform1i(snowTexLoc, 3);

//---------Load buffer data-----------------------
	generateData();

	GLuint vboID[2];
	glGenVertexArrays(1, &vaoID);
    glBindVertexArray(vaoID);

    glGenBuffers(2, vboID);

    glBindBuffer(GL_ARRAY_BUFFER, vboID[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(0);  // Vertex position

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboID[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elems), elems, GL_STATIC_DRAW);

    glBindVertexArray(0);

	glPatchParameteri(GL_PATCH_VERTICES, 4);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

//Display function to compute uniform values based on transformation parameters and to draw the scene
void display()
{
	glm::vec4 cameraPosn = glm::vec4(eye_x, eye_y, eye_z, 1.0);
	glUniform4fv(eyeLoc, 1, &cameraPosn[0]); //pass camera position to control shader

	//--------Compute matrices----------------------
	glm::mat4 proj = glm::perspective(30.0f * toRad, 1.25f, 20.0f, 500.0f);  //perspective projection matrix
	glm::mat4 view = lookAt(glm::vec3(eye_x, eye_y, eye_z), glm::vec3(look_x, look_y, look_z), glm::vec3(0.0, 1.0, 0.0)); //view matri
	
	cout << "eyepos x : " << eye_x <<endl;
	cout << "eyepos y : " << eye_y << endl;
	cout << "eyepos z : " << eye_z << endl;

	glUniformMatrix4fv(mvMatrixLoc, 1, GL_FALSE, &view[0][0]);
	
	glm::mat4 mvpMatrix = proj * view;  //Product matrix
	glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, &mvpMatrix[0][0]);

	glm::mat4 invMatrix = glm::inverse(view);
	glUniformMatrix4fv(norMatrixLoc, 1, GL_TRUE, &invMatrix[0][0]);

	//glm::vec4 lightPosn = glm::vec4(-100.0, 10.0, 10.0, 1.0);
	glm::vec4 lightPosn = glm::vec4(-50, 10, 60, 1.0);
	//glm::vec4 lightEye = lightPosn;
	glm::vec4 lightEye = view * lightPosn;
	glUniform4fv(lightLoc, 1, &lightEye[0]);

	
	//cracking
	glUniform1i(fixCrackingLoc, fixCracking);

	//wireframe
	glUniform1i(wireframeLoc, isWireframe);

	//fog
	glUniform1i(fogLoc, hasFog);

	//fogLevel
	glUniform1f(fogLevelLoc, fogLevel);

	//waterlevel
	glUniform1f(waterLevelLoc, waterLevel);

	//snowlevel
	glUniform1f(snowLevelLoc, snowLevel);


	
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glBindVertexArray(vaoID);
	//glDrawElements(GL_QUADS, 81 * 4, GL_UNSIGNED_SHORT, NULL);
	glDrawElements(GL_PATCHES, 81 * 4, GL_UNSIGNED_SHORT, NULL);
	glFlush();
}

void special(int key, int x, int y)
{
	if (key == GLUT_KEY_UP) {
		eye_x += sin(angle);
		eye_z -= cos(angle);
		
	}
	else if (key == GLUT_KEY_DOWN) {
		eye_x -= sin(angle);
		eye_z += cos(angle);
	}
	else if (key == GLUT_KEY_LEFT) {
		angle -= rotation; 
	}
	else if (key == GLUT_KEY_RIGHT) {
		angle += rotation;
	}

	if (eye_x >= eye_x_max) eye_x = eye_x_max;
	if (eye_x <= eye_x_min) eye_x = eye_x_min;
	if (eye_z >= eye_z_max) eye_z = eye_z_max;
	if (eye_z <= eye_z_min) eye_z = eye_z_min;
	look_x = eye_x + 90 * sin(angle);
	look_z = eye_z - 90 * cos(angle);
	
	glutPostRedisplay();

}


void keyEvents(unsigned char key, int x, int y)
{
	if (key == 0x1B) exit(EXIT_SUCCESS);

	/* Change Textures */
	if (key == '1') {
		loadTexture("./HeightMaps/MtCook.tga");
	}
	if (key == '2') {
		loadTexture("./HeightMaps/MtRuapehu.tga");
	}
	if (key == 'c' ) {
		fixCracking = !fixCracking;
	}

	if (key == 'f') {
		hasFog = !hasFog;

	}
	if (hasFog) {
		if (key == 'i') {
			fogLevel += 0.005;
			if (fogLevel > 0.05) {
				fogLevel = 0.05;
			}
		}
		if (key == 'd') {
			fogLevel -= 0.005;
			if (fogLevel < 0.02) {
				fogLevel = 0.02;
			}
		}
	}
	if (key == 0x20) {	
		if (!(isWireframe = !isWireframe))
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  // Wire view if true
		else
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	if (key == 'q') {
		waterLevel += 0.1;
		if (waterLevel > 5){
			waterLevel = 5;
		}
	}
	if (key == 'a') {
		waterLevel -= 0.1;
		if (waterLevel < 0){
			waterLevel = 0;
		}
	}

	if (key == 'w') {
		snowLevel -= 0.1;
		if (snowLevel < 4) {
			snowLevel = 4;
		}
	}
	if (key == 's') {
		snowLevel += 0.1;
		if (snowLevel > 10) {
			snowLevel = 10;
		}
	}
	glutPostRedisplay();

}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_SINGLE | GLUT_DEPTH);
	glutInitWindowSize(1000, 800);
	glutCreateWindow("Terrain");
	glutInitContextVersion (4, 2);
	glutInitContextProfile ( GLUT_CORE_PROFILE );

	if(glewInit() == GLEW_OK)
	{
		cout << "GLEW initialization successful! " << endl;
		cout << " Using GLEW version " << glewGetString(GLEW_VERSION) << endl;
	}
	else
	{
		cerr << "Unable to initialize GLEW  ...exiting." << endl;
		exit(EXIT_FAILURE);
	}

	initialise();
	glutDisplayFunc(display); 
	glutSpecialFunc(special);
	glutKeyboardFunc(keyEvents);
	glutMainLoop();
	return 0;
}

