#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programDepth;
GLuint programTexture;
GLuint programShadow;

GLuint depthTexture;
GLuint FramebufferObject;


Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;
obj::Model renderModel;

glm::vec3 cameraPos = glm::vec3(0, 0, 14);
glm::vec3 cameraDir;
glm::vec3 cameraSide;

glm::mat4 cameraMatrix, perspectiveMatrix;
glm::mat4 lightProjection;
glm::mat4 lightView;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

glm::quat rotation = glm::quat(1, 0, 0, 0);
glm::vec3 rotationChangeXYZ = glm::vec3(0, 0, 0);

GLuint textureAsteroid;
GLuint textureEarth;

GLuint cubemapTexture;

std::vector<glm::vec4> planets;
std::vector<float> tangent(1203);

float quad[] = {
	0.25,0.25, 0.0, 1.0,
	0.25, -0.25, 0.0, 1.0,
	-0.25, -0.25, 0.0, 1.0,

	0.25, 0.25, 0.0, 1.0,
	-0.25, 0.25, 0.0, 1.0,
	-0.25, -0.25, 0.0, 1.0
};


void keyboard(unsigned char key, int x, int y)
{

	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch (key)
	{
	case 'z': rotationChangeXYZ.z += 0.1f; break;
	case 'x': rotationChangeXYZ.z -= 0.1f; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += cameraSide * moveSpeed; break;
	case 'a': cameraPos -= cameraSide * moveSpeed; break;
	}
}

void mouse(int x, int y)
{
	static int lastMouseX = x;
	static int lastMouseY = y;
	rotationChangeXYZ.y = 0.01f*(x - lastMouseX);
	rotationChangeXYZ.x = 0.01f*(y - lastMouseY);
	lastMouseX = x;
	lastMouseY = y;
}

glm::mat4 createCameraMatrix()
{
	glm::quat rotationChange = glm::quat(rotationChangeXYZ);
	rotation = rotationChange * rotation;
	rotation = glm::normalize(rotation);
	rotationChangeXYZ = glm::vec3(0);
	cameraDir = glm::inverse(rotation) * glm::vec3(0, 0, -1);
	cameraSide = glm::inverse(rotation) * glm::vec3(1, 0, 0);

	return Core::createViewMatrixQuat(cameraPos, rotation);
}



void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);


	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	
	Core::DrawModel(model);


	glUseProgram(0);
}

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectDepth(obj::Model * model, glm::mat4 modelMatrix, glm::mat4 projMatrix, glm::mat4 inverseLightMatrix)
{
	GLuint program = programDepth;

	glUseProgram(program);

	glm::mat4 transformation = projMatrix * inverseLightMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "depthMVP"), 1, GL_FALSE, (float*)&transformation);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawObjectShadow(obj::Model * model, glm::mat4 modelMatrix, glm::mat4 projMatrix, glm::mat4 lightMatrix, GLuint textureId, GLuint depthTexture)
{
	GLuint program = programShadow;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);
	Core::SetActiveTexture(depthTexture, "shadowMap", program, 1);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glm::mat4 lightTransformation = projMatrix * lightMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	glUniformMatrix4fv(glGetUniformLocation(program, "lightMatrix"), 1, GL_FALSE, (float*)&lightTransformation);

	Core::DrawModel(model);

	glUseProgram(0);
}

void renderScene()
{
	
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;


	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();


	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);


	glm::mat4 planetModelMatrix = glm::translate(glm::vec3(sin(-time), 2.0, cos(-time))) * glm::scale(glm::vec3(3.0));


	glm::mat4 renderTarget = glm::translate(glm::vec3(0, -5, 0)) * glm::rotate(-1.56f, glm::vec3(1, 0, 0)) * glm::rotate(1.56f, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(7, 14, 14));

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferObject);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, -30.0f, 30.0f);
	lightView = glm::lookAt(-lightDir,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectDepth(&sphereModel, planetModelMatrix, lightProjection, lightView);
	drawObjectDepth(&renderModel,	renderTarget, lightProjection, lightView);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//drawObjectTexture(&renderModel, renderTarget, depthTexture);

	

	drawObjectShadow(&sphereModel, planetModelMatrix, lightProjection, lightView, textureEarth,depthTexture);
	drawObjectShadow(&renderModel, renderTarget, lightProjection, lightView, textureAsteroid, depthTexture);

	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programDepth = shaderLoader.CreateProgram("shaders/shader_depth.vert", "shaders/shader_depth.frag");
	programShadow = shaderLoader.CreateProgram("shaders/shader_shadow.vert", "shaders/shader_shadow.frag");

	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	renderModel = obj::loadModelFromFile("models/render.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	textureEarth = Core::LoadTexture("textures/earth.png");
	textureAsteroid = Core::LoadTexture("textures/ypos.png");


	std::vector<float> unitY = { 0.0, 1.0, 0.0 };
	std::vector<float> unitX = { 1.0, 0.0, 0.0 };

	glm::vec3 uY = { 0.0, -1.0, 0.0 };
	glm::vec3 uX = { -1.0, 0.0, 0.0 };

	glm::vec3 tang[1203];

	for (int i = 0; i < sphereModel.normal.size(); i += 3)
	{


		glm::vec3 normal = { sphereModel.normal[i + 0], sphereModel.normal[i + 1], sphereModel.normal[i + 2] };

		if (sphereModel.normal[i + 1] < 0.99 && sphereModel.normal[i + 1] > -0.99) tang[i] = glm::normalize(glm::cross(normal, uY));
		else tang[i] = glm::normalize(glm::cross(normal, uX));



		tangent[i + 0] = tang[i].x;
		tangent[i + 1] = tang[i].y;
		tangent[i + 2] = tang[i].z;
		
	}


	//Generate Framebuffer
	FramebufferObject = 0;
	glGenFramebuffers(1, &FramebufferObject);
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferObject); 

	//Generate depth texture
	glGenTextures(1, &depthTexture);
	glBindTexture(GL_TEXTURE_2D, depthTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	//Filtering
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//Attach depth texture to frame buffer
	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferObject);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthTexture, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//shadows
	

}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
	glutPostRedisplay();
}

int main(int argc, char ** argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowPosition(200, 200);
	glutInitWindowSize(1024, 1024);
	glutCreateWindow("CGP");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutPassiveMotionFunc(mouse);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
