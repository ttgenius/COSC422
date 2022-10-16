//  ========================================================================
//  COSC422: Advanced Computer Graphics;  University of Canterbury (2022)
//
//  FILE NAME: Skeleton Animation.cpp
//  See Ex14_SkeletalAnimation.pdf for details 
//  ========================================================================

#include <iostream>
#include <fstream>
#include <cmath> 
#include <GL/freeglut.h>

using namespace std;

#include <assimp/cimport.h>
#include <assimp/types.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "assimp_extras.h"

#include "loadBMP.h"
#include "loadTGA.h"

#include "math.h"
#include "stdlib.h"
#include "stdio.h"

#define GL_CLAMP_TO_EDGE 0x812F

//----------Globals----------------------------
const aiScene* scene = NULL;
aiVector3D scene_min, scene_max, scene_center;
float scene_scale;

aiAnimation* anim;
int tDuration;       //Animation duration in ticks.
int tick = 0; 
float fps;
int timeStep;

aiVector3D footVec;
GLuint txId[9];  //Texture ID

//float  eye_x = 0, eye_y = 0, eye_z = 7;    //Initial camera position
//float look_x = 2, look_y = 0, look_z = 20;    //"Look-at" point along -z direction
float toRad = 3.14159265 / 180.0;     //Conversion from degrees to rad
float angle = 0;

float PI = 3.14159265;
float CDR = PI / 180.0;

GLUquadric* q;
float pig_angle = 0;
float theta = 20.0;
float jump = 0;
int direction = 1;
float t = 0;

float lightPosn[4] = { -5, 10, 10, 1 };
float shadowMat[16] = { lightPosn[1],0,0,0, -lightPosn[0],0,-lightPosn[2],-1,0,0,lightPosn[1],0, 0,0,0,lightPosn[1] };

float cam_y = 1;
float cam_x = 0;
float cam_z = 10;
float camera_movement_speed = 1;
float camera_rotation_speed = 3;
float view_angle = 0;


float ball_x = -0.3;
float ball_y = 0.2;
float ball_z = 0.6;


float ball_time = 0;
float ball_velocity = 10;
float ball_vx = ball_velocity * cos(45 * CDR) * cos(45 * CDR);;
float ball_vy = ball_velocity * sin(45 * CDR) * cos(45 * CDR);;
float ball_vz = ball_velocity * cos(45 * CDR);
float gravity = 9.81;
bool ballReset = false;
bool ballKicked = false;

bool pause = false;

void loadTexture(void)
{
	glGenTextures(9, txId); 	// Create texture ids

	glBindTexture(GL_TEXTURE_2D, txId[0]);  //Use this texture
	loadBMP("ground.bmp");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[1]);  //Use this texture
	loadBMP("roof0.bmp");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[2]);  //Use this texture
	loadBMP("wall.bmp");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, txId[3]);  //Use this texture
	loadTGA("organic_bk.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, txId[4]);  //Use this texture
	loadTGA("organic_rt.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, txId[5]);  //Use this texture
	loadTGA("organic_ft.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, txId[6]);  //Use this texture
	loadTGA("organic_lf.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glBindTexture(GL_TEXTURE_2D, txId[7]);  //Use this texture
	loadTGA("organic_up.tga");
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);	//Set texture parameters
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

//--Draws a grid of lines on the floor plane -------------------------------
void drawFloor()
{
	float white[4] = { 1., 1., 1., 1. };
	float black[4] = { 0 };

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, txId[0]);

	//glTexEnvi(GL_TEXTURE_ENV,GL_TEXTURE_ENV_MODE,GL_MODULATE);
	glBegin(GL_QUADS);
	glMaterialfv(GL_FRONT, GL_SPECULAR, black);
	for (int i = -100; i < 100; i += 10)
	{
		for (int j = -100; j < 100; j += 10)
		{
			glTexCoord2f(0.0, 0.0);
			glVertex3f(i, 0.01, j); 
			glTexCoord2f(0.0, 1.0);
			glVertex3f(i, 0.01, j + 10);
			glTexCoord2f(1.0, 1.0);
			glVertex3f(i + 10, 0.01, j + 10);
			glTexCoord2f(1.0, 0.0);
			glVertex3f(i + 10, 0.01, j);
		}
	}
	glEnd();
	glDisable(GL_TEXTURE_2D);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
}

void skybox() {
	glEnable(GL_BLEND);
	glColor4f(1.0, 1.0, 1.0, 0.9);
	glPushMatrix();
	float white[4] = { 1.0, 1.0, 1.0, 1.0 };
	float black[4] = { 0.0, 0.0, 0.0, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, black);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, white);
	glEnable(GL_TEXTURE_2D);
	// Back
	glBindTexture(GL_TEXTURE_2D, txId[3]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.5);		glVertex3f(-100.0, 0, -100.0);
	glTexCoord2f(1.0, 0.5);		glVertex3f(100.0, 0, -100.0);
	glTexCoord2f(1.0, 1.0);		glVertex3f(100.0, 100.0, -100.0);
	glTexCoord2f(0.0, 1.0);		glVertex3f(-100.0, 100.0, -100.0);
	glEnd();
	// Right
	glBindTexture(GL_TEXTURE_2D, txId[4]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.5);		glVertex3f(100.0, 0, -100.0);
	glTexCoord2f(1.0, 0.5);		glVertex3f(100.0, 0, 100.0);
	glTexCoord2f(1.0, 1.0);		glVertex3f(100.0, 100.0, 100.0);
	glTexCoord2f(0.0, 1.0);		glVertex3f(100.0, 100.0, -100.0);
	glEnd();
	/*
	// Front
	glBindTexture(GL_TEXTURE_2D, txId[5]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.5);		glVertex3f(100.0, 0, 100.0);
	glTexCoord2f(1.0, 0.5);		glVertex3f(-100.0, 0, 100.0);
	glTexCoord2f(1.0, 1.0);		glVertex3f(-100.0, 100.0, 100.0);
	glTexCoord2f(0.0, 1.0);		glVertex3f(100.0, 100.0, 100.0);
	glEnd();
	*/
	// Left
	glBindTexture(GL_TEXTURE_2D, txId[6]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.5);		glVertex3f(-100.0, 0, 100.0);
	glTexCoord2f(1.0, 0.5);		glVertex3f(-100.0, 0, -100.0);
	glTexCoord2f(1.0, 1.0);		glVertex3f(-100.0, 100.0, -100.0);
	glTexCoord2f(0.0, 1.0);		glVertex3f(-100.0, 100.0, 100.0);
	glEnd();
	/*
	// Top
	glBindTexture(GL_TEXTURE_2D, txId[7]);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);		glVertex3f(100.0, 100.0, -100.0);
	glTexCoord2f(1.0, 0.0);		glVertex3f(100.0, 100.0, 100.0);
	glTexCoord2f(1.0, 1.0);		glVertex3f(-100.0, 100.0, 100.0);
	glTexCoord2f(0.0, 1.0);		glVertex3f(-100.0, 100.0, -100.0);
	glEnd();
	*/
	glDisable(GL_TEXTURE_2D);
	glMaterialfv(GL_FRONT, GL_SPECULAR, white);
	glPopMatrix();
	glDisable(GL_BLEND);

}

//Cottage, a custom_buil model using coordinates and polygon definitions
void drawRoof() {

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, txId[1]);
	glColor3f(1, 1, 1);

	glBegin(GL_TRIANGLES);

	glNormal3f(0, 1, 1);
	glTexCoord2f(0, 0);
	glVertex3f(-10, 6, 4);
	glTexCoord2f(5, 0);
	glVertex3f(10, 6, 4);
	glTexCoord2f(2.5, 5);
	glVertex3f(0, 12, 0);

	glNormal3f(1, 1, 0);
	glTexCoord2f(0, 0);
	glVertex3f(10, 6, 4);
	glTexCoord2f(5, 0);
	glVertex3f(10, 6, -4);
	glTexCoord2f(2.5, 5);
	glVertex3f(0, 12, 0);

	glNormal3f(0, 1, -1);
	glTexCoord2f(0, 0);
	glVertex3f(10, 6, -4);
	glTexCoord2f(5, 0);
	glVertex3f(-10, 6, -4);
	glTexCoord2f(2.5, 5);
	glVertex3f(0, 12, 0);

	glNormal3f(-1, 1, 0);
	glTexCoord2f(0, 0);
	glVertex3f(-10, 6, -4);
	glTexCoord2f(5, 0);
	glVertex3f(-10, 6, 4);
	glTexCoord2f(2.5, 5);
	glVertex3f(0, 12, 0);

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawWalls() {

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, txId[2]);
	glColor3f(1, 1, 1);

	glBegin(GL_QUADS);

	glNormal3f(0, 0, 1);                  //front side left of the door
	glTexCoord2f(0, 0);
	glVertex3f(-10, 0, 4);
	glTexCoord2f(1, 0);
	glVertex3f(-1, 0, 4);
	glTexCoord2f(1, 1);
	glVertex3f(-1, 6, 4);
	glTexCoord2f(0, 1);
	glVertex3f(-10, 6, 4);

	glNormal3f(0, 0, 1);                 //above the door
	glTexCoord2f(0, 0);
	glVertex3f(-1, 3, 4);
	glTexCoord2f(0.22, 0);
	glVertex3f(1, 3, 4);
	glTexCoord2f(0.22, 0.5);
	glVertex3f(1, 6, 4);
	glTexCoord2f(0, 0.5);
	glVertex3f(-1, 6, 4);

	glNormal3f(0, 0, 1);                  //right of the door
	glTexCoord2f(0, 0);
	glVertex3f(1, 0, 4);
	glTexCoord2f(1, 0);
	glVertex3f(10, 0, 4);
	glTexCoord2f(1, 1);
	glVertex3f(10, 6, 4);
	glTexCoord2f(0, 1);
	glVertex3f(1, 6, 4);

	glNormal3f(1, 0, 0);                   //right side
	glTexCoord2f(0, 0);
	glVertex3f(10, 0, 4);
	glTexCoord2f(1, 0);
	glVertex3f(10, 0, -4);
	glTexCoord2f(1, 1);
	glVertex3f(10, 6, -4);
	glTexCoord2f(0, 1);
	glVertex3f(10, 6, 4);

	glNormal3f(0, 0, -1);                  //backside left of the door
	glTexCoord2f(0, 0);
	glVertex3f(-10, 0, -4);
	glTexCoord2f(1, 0);
	glVertex3f(-1, 0, -4);
	glTexCoord2f(1, 1);
	glVertex3f(-1, 6, -4);
	glTexCoord2f(0, 1);
	glVertex3f(-10, 6, -4);

	glNormal3f(0, 0, -1);                 //above the door
	glTexCoord2f(0, 0);
	glVertex3f(-1, 3, -4);
	glTexCoord2f(1, 0);
	glVertex3f(1, 3, -4);
	glTexCoord2f(1, 1);
	glVertex3f(1, 6, -4);
	glTexCoord2f(0, 1);
	glVertex3f(-1, 6, -4);

	glNormal3f(0, 0, -1);                  //right of the door
	glTexCoord2f(0, 0);
	glVertex3f(1, 0, -4);
	glTexCoord2f(1, 0);
	glVertex3f(10, 0, -4);
	glTexCoord2f(1, 1);
	glVertex3f(10, 6, -4);
	glTexCoord2f(0, 1);
	glVertex3f(1, 6, -4);

	glNormal3f(-1, 0, 0);
	glTexCoord2f(0, 0);
	glVertex3f(-10, 0, 4);
	glTexCoord2f(1, 0);
	glVertex3f(-10, 0, -4);
	glTexCoord2f(1, 1);
	glVertex3f(-10, 6, -4);
	glTexCoord2f(0, 1);
	glVertex3f(-10, 6, 4);

	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawHouse() {
	glPushMatrix();
	glEnable(GL_TEXTURE_2D);
	drawRoof();
	drawWalls();
	glDisable(GL_TEXTURE_2D);
	glPopMatrix();

}


void drawPig()
{
	//glColor3f(1, 0.7529, 0.7961);
	glPushMatrix();                          //body
	glColor3f(1, 0.7529, 0.7961);
	glTranslatef(30.0, 4, 0.0);
	glRotatef(-90.0, 0., 1., 0.);
	gluCylinder(q, 2.0, 2.0, 4, 20, 5);
	glTranslatef(0.0, 0.0, 4);
	gluDisk(q, 0.0, 2.0, 20, 4);
	glTranslatef(0.0, 0.0, -4);
	gluDisk(q, 0.0, 2.0, 20, 4);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(25, 4, 0.0);                  //head
	glutSolidSphere(1.5, 20, 20);
	glPopMatrix();

	glPushMatrix();                          //legs
	glTranslatef(27, 1, 0.5);
	glScalef(0.5, 2, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();                          //legs
	glColor3f(1, 0.7529, 0.7961);
	glTranslatef(27, 1, -0.5);
	glScalef(0.5, 2, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();                          //legs
	glTranslatef(29.5, 1, 0.5);
	glScalef(0.5, 2, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();                          //legs
	glTranslatef(29.5, 1, -0.5);
	glScalef(0.5, 2, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();                          //tail
	glTranslatef(30, 4, 0);
	glRotatef(theta, 0, 1, 0);
	glRotatef(-90, 0, 0, 1);
	glScalef(0.5, 3, 0.5);
	glutSolidCube(1);
	glPopMatrix();

}


void drawSheep() {
	glColor3f(1, 1, 1);
	glPushMatrix();                          //body
	glTranslatef(-26, 4, 0.0);
	glRotatef(-90.0, 0., 1., 0.);
	gluCylinder(q, 2.0, 2.0, 4, 20, 5);
	glTranslatef(0.0, 0.0, 4);
	gluDisk(q, 0.0, 2.0, 20, 4);
	glTranslatef(0.0, 0.0, -4);
	gluDisk(q, 0.0, 2.0, 20, 4);
	glPopMatrix();

	glPushMatrix();
	glTranslatef(-25, 5, 0.0);
	glutSolidCube(2);                         //head                   
	glPopMatrix();

	glPushMatrix();                          //legs
	glTranslatef(-27, 1, 0.5);
	glScalef(0.5, 2, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();                          //legs
	glTranslatef(-27, 1, -0.5);
	glScalef(0.5, 2, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();                          //leg
	glTranslatef(-29.5, 1, 0.5);
	glScalef(0.5, 2, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();                          //leg
	glTranslatef(-29.5, 1, -0.5);
	glScalef(0.5, 2, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	glPushMatrix();                          //tail
	glTranslatef(-30, 4, 0);
	glRotatef(theta, 0, 1, 0);
	glRotatef(-90, 0, 0, 1);
	glScalef(0.5, 3, 0.5);
	glutSolidCube(1);
	glPopMatrix();

	}



// ------A recursive function to traverse scene graph and render each mesh----------
// Simplified version for rendering a skeleton mesh
void render(const aiNode* node)
{
	aiMatrix4x4 m = node->mTransformation;
	aiMesh* mesh;
	aiFace* face;
	float materialCol[4] = { 1, 0, 1, 1 };
	int meshIndex;

	m.Transpose();   //Convert to column-major order
	glPushMatrix();
	glMultMatrixf((float*)&m);   //Multiply by the transformation matrix for this node


	if ((node->mName) == aiString("Chest"))
	{
		glPushMatrix();
		glTranslatef(0, 7, 0);
		glScalef(16, 20, 4);
		glutSolidCube(1);
		glPopMatrix();
	}
	else if ((node->mName) == aiString("Hips"))
	{
		glPushMatrix();
		glScalef(16, 4, 4);
		glutSolidCube(1);
		glPopMatrix();
	}
	else if ((node->mName) == aiString("RightCollar"))
	{
		glPushMatrix();
		glTranslatef(8, 0, 0);
		glutSolidSphere(2.8, 20, 20);
		glPopMatrix();
	}
	else if ((node->mName) == aiString("LeftCollar"))
	{
		glPushMatrix();
		glTranslatef(-8, 0, 0);
		glutSolidSphere(2.8, 20, 20);
		glPopMatrix();
	}
	else if (((node->mName) == aiString("RightUpLeg")) || ((node->mName) == aiString("LeftUpLeg")))
	{
		glPushMatrix();
		glTranslatef(0, -9, 0);
		glScalef(3, 18, 3);
		glutSolidCube(1);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0, -18, 0);
		glutSolidSphere(3, 20, 20);
		glPopMatrix();
	}
	else if (((node->mName) == aiString("RightLowLeg")) || ((node->mName) == aiString("LeftLowLeg")))
	{
		glPushMatrix();
		glTranslatef(0, -9, 0);
		glScalef(3, 18, 3);
		glutSolidCube(1);
		glPopMatrix();
	}
	else if (((node->mName) == aiString("RightFoot")) || ((node->mName) == aiString("LeftFoot")))
	{
		
			//Calculates the world coordinates of the Right Foot
		
		if ((node->mName) == aiString("RightFoot"))
		{
			aiNode* parent = node->mParent;
			aiMatrix4x4 matrices[4];
			matrices[3] = node->mTransformation;
			int index = 2;
			while (parent != NULL) {
				matrices[index] = parent->mTransformation;
				parent = parent->mParent;
				index--;
			}
			aiMatrix4x4 worldMatrix = matrices[0];
			for (int i = 1; i < 4; i++)
			{
				worldMatrix *= matrices[i];
			}
			footVec = aiVector3D(0, 0, 0);
			footVec *= worldMatrix;
		}
		glPushMatrix();
		glTranslatef(0, -1.5, 2);
		glScalef(3, 3, 7);
		glutSolidCube(1);
		glPopMatrix();
	}
	else if (((node->mName) == aiString("RightUpArm")) || ((node->mName) == aiString("LeftUpArm")))
	{
		glPushMatrix();
		glTranslatef(0, -7, 0);
		glScalef(2.5, 14, 2.5);
		glutSolidCube(1);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0, -14, 0);
		glutSolidSphere(2.8, 20, 20);
		glPopMatrix();
	}
	else if (((node->mName) == aiString("RightHand")) || ((node->mName) == aiString("LeftHand")))
	{
		glPushMatrix();
		glTranslatef(0, 0, 0);
		glScalef(2.5, 14, 2.5);
		glutSolidCube(1);
		glPopMatrix();

		glPushMatrix();
		glTranslatef(0, -8, 0);
		glutSolidSphere(2.8, 20, 20);
		glPopMatrix();
	}
	else if (((node->mName) == aiString("Neck")))
	{
		glPushMatrix();
		glTranslatef(0, 3, 0);
		glRotatef(90, 1, 0, 0);
		glutSolidCylinder(1.5, 5, 50, 10);
		glPopMatrix();
	}
	else if (((node->mName) == aiString("Head")))
	{
	   
	    glPushMatrix();
		//glColor4f(1, 0.7529, 0.7961, 1);  // a pink hat
		glTranslatef(0, 6, 0);
		glRotatef(-90, 1, 0, 0);
		glutSolidCone(10, 5, 20, 20);
		glPopMatrix();
		

		glPushMatrix();
		glTranslatef(0, 2, 0);
		glutSolidSphere(5, 20, 20);
		glPopMatrix();
	}
	
	/*
	//The scene graph for a skeleton contains at most one mesh per node
	//Skeleton meshes are always triangle meshes
	else {
		if (node->mNumMeshes > 0)
		{
			meshIndex = node->mMeshes[0];          //Get the mesh indices from the current node
			mesh = scene->mMeshes[meshIndex];    //Using mesh index, get the mesh object
			glColor4fv(materialCol);   //Default material colour

			//Draw the mesh in the current node
			for (int k = 0; k < mesh->mNumFaces; k++)
			{
				face = &mesh->mFaces[k];
				glBegin(GL_TRIANGLES);
				for (int i = 0; i < face->mNumIndices; i++) {
					int vertexIndex = face->mIndices[i];
					if (mesh->HasNormals())
						glNormal3fv(&mesh->mNormals[vertexIndex].x);
					glVertex3fv(&mesh->mVertices[vertexIndex].x);
				}
				glEnd();
			}
		}
	}
	*/
	// Recursively draw all children of the current node
	for (int i = 0; i < node->mNumChildren; i++)
		render(node->mChildren[i]);

	glPopMatrix();
}

//----- Function to update node matrices for each tick ------
// Complete this function
void updateNodeMatrices(int tick)
{
	//aiAnimation* anim;   //Animation object
	aiNodeAnim* chnl;
	aiVector3D posn;
	aiQuaternion rotn;
	aiMatrix4x4 matPos;
	aiMatrix3x3 matRotn3;
	aiMatrix4x4 matRot;
	aiMatrix4x4 matProd;
	aiNode* node;

	//anim = scene->mAnimations[0];
	for (int i = 0; i < anim->mNumChannels; i++)
	{
		//See slides [10]-33, 34  and complete this section
		matPos = aiMatrix4x4(); //Identity
		matRot = aiMatrix4x4();
		aiNodeAnim* chnl = anim->mChannels[i];        //Channel
		
		chnl = anim->mChannels[i];
		if (chnl->mNumPositionKeys > 1) {
			posn = chnl->mPositionKeys[tick].mValue;
		}
		else {
			posn = chnl->mPositionKeys[0].mValue;
		}
		if (chnl->mNumRotationKeys > 1) {
			rotn = chnl->mRotationKeys[tick].mValue;
		}
		else {
			rotn = chnl->mRotationKeys[0].mValue;
		}

		matPos.Translation(posn, matPos);
		matRotn3 = rotn.GetMatrix();
		matRot = aiMatrix4x4(matRotn3);
		matProd = matPos * matRot;

		node = scene->mRootNode->FindNode(chnl->mNodeName);
		node->mTransformation = matProd;
	}
}


void kickBall()
{
	if (ball_y > 0.2)
	{
		ball_x -= ball_vx / 1000 * timeStep;
		ball_y += ball_vy / 1000 * timeStep;
		ball_z += ball_vz / 1000 * timeStep;
		ball_vy -= gravity / 1000 * timeStep;
	}
	else if (ballReset)
	{
		ball_x = -0.3;
		ball_y = 0.2;
		ball_z = 0.6;

		ball_vx = ball_velocity * cos(45 * CDR) * cos(45 * CDR);
		ball_vy = ball_velocity * sin(45 * CDR) * cos(45 * CDR);
		ball_vz = ball_velocity * cos(45 * CDR);

		ballKicked = false;
		ballReset = false;
	}
}



void update(int value) {
	if (!pause){
		if (tick > tDuration) {
			tick = 0;
			ballReset = true;
		}
	
		else {
			updateNodeMatrices(tick);
			tick++;

			if ((footVec.x * scene_scale) <= ball_x + 0.2 && (footVec.z * scene_scale) >= ball_z - 0.2 && !ballKicked) {
				ballKicked = true;
				//ballReset = false;
			}
			if (ballKicked) {
				kickBall();
			}
		}

		glutTimerFunc(timeStep, update, tick);
		}
	else {
		glutTimerFunc(timeStep, update, 0);
	}
	glutPostRedisplay();

}


void main_timer(int value)
{
	if (!pause) {
		if (theta == 20) {
			direction = -1;
		}
		if (theta == -20) {
			direction = 1;
		}
		theta += direction;

		t += 0.01;
	}
	glutTimerFunc(20, main_timer, 0);
	glutPostRedisplay();

}


void animal_timer(int value)
{
	if (!pause) {
		if (pig_angle < 360) {
			pig_angle++;
		}
		else {
			pig_angle = 0;
		}

		if (jump == 5) {
			direction = -1;
		}
		if (jump == 0) {
			direction = 1;
		}
		jump += direction;
	}
	glutTimerFunc(100, animal_timer, 0);
	glutPostRedisplay();


}

// Camera rotation
void cameraRotation(int direction) {
	view_angle += direction * camera_rotation_speed;

	if (view_angle >= 360) {
		view_angle = 0;
	}
	else if (view_angle <= 0) {
		view_angle = 360;
	}
}

// Camera zoom
void cameraZoom(int direction) {
	//cout << cam_z << endl;
	cam_z += direction * camera_movement_speed;

	if (cam_z <= 2) {
		cam_z = 2;
	}

	if (cam_z >= 90) {
		cam_z = 90;
	}
}



void special(int key, int x, int y)
{
	if (key == GLUT_KEY_UP) {
		cameraZoom(-1);
	}
	else if (key == GLUT_KEY_DOWN) {
		cameraZoom(1);
	}
	else if (key == GLUT_KEY_LEFT) {
		cameraRotation(1);
	}
	else if (key == GLUT_KEY_RIGHT) {
		cameraRotation(-1);
	}
	glutPostRedisplay();

}


void keyboard(unsigned char key, int x, int y)
{

	if (key == ' ') {
		pause = !pause;
	}
	
	glutPostRedisplay();
}




//--------------------OpenGL initialization------------------------
void initialise()
{
	float ambient[4] = { 0.2, 0.2, 0.2, 1.0 };  //Ambient light
	float white[4] = { 1, 1, 1, 1 };		    //Light's colour
	float black[4] = { 0, 0, 0, 1 };

	q = gluNewQuadric();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, white);
	glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
	glLightfv(GL_LIGHT0, GL_SPECULAR, black);   //Disable specular light
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_NORMALIZE);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(40, 1, 1.0, 500.0);

	//---- Load the model ------
	scene = aiImportFile("Dance.bvh", aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_Debone);

	anim = scene->mAnimations[0];
	fps = anim->mTicksPerSecond;
	tDuration = anim->mDuration;
	timeStep = int(1000 / fps);

	if (scene == NULL) exit(1);
	printTreeInfo(scene->mRootNode);
	printAnimInfo(scene, 0);
	
	loadTexture();

	get_bounding_box(scene, &scene_min, &scene_max);
	scene_center = (scene_min + scene_max) * 0.5f;
	aiVector3D scene_diag = scene_max - scene_center;
	scene_scale = 1.0 / scene_diag.Length();
}


void drawBall() {
	glPushMatrix();
	glColor3f(1, 0, 0);
	glTranslatef(ball_x, ball_y, ball_z);
	glutSolidSphere(0.1, 20, 20);
	glPopMatrix();
}

void drawBallShadow()
{

	glDisable(GL_LIGHTING);
	glDisable(GL_TEXTURE_2D);

	glPushMatrix();
	glTranslatef(0, 0.05, 0);
	glMultMatrixf(shadowMat);
	glColor4f(0.1, 0.1, 0.1, 1);
	glTranslatef(ball_x, ball_y, ball_z);
	glutSolidSphere(0.1, 20, 20);
	glPopMatrix();

	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);

}




void drawRobot() {
	glPushMatrix(); //robot
	glScalef(scene_scale, scene_scale, scene_scale);
	//glTranslatef(-xpos, 0, -zpos);   //Move model to origin
	glTranslatef(-scene_center.x, -scene_center.y, -scene_center.z);
	render(scene->mRootNode);
	glPopMatrix();
}

void drawRobotShadow() {
	glDisable(GL_LIGHTING); //robot shadow
	glDisable(GL_TEXTURE_2D);
	glPushMatrix();
	glTranslatef(0, 0.05, 0);
	glMultMatrixf(shadowMat);
	glScalef(scene_scale, scene_scale, scene_scale);  //robot shadow
	//glTranslatef(-xpos, 0, -zpos);
	glTranslatef(-scene_center.x, -scene_center.y, -scene_center.z);
	glColor4f(0.1, 0.1, 0.1, 1);
	render(scene->mRootNode);
	glPopMatrix();
	glEnable(GL_LIGHTING);
	glEnable(GL_TEXTURE_2D);
}

//------The main display function---------
void display()
{
	//float lightPosn[4] = { -5, 10, 10, 1 };
	aiMatrix4x4 m = scene->mRootNode->mTransformation;
	float xpos = m.a4;   //Root joint's position in world space
	float ypos = m.b4;
	float zpos = m.c4;
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	//gluLookAt(0, 0, 7, 0, 0, 0, 0, 1, 0);
	gluLookAt(cam_x, cam_y, cam_z, 0, cam_y, 0, 0, 1, 0);
	glLightfv(GL_LIGHT0, GL_POSITION, lightPosn);
	
	glDisable(GL_LIGHTING);
	drawFloor();
	skybox();
	glEnable(GL_LIGHTING);
	
	glPushMatrix();
	glTranslatef(-scene_center.x, -scene_center.y, -scene_center.z);
	glRotatef(view_angle, 0, 1, 0);
	glTranslatef(scene_center.x, scene_center.y, scene_center.z);
	
		drawRobot();
		drawRobotShadow();
		drawBall();
		drawBallShadow();
		

	
		glPushMatrix();              //cottage
			//glTranslatef(-30, 0, -80);
			//glRotatef(22, 0, 1, 0);
		glTranslatef(0, 0, -80);
		glScalef(1.5, 1.5, 1.5);
		drawHouse();
		glPopMatrix();

		for (int i = 0; i <= 2;i++) {               //pinky pigs
			for (int j = 0;j <= 2;j++) {
				glPushMatrix();
				glTranslatef(5 * i, 0, -10 * j);
				glTranslatef(30, 4, 0);
				glRotatef(-pig_angle, 0, 1, 0);
				glTranslatef(-30, -4, 0);
				glScalef(0.2, 0.2, 0.2);
				drawPig();
				glPopMatrix();
			}
		}

		for (int i = 0; i <= 2;i++) {                //whitie sheeps  
			for (int j = 0;j <= 2;j++) {
				glPushMatrix();
				glTranslatef(-5 * i, jump, -10 * j);
				glScalef(0.2, 0.2, 0.2);
				drawSheep();
				glPopMatrix();
			}
		}


		/*
		glPushMatrix();              
			glTranslatef(4, 0, -20); //pig
			glScalef(0.2, 0.2, 0.2);
			drawPig();
		glPopMatrix();
	*/
	glPopMatrix();

	glutSwapBuffers();
}


int main(int argc, char** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 800);
	glutCreateWindow("Skeleton Animation");

	initialise();
	glutDisplayFunc(display);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutTimerFunc(timeStep, update, 0);
	glutTimerFunc(20, main_timer, 0);
	glutTimerFunc(80, animal_timer, 0);
	glutMainLoop();

	aiReleaseImport(scene);
}

