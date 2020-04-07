/************************************************************
*                   CSCI 4110 Lab 4
*
*  Basic OpenGL program that shows how to set up a
*  VAO and some basic shaders.  This program draws
*  a cube or sphere depending upon whether CUBE or
*  SPHERE is defined.
*
**********************************************************/
#define GLM_FORCE_RADIANS
#define _USE_MATH_DEFINES

#include <math.h>
#include <GL/glew.h>
#define GLFW_DLL
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include "Shaders.h"
#include "tiny_obj_loader.h"
#include <iostream>
#include "texture.h"

//const char* url = "metatron.obj";
//const char* url = "Meshs/drone.obj";
//const char* url = "Meshs/voxelBall.obj";



float eyex, eyey, eyez;	// current user position
float tran=1.0f;
double theta, phi;		// user's position  on a sphere centered on the object
double r;				// radius of the sphere
float rotv = 0, roth = 0;
float scalef = 1.0f;
glm::vec4 colour;
glm::vec3 eye_pos;
glm::vec3 light;
glm::vec4 material;

GLuint program;

glm::mat4 projection;	// projection matrix

GLuint objVAO;			// vertex object identifier
int triangles;			// number of triangles
GLuint ibuffer;			// index buffer identifier


int init(const char* url) {
	GLuint vbuffer;
	GLuint tBuffer;
	GLint vPosition;
	GLint vNormal;
	GLint vTex;
	GLfloat *vertices;
	GLfloat *normals;
	GLuint *indices;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;

	int nv;
	int nn;
	int ni;
	int nt;
	int i;

	glGenVertexArrays(1, &objVAO);
	glBindVertexArray(objVAO);

	/*  Load the obj file */

	std::string err = tinyobj::LoadObj(shapes, materials, url, 0);

	if (!err.empty()) {
		std::cerr << err << std::endl;
		return 1;
	}

	/*  Retrieve the vertex coordinate data */

	nv = (int)shapes[0].mesh.positions.size();
	vertices = new GLfloat[nv];
	for (i = 0; i<nv; i++) {
		vertices[i] = shapes[0].mesh.positions[i];
	}

	/*  Retrieve the vertex normals */

	nn = (int)shapes[0].mesh.normals.size();
	normals = new GLfloat[nn];
	for (i = 0; i<nn; i++) {
		normals[i] = shapes[0].mesh.normals[i];
	}

	/*  Retrieve the triangle indices */

	ni = (int)shapes[0].mesh.indices.size();
	triangles = ni / 3;
	indices = new GLuint[ni];
	for (i = 0; i<ni; i++) {
		indices[i] = shapes[0].mesh.indices[i];
	}
	/*
		calc text coord
	*/

	double verts = nv / 3;
	nt = 2 * verts;
	GLfloat* tex = new GLfloat[nt];
	for (i = 0; i < verts; i++) {
		GLfloat x = vertices[3 * i];
		GLfloat y = vertices[3 * i + 1];
		GLfloat z = vertices[3 * i + 2];
		theta = atan2(x,z);
		phi = atan2(y,sqrt(x*x + z*z));

		//tex[2 * i] = (theta+M_PI) / (2 * M_PI);
		tex[2 * i] = fabs(theta) / M_PI;
		
		tex[2 * i + 1] = phi / M_PI;
	}

	
	glGenBuffers(1, &vbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vbuffer);
	glBufferData(GL_ARRAY_BUFFER, (nv + nn + nt)*sizeof(GLfloat), NULL,
	GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, nv*sizeof(GLfloat), vertices);
	glBufferSubData(GL_ARRAY_BUFFER, nv*sizeof(GLfloat), nn*sizeof(GLfloat), normals);
	glBufferSubData(GL_ARRAY_BUFFER, (nv + nn)*sizeof(GLfloat), nt*sizeof(GLfloat),
	tex);


	/*
	*  load the vertex indexes
	*/
	glGenBuffers(1, &ibuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, ni*sizeof(GLuint), indices, GL_STATIC_DRAW);


	/*
	*  link the vertex coordinates to the vPosition
	*  variable in the vertex program.  Do the same
	*  for the normal vectors.
	*/

	glUseProgram(program);
	vTex = glGetAttribLocation(program, "vTex");
	glVertexAttribPointer(vTex, 2, GL_FLOAT, GL_FALSE, 0, (void*)((nv + nn)*sizeof(GLfloat)));
	glEnableVertexAttribArray(vTex);
	vPosition = glGetAttribLocation(program, "vPosition");
	glVertexAttribPointer(vPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);
	vNormal = glGetAttribLocation(program, "vNormal");
	glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, (void*)((nv/2)*sizeof(vertices)));
	glEnableVertexAttribArray(vNormal);

	return 0;
}


void framebufferSizeCallback(GLFWwindow *window, int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window of zero width).

	if (h == 0)
		h = 1;

	float ratio = 1.0f * w / h;

	glfwMakeContextCurrent(window);

	glViewport(0, 0, w, h);

	projection = glm::perspective(0.7f, ratio, 1.0f, 100.0f);

}

void display(void) {
	glm::mat4 view;
	int viewLoc;
	int projLoc;
	int tranLoc;
	int colour;
	int eyeLoc;
	int materialLoc;

	eyex = (float)(r * sin(theta) * cos(phi));
	eyey = (float)(r * sin(theta) * sin(phi));
	eyez = (float)(r * cos(theta));

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program);

	view = glm::lookAt(glm::vec3(eyex, eyey, eyez),
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 0.0f, 1.0f));

	view = glm::scale(view, glm::vec3(scalef));

	glm::mat4 transform(1.0f);
	transform = glm::rotate(transform, rotv, glm::vec3(1.0, 0.0, 0.0));
	transform = glm::rotate(transform, roth,glm::vec3(0.0, 0.0, 1.0));

	viewLoc = glGetUniformLocation(program, "modelView");
	glUniformMatrix4fv(viewLoc, 1, 0, glm::value_ptr(view*transform));
	projLoc = glGetUniformLocation(program, "projection");
	glUniformMatrix4fv(projLoc, 1, 0, glm::value_ptr(projection));
	eyeLoc = glGetUniformLocation(program, "Eye");
	glUniform3fv(eyeLoc, 1, glm::value_ptr(eye_pos));
	
	material = glm::vec4(0.4,0.5,0.8,2.0);

	materialLoc = glGetUniformLocation(program, "material");
	tranLoc = glGetUniformLocation(program, "tran");

	glUniform4fv(materialLoc, 1, glm::value_ptr(material));
	glUniform1f(tranLoc,tran);

	glBindVertexArray(objVAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuffer);
	glDrawElements(GL_TRIANGLES, 3*triangles, GL_UNSIGNED_INT, NULL);

}

double lastX = 0, lastY = 0;
double mouseX = 0, mouseY = 0;
double speed = 0.01f;

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	scalef += yoffset * speed * 10;

}

static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	mouseX = xpos;
	mouseY = ypos;

	float distX = abs(lastX - mouseX);
	float distY = abs(lastY - mouseY);

	if (lastX < mouseX) {
		phi -= speed * distX;
	}
	else {
		phi += speed * distX;
	}

	if (lastY < mouseY) {
		theta += speed * distY;
	}
	else {
		theta -= speed * distY;
	}

	lastX = mouseX;
	lastY = mouseY;

}


static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_LEFT && (action == GLFW_REPEAT)) {
		if (lastX <= mouseX) {
			phi -= speed;
		}
		else {
			phi += speed;
		}

		if (lastY <= mouseY) {
			theta -= speed;
		}
		else {
			theta += speed;
		}
	}


}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);

	if (key == GLFW_KEY_UP) {
		rotv += 0.1f;
	}
	if (key == GLFW_KEY_DOWN) {
		rotv -= 0.1f;
	}
	if (key == GLFW_KEY_LEFT) {
		roth -=0.1f;
	}
	if (key == GLFW_KEY_RIGHT) {
		roth += 0.1f;
	}

	if (action == GLFW_PRESS) {
		if (key == GLFW_KEY_C) {
			init("Meshs/Drone.obj");
		}

	}
}

void drop_callback(GLFWwindow* window, int count, const char** paths)
{
	const char* path = paths[0];
	// Validation
	char temp[255];
	std::strcpy(temp, path);
	char* st = std::strtok(temp, "\\");
	char* lst = NULL;
	while (st != NULL) {
		st = std::strtok(NULL, "\\");
		if (st != NULL) {
			lst = st;
		}
	}
	std::strcpy(temp, lst);
	//std::strtok(lst, ".");
	lst = std::strtok(lst, ".");
	char* c =NULL;
	while (lst != NULL) {
		lst = std::strtok(NULL, ".");
		if (lst != NULL) {
			c = lst;
		}
	}
	//printf("%s %d\n",c, std::strcmp("obj", c));
	// DO
	if (!std::strcmp("obj",c)) {
		printf("Opening \"%s\"...\n",temp);
		init(path);
		printf("Reading Complete.\n");
	}
	else {
		printf("\"%s\" is Not a valid File.\n",temp);
	}
}

void error_callback(int error, const char* description)
{
	fprintf(stderr, "Error: %s\n", description);
}

int main(int argc, char **argv) {


	int fs;
	int vs;
	GLFWwindow *window;

	// start by setting error callback in case something goes wrong

	glfwSetErrorCallback(error_callback);

	// initialize glfw

	if (!glfwInit()) {
		fprintf(stderr, "can't initialize GLFW\n");
	}

	// create the window used by our application

	window = glfwCreateWindow(512, 512, "Display", NULL, NULL);

	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}

	// establish framebuffer size change and input callbacks

	glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

	glfwSetKeyCallback(window, key_callback);

	glfwSetScrollCallback(window, scroll_callback);

	glfwSetMouseButtonCallback(window, mouse_button_callback);

	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwSetDropCallback(window, drop_callback);

	// now initialize glew our extension handler

	glfwMakeContextCurrent(window);

	GLenum error = glewInit();
	if (error != GLEW_OK) {
		printf("Error starting GLEW: %s\n", glewGetErrorString(error));
		exit(0);
	}

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.7,0.7, 0.8, 1.0);
	glViewport(0, 0, 512, 512);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

	projection = glm::perspective(0.7f, 1.0f, 1.0f, 100.0f);

	vs = buildShader(GL_VERTEX_SHADER, (char*)"vert.vs");
	fs = buildShader(GL_FRAGMENT_SHADER, (char*)"frag.fs");
	program = buildProgram(vs, fs, 0);
	dumpProgram(program, (char*)"Shader Program");

	init("Meshs/Sphere.obj");

	eyex = 0.0;
	eyez = 0.0;
	eyey = 10.0;

	eye_pos = glm::vec3(eyex,eyez,eyey); 

	theta = 1.5;
	phi = 1.5;
	r = 10.0;

	glfwSwapInterval(1);


	// GLFW main loop, display model, swapbuffer and check for input

	while (!glfwWindowShouldClose(window)) {
		display();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();

}
