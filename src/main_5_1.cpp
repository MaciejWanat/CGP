#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <windows.h>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"
#include <iostream>

GLuint programColor;
GLuint programTexture;
GLuint programSkybox;
GLuint programDepth;
GLuint programShadow;
GLuint programTextureNorm;
GLuint programTextureBasic;

Core::Shader_Loader shaderLoader;

int cubeMapID;

obj::Model shipModel;
obj::Model sphereModel;
obj::Model renderModel;

GLuint depthTexture;
GLuint textureEarth;
GLuint FramebufferObject;
GLuint textureEarthNormal;

glm::mat4 lightProjection;
glm::mat4 lightView;

//Camera
float cameraAngleX = 4.7;
float cameraAngleY = 5;
float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;
glm::mat4 cameraMatrix, perspectiveMatrix;
float yaw = 0.0;
float pitch = 0.0;
float roll = 0.0;

float oldX = 0;
float oldY = 0;;

std::vector<float> tangent(1203);

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

glm::vec3 circle_points[220]; //wektor punktów do naszej krzywej
glm::mat4 rotations[220]; //macierz rotacji BNT - wzd³u¿ krzywej
int pointCounter = 0;
glm::vec3 ship_pos;

//nasze piêkne cz¹steczki
struct Particle {
	glm::vec3 pos;
	glm::quat rot;

	glm::vec3 vel; //linMom??
	glm::vec3 angVel; //AngMom??

	glm::vec3 force;
	glm::vec3 torque;

	glm::mat4 shipDepthModel;
};

std::vector<Particle> spaceships;

glm::mat4 createTranslationMatrixXYZ(float X, float Y, float Z) {
	glm::mat4 translationMatrix;
	translationMatrix[3][0] = X;
	translationMatrix[3][1] = Y;
	translationMatrix[3][2] = Z;
	return translationMatrix;
}

GLuint textureAsteroid, xpos, xneg, ypos, yneg, zpos, zneg;

GLuint cubemapTexture;

std::vector<glm::vec4> planets;

const float cubeVertices[] = {
	30.5f, 30.5f, 30.5f, 1.0f,
	30.5f, -30.5f, 30.5f, 1.0f,
	-30.5f, 30.5f, 30.5f, 1.0f,

	30.5f, -30.5f, 30.5f, 1.0f,
	-30.5f, -30.5f, 30.5f, 1.0f,
	-30.5f, 30.5f, 30.5f, 1.0f,

	30.5f, 30.5f, -30.5f, 1.0f,
	-30.5f, 30.5f, -30.5f, 1.0f,
	30.5f, -30.5f, -30.5f, 1.0f,

	30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, 30.5f, -30.5f, 1.0f,
	-30.5f, -30.5f, -30.5f, 1.0f,

	-30.5f, 30.5f, 30.5f, 1.0f,
	-30.5f, -30.5f, 30.5f, 1.0f,
	-30.5f, -30.5f, -30.5f, 1.0f,

	-30.5f, 30.5f, 30.5f, 1.0f,
	-30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, 30.5f, -30.5f, 1.0f,

	30.5f, 30.5f, 30.5f, 1.0f,
	30.5f, -30.5f, -30.5f, 1.0f,
	30.5f, -30.5f, 30.5f, 1.0f,

	30.5f, 30.5f, 30.5f, 1.0f,
	30.5f, 30.5f, -30.5f, 1.0f,
	30.5f, -30.5f, -30.5f, 1.0f,

	30.5f, 30.5f, -30.5f, 1.0f,
	30.5f, 30.5f, 30.5f, 1.0f,
	-30.5f, 30.5f, 30.5f, 1.0f,

	30.5f, 30.5f, -30.5f, 1.0f,
	-30.5f, 30.5f, 30.5f, 1.0f,
	-30.5f, 30.5f, -30.5f, 1.0f,

	30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, -30.5f, 30.5f, 1.0f,
	30.5f, -30.5f, 30.5f, 1.0f,

	30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, -30.5f, -30.5f, 1.0f,
	-30.5f, -30.5f, 30.5f, 1.0f,
};


void mouseMove(int x, int y)
{
	float sen = 0.05;
	float margin = 5;

	if (x - margin > oldX)
	{
		yaw += sen;
		cameraAngleX += sen;
	}
	else if (x + margin < oldX)
	{
		yaw -= sen;
		cameraAngleX -= sen;
	}

	if (y - margin > oldY)
	{
		pitch += sen;
		cameraAngleY += sen;
	}
	else if (y + margin < oldY)
	{
		pitch -= sen;
		cameraAngleY -= sen;
	}

	oldX = x;
	oldY = y;
}

void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	float moveSpeedUpDown = 0.3f;
	switch(key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'c': cameraPos += glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeedUpDown; break;
	case 'v': cameraPos -= glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeedUpDown; break;
	}
}

glm::mat4 createCameraMatrix()
{
	cameraDir = glm::vec3(cosf(cameraAngleX), 0.0f, sinf(cameraAngleX));

	return Core::createViewMatrix(cameraPos, yaw, pitch, roll);
}

void drawObjectColor(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programColor;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

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

void drawObjectTextureBasic(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId)
{
	GLuint program = programTextureBasic;

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

void drawObjectTextureNormal(obj::Model * model, glm::mat4 modelMatrix, GLuint textureId, GLuint normalMap)
{
	GLuint program = programTextureNorm;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
	glUniform3f(glGetUniformLocation(program, "cameraPos"), cameraPos.x, cameraPos.y, cameraPos.z);
	Core::SetActiveTexture(textureId, "textureSampler", program, 0);
	Core::SetActiveTexture(normalMap, "normalMap", program, 1);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	//glm::mat4 lightTransformation = projMatrix * lightMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);
	//glUniformMatrix4fv(glGetUniformLocation(program, "lightMatrix"), 1, GL_FALSE, (float*)&lightTransformation);

	Core::DrawModelNormal(model);

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

void drawSkybox(GLuint cubemapID) 
{
	GLuint program = programSkybox;

	glUseProgram(program);

	Core::SetActiveTexture(cubemapID, "skybox", program, 0);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);


	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapID);

	Core::DrawVertexArray(cubeVertices, 36, 4);

	glUseProgram(0);
}

//wyznaczenie punktów na okrêgu
void drawCircle(GLfloat x, GLfloat y, GLfloat z, GLfloat radius, GLint numberOfSides)
{
	GLint numberOfVertices = numberOfSides + 1;

	GLfloat doublePi = 2.0f * 3.14f;

	GLfloat* circleVerticesX = new GLfloat[numberOfVertices];
	GLfloat* circleVerticesY = new GLfloat[numberOfVertices];
	GLfloat* circleVerticesZ = new GLfloat[numberOfVertices];

	for (int i = 0; i < numberOfVertices; i++)
	{
		circleVerticesX[i] = x + (radius * cos(i * doublePi / numberOfSides));
		circleVerticesY[i] = y;
		circleVerticesZ[i] = z + (radius * sin(i * doublePi / numberOfSides));
	}

	for (int i = 0; i < numberOfVertices; i++)
	{
		circle_points[i].x = circleVerticesX[i];
		circle_points[i].y = circleVerticesY[i];
		circle_points[i].z = circleVerticesZ[i];
	}
}

//wyznaczenie wektorów BNT
void parallel_transport() {
	glm::vec3 tangent[220];
	glm::vec3 normal[220];
	glm::vec3 binormal[220];
	glm::vec3 T, N, B;
	for (int i = 0; i < 219; i++) {
		T = glm::normalize(circle_points[i + 1] - circle_points[i]);
		tangent[i] = T;
	}
	T = glm::normalize(circle_points[1] - circle_points[219]);
	tangent[219] = T;
	N = (tangent[0] / glm::length(tangent[0]));
	normal[0] = N;

	for (int i = 0; i < 219; i++)
	{
		B = glm::cross(tangent[i], tangent[i + 1]);
		if (glm::length(B) == 0)
			normal[i + 1] = normal[i];
		else {
			B = (B / glm::length(B));
			float theta = acosf(glm::dot(tangent[i],tangent[i+1]));
			normal[i + 1] =  glm::vec4(normal[i], 0.0f) * glm::rotate(theta, B);
		}
		binormal[i] = glm::cross(tangent[i], normal[i]);
		binormal[i] = glm::normalize(binormal[i]);
	}
	binormal[219] = glm::cross(tangent[219], normal[219]);
	binormal[219] = glm::normalize(binormal[219]);

	glm::mat3 BNT;
	for (int i = 0; i < 220; i++) {
		BNT[0] = binormal[i];
		BNT[1] = normal[i];
		BNT[2] = tangent[i];
		rotations[i] = BNT;
	}
}

void initialise_particles(int qty)
{
	for (int i = 0; i < qty; i++) {
		Particle x;
		x.pos.x = -2.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0 - (-2.0))));
		x.pos.y = -2.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0 - (-2.0))));
		x.pos.z = -2.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0 - (-2.0))));
		x.vel = glm::vec3(0, 0, 0);
		spaceships.push_back(x);
	}
}

void renderScene()
{
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	// Aktualizacja macierzy widoku i rzutowania. Macierze sa przechowywane w zmiennych globalnych, bo uzywa ich funkcja drawObject.
	// (Bardziej elegancko byloby przekazac je jako argumenty do funkcji, ale robimy tak dla uproszczenia kodu.
	//  Jest to mozliwe dzieki temu, ze macierze widoku i rzutowania sa takie same dla wszystkich obiektow!)
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0f, 0.3f, 0.3f, 1.0f);

	// Macierz statku "przyczepia" go do kamery. Warto przeanalizowac te linijke i zrozumiec jak to dziala.
	//glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0,-0.25f,0)) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	ship_pos = glm::vec3(circle_points[pointCounter % 220].x, circle_points[pointCounter % 220].y, circle_points[pointCounter % 220].z);
	glm::mat4 shipModelMatrix =  glm::translate(glm::vec3(ship_pos.x, ship_pos.y, ship_pos.z)) * rotations[pointCounter % 220];
	drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.7f,0.0f,0.0f));
	pointCounter++;
	//Sleep(50);

	//przemieszczanie stateczków
	for (int i = 0; i < spaceships.size(); i++)
	{
		glm::mat4 shipModelMatrix = glm::translate(spaceships[i].pos) * rotations[pointCounter % 220] * glm::scale(glm::vec3(0.10f));
		spaceships[i].shipDepthModel = shipModelMatrix;
		drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.0f,0.0f,0.7f));


		glm::vec3 v1, v2, v3;
		float m1 = 5;
		float m2 = 3;
		float m3 = 0.05;
		//v1 = rule1: centre_of_mass = ship_pos;
		v1 = glm::normalize(ship_pos - spaceships[i].pos);
		v1 = m1 * v1;
		//v2 = rule2: keep a distance away from other objects
		if (glm::length(ship_pos - spaceships[i].pos) < 2.5)
			spaceships[i].pos -= (ship_pos - spaceships[i].pos);
		for (int j = 0; j < spaceships.size(); j++){
			v2 = glm::vec3(0, 0, 0);
			if (j != i) //boid which is near by
				if (glm::length(spaceships[j].pos - spaceships[i].pos) < 2)
					v2 -= (spaceships[j].pos - spaceships[i].pos);
		}
		v2 = m2 * v2;
		//v3 = rule3: match velocity with near boids
		for (int j = 0; j < spaceships.size(); j++){
			if (j != i)
				v3 += spaceships[j].vel;
		}
		v3 = v3 / (spaceships.size() - 1);
		v3 = (v3 - spaceships[i].vel) / 8;
		v3 = m3 * v3;

		spaceships[i].vel += v1 + v2 + v3;
		//limiting the speed
		int vlim = 1;
		if (glm::length(spaceships[i].vel) > vlim)
			spaceships[i].vel = (spaceships[i].vel / glm::length(spaceships[i].vel)) * vlim;

		spaceships[i].pos += spaceships[i].vel;
	}
	//planets with shadows
	glm::mat4 planetModelMatrix = glm::translate(glm::vec3(sin(-time), 2.0, cos(-time))) * glm::scale(glm::vec3(3.0));

	glm::mat4 renderTarget = glm::translate(glm::vec3(0, -5, 0)) * glm::rotate(-1.56f, glm::vec3(1, 0, 0)) * glm::rotate(1.56f, glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(7, 14, 14));

	glBindFramebuffer(GL_FRAMEBUFFER, FramebufferObject);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	lightProjection = glm::ortho(-30.0f, 30.0f, -30.0f, 30.0f, -30.0f, 30.0f);
	lightView = glm::lookAt(-lightDir,
		glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f));

	drawObjectDepth(&sphereModel, planetModelMatrix, lightProjection, lightView);
	drawObjectDepth(&renderModel, renderTarget, lightProjection, lightView);
	for (int i = 0; i < spaceships.size(); i++)
	{
		drawObjectDepth(&shipModel, spaceships[i].shipDepthModel, lightProjection, lightView);
	}
	drawObjectDepth(&shipModel, shipModelMatrix, lightProjection, lightView);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//planet with normal mapping
	drawObjectTextureNormal(&sphereModel, planetModelMatrix, textureEarth, textureEarthNormal);
	//drawObjectTextureBasic(&sphereModel, planetModelMatrix, textureEarth);

	//working earth here >
	//drawObjectShadow(&sphereModel, planetModelMatrix, lightProjection, lightView, textureEarth, depthTexture);
	//drawObjectShadow(&renderModel, renderTarget, lightProjection, lightView, textureAsteroid, depthTexture);
	
	//planets with reflexes (from old code)
	/*
	for (int i = 0; i < planets.size(); i++)
	{
		glm::mat4 planetModelMatrix = glm::translate(glm::vec3(planets[i])) * glm::scale(glm::vec3(planets[i].w));
		drawObjectTexture(&sphereModel, planetModelMatrix, cubeMapID);
	}
	*/
	drawSkybox(cubeMapID);
	
	glutSwapBuffers();
}

void init()
{
	srand(time(0));
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	programTextureNorm = shaderLoader.CreateProgram("shaders/shader_norm.vert", "shaders/shader_norm.frag");
	programTextureBasic = shaderLoader.CreateProgram("shaders/shader_basic.vert", "shaders/shader_basic.frag");
	programDepth = shaderLoader.CreateProgram("shaders/shader_depth.vert", "shaders/shader_depth.frag");
	programShadow = shaderLoader.CreateProgram("shaders/shader_shadow.vert", "shaders/shader_shadow.frag");
	programSkybox = shaderLoader.CreateProgram("shaders/sky_box.vert", "shaders/sky_box.frag");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	textureAsteroid = Core::LoadTexture("textures/asteroid2.png");
	renderModel = obj::loadModelFromFile("models/render.obj");
	textureEarth = Core::LoadTexture("textures/earth.png");
	textureEarthNormal = Core::LoadTexture("textures/earth_normalmap.png");
	cubeMapID = Core::setupCubeMap("textures/xpos.png", "textures/xneg.png", "textures/ypos.png", "textures/yneg.png", "textures/zpos.png", "textures/zneg.png");

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

	sphereModel.tangent = tangent;

	for (int i = 0; i < 10; i++)
	{
		glm::vec3 position = glm::ballRand(30.0f);
		float scale = glm::linearRand(0.5f, 5.0f);
		planets.push_back(glm::vec4(position, scale));
	}

	drawCircle(0, 3, 0, 5, 219);
	parallel_transport();
	initialise_particles(50);

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
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(programSkybox);
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
	glutCreateWindow("Niezwykle zaawansowana symulacja graficzna");
	glewInit();

	init();
	glutMotionFunc(mouseMove);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
