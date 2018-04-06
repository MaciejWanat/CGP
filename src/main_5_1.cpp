#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include "gtx/spline.hpp"
#include <iostream>
#include <cmath>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"

GLuint programColor;
GLuint programTexture;
GLuint sunTexture;
GLuint earth;
GLuint sun;
GLuint grid;
GLuint merkury;
GLuint moon;
GLuint venus;
GLuint mars;
GLuint stars;
GLuint ship;


struct Particle {
	glm::vec3 pos;
	glm::vec3 vel;
	glm::quat rot;
};

std::vector<Particle> spaceships;


struct Light {
	glm::vec3 position;
	glm::vec3 intensities; //color of the light
	float attenuation;
};

Light gLight1;
Light gLight2;

Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;

float cameraAngleX = 4.7;
float cameraAngleY = 5;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

float yaw = 0.0;
float pitch = 0.0;
float roll = 0.0;

float oldX = 0;
float oldY = 0;

glm::vec4 points[250];
int pointCounter = 0;
float vertexArray[12];

void keyboard(unsigned char key, int x, int y)
{	
	float moveSpeed = 0.1f;
	float moveSpeedUpDown = 0.3f;
	float angleSpeed = 0.1f;

	switch(key)
	{
	case '1': gLight1.intensities = glm::vec3(1, 1, 1); break;
	case '2': gLight1.intensities = glm::vec3(1, 0, 1); break;
	case '3': gLight1.intensities = glm::vec3(0, 1, 0); break;
	case '4': gLight1.intensities = glm::vec3(0, 0, 1); break;
	case '5': gLight1.position = glm::vec3(0.0f, 10.0f, 0.0f); break;
	case '6': gLight1.position = glm::vec3(0.0f, -10.0f, 0.0f); break;
	case '7': gLight1.position = glm::vec3(10.0f, 0.0f, 0.0f); break;
	case '8': gLight1.position = glm::vec3(-10.0f, 0.0f, 0.0f); break;
	case '0': gLight1.position = glm::vec3(0.0f, 0.0f, 0.0f); gLight1.intensities = glm::vec3(1, 1, 1); break;
	case 'z': cameraAngleX -= angleSpeed; break;
	case 'x': cameraAngleX += angleSpeed; break;
	case 's': cameraPos += cameraDir * moveSpeed; break;
	case 'w': cameraPos -= cameraDir * moveSpeed; break;
	case 'a': cameraPos += glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'd': cameraPos -= glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'c': cameraPos += glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeedUpDown; break;
	case 'v': cameraPos -= glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeedUpDown; break;
	}
}

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

glm::mat4 createCameraMatrix()
{
	cameraDir = glm::vec3(cosf(cameraAngleX), 0.0f, sinf(cameraAngleX));

	return Core::createViewMatrix(cameraPos, yaw, pitch, roll);
}


void drawCurve(glm::vec4 v1, glm::vec4 t1, glm::vec4 v2, glm::vec4 t2, int points_in) 
{
	points[0] = v1;
	points[points_in] = v2;
	float step = 1.0 / (float)points_in;
	float genPoint = 0.0;

	for (int j = 1; j < points_in; j++)
	{
		points[j] = glm::hermite(v1, t1, v2, t2, genPoint);
		genPoint += step;
	}
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

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint zmienna)
{
	GLuint program = programTexture;

	glUseProgram(program);

	Core::SetActiveTexture(zmienna, "tex", program, 1);
	glUniform3f(glGetUniformLocation(program, "light1.position"), gLight1.position.x,gLight1.position.y,gLight1.position.z);
	glUniform3f(glGetUniformLocation(program, "light1.intensities"), gLight1.intensities.x, gLight1.intensities.y, gLight1.intensities.z);
	glUniform1f(glGetUniformLocation(program, "light1.attenuation"), gLight1.attenuation);

//	glUniform3f(glGetUniformLocation(program, "light2.position"), gLight2.position.x, gLight2.position.y, gLight2.position.z);
//	glUniform3f(glGetUniformLocation(program, "light2.intensities"), gLight2.intensities.x, gLight2.intensities.y, gLight2.intensities.z);
//	glUniform1f(glGetUniformLocation(program, "light2.attenuation"), gLight2.attenuation);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

void drawSunObjectTexture(obj::Model * model, glm::mat4 modelMatrix, GLuint zmienna)
{
	GLuint program = sunTexture;

	glUseProgram(program);

	Core::SetActiveTexture(zmienna, "tex", program, 1);
	glUniform3f(glGetUniformLocation(program, "light1.position"), gLight1.position.x, gLight1.position.y, gLight1.position.z);
	glUniform3f(glGetUniformLocation(program, "light1.intensities"), gLight1.intensities.x, gLight1.intensities.y, gLight1.intensities.z);

	//glUniform3f(glGetUniformLocation(program, "light2.position"), gLight2.position.x, gLight2.position.y, gLight2.position.z);
	//glUniform3f(glGetUniformLocation(program, "light2.intensities"), gLight2.intensities.x, gLight2.intensities.y, gLight2.intensities.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "model"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

glm::mat4 createScalingMatrix(float number) {
	glm::mat4 scalingMatrix;
	scalingMatrix[0][0] = number;
	scalingMatrix[1][1] = number;
	scalingMatrix[2][2] = number;
	return scalingMatrix;
}

glm::mat4 createRotationMatrix(float time) {
	glm::mat4 rotationMatrix;
	rotationMatrix[0][0] = cos(time);
	rotationMatrix[0][2] = -sin(time);
	rotationMatrix[2][0] = sin(time);
	rotationMatrix[2][2] = cos(time);
	return rotationMatrix;
}

glm::mat4 createTranslationMatrix(float number) {
	glm::mat4 translationMatrix;
	translationMatrix[3][0] = number;
	translationMatrix[3][1] = 0.0f;
	translationMatrix[3][2] = number;
	return translationMatrix;

}

glm::mat4 createTranslationMatrixXYZ(float X, float Y, float Z) {
	glm::mat4 translationMatrix;
	translationMatrix[3][0] = X;
	translationMatrix[3][1] = Y;
	translationMatrix[3][2] = Z;
	return translationMatrix;

}


void renderScene()
{
	float time = glutGet(GLUT_ELAPSED_TIME) / 1000.0f;
	cameraMatrix = createCameraMatrix();
	perspectiveMatrix = Core::createPerspectiveMatrix();


	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	
	// Switch to orthographic view for background drawing
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	gluOrtho2D(0.0f, 1.0f, 0.0f, 1.0f);
	glEnable(GL_TEXTURE_2D);
	glMatrixMode(GL_MODELVIEW);
	glBindTexture(GL_TEXTURE_2D, stars);    // Background texture


	// No depth buffer writes for background
	glDepthMask(GL_FALSE);

	// Background image
	glBegin(GL_QUADS);
	glTexCoord2f(0.0f, 0.0f);
	glVertex2f(0.0f, 0.0f);

	glTexCoord2f(1.0f, 0.0f);
	glVertex2f(1.0f, 0.0f);

	glTexCoord2f(1.0f, 1.0f);
	glVertex2f(1.0f, 1.0f);

	glTexCoord2f(0.0f, 1.0f);
	glVertex2f(0.0f, 1.0f);
	glEnd();

	// Back to 3D land
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glDepthMask(GL_TRUE);
	
	//S£OÑECZKO
	glm::mat4 sunTranslationMatrix = createTranslationMatrix(0.0f);

	glm::mat4 sunRotationMatrix = createRotationMatrix(time);
	glm::mat4 sunScalingMatrix = createScalingMatrix(3.0f);
	glm::mat4 sunMatrix = sunRotationMatrix * sunTranslationMatrix * sunScalingMatrix;

	//MERKURY
	glm::mat4 merkuryTranslationMatrix = createTranslationMatrix(3.8f);
	glm::mat4 merkuryRotationMatrix = createRotationMatrix(time/2);
	glm::mat4 merkuryScalingMatrix = createScalingMatrix(0.38f);
	glm::mat4 merkuryAroundSunMatrix = createRotationMatrix(time);
	glm::mat4 merkuryMatrix = merkuryAroundSunMatrix * merkuryTranslationMatrix * merkuryRotationMatrix * merkuryScalingMatrix;

	//WENUS
	glm::mat4 venusTranslationMatrix = createTranslationMatrix(7.2f);
	glm::mat4 venusRotationMatrix = createRotationMatrix(time*(0.35f));
	glm::mat4 venusScalingMatrix = createScalingMatrix(0.94f);
	glm::mat4 venusAroundSunMatrix = createRotationMatrix(time*0.15f);
	glm::mat4 venusMatrix = venusAroundSunMatrix * venusTranslationMatrix * venusRotationMatrix * venusScalingMatrix;

	//MARS
	glm::mat4 marsTranslationMatrix = createTranslationMatrix(15.0f);
	glm::mat4 marsRotationMatrix = createRotationMatrix(time);
	glm::mat4 marsScalingMatrix = createScalingMatrix(0.53f);
	glm::mat4 marsAroundSunMatrix = createRotationMatrix(time/8);
	glm::mat4 marsMatrix = marsAroundSunMatrix * marsTranslationMatrix * marsRotationMatrix * marsScalingMatrix;

	//ZIEMIA
	glm::mat4 earthTranslationMatrix = createTranslationMatrix(10.0f);
	glm::mat4 earthRotationMatrix = createRotationMatrix(time);
	glm::mat4 earthAroundSunMatrix = createRotationMatrix(time/4);
	glm::mat4 earthMatrix = earthAroundSunMatrix * earthTranslationMatrix * earthRotationMatrix;

	//KSIÊ¯YC
	glm::mat4 moonTranslationMatrix = createTranslationMatrix(1.0f);
	glm::mat4 moonRotationMatrix = createRotationMatrix(time*(0.75f));
	glm::mat4 moonRotationAroundEarthMatrix = createRotationMatrix(time*(0.75f));
	glm::mat4 moonScalingMatrix = createScalingMatrix(0.2f);
	glm::mat4 moonMatrix = earthAroundSunMatrix * earthTranslationMatrix * moonRotationAroundEarthMatrix * moonTranslationMatrix * moonRotationMatrix * moonScalingMatrix;

	//MARS KSIEZYC 1
	glm::mat4 marsMoon1TranslationMatrix = createTranslationMatrix(1.0f);
	glm::mat4 marsMoon1RotationMatrix = createRotationMatrix(time*(0.75f));
	glm::mat4 marsMoon1RotationAroundMarsMatrix = createRotationMatrix(time*(0.75f));
	glm::mat4 marsMoon1ScalingMatrix = createScalingMatrix(0.18f);
	glm::mat4 marsMoon1Matrix = marsAroundSunMatrix * marsTranslationMatrix * marsMoon1RotationAroundMarsMatrix * marsMoon1TranslationMatrix * marsMoon1RotationMatrix * marsMoon1ScalingMatrix;


	//MARS KSIEZYC 2
	glm::mat4 marsMoon2TranslationMatrix = createTranslationMatrix(2.0f);
	glm::mat4 marsMoon2RotationMatrix = createRotationMatrix(time*(0.25f));
	glm::mat4 marsMoon2RotationAroundMarsMatrix = createRotationMatrix(time*(0.25f));
	glm::mat4 marsMoon2ScalingMatrix = createScalingMatrix(0.12f);
	glm::mat4 marsMoon2Matrix = marsAroundSunMatrix * marsTranslationMatrix * marsMoon2RotationAroundMarsMatrix * marsMoon2TranslationMatrix * marsMoon2RotationMatrix * marsMoon2ScalingMatrix;

	//drawObjectTexture(&sphereModel, earthMatrix, earth);
	//drawSunObjectTexture(&sphereModel, sunMatrix, sun);
	//drawObjectTexture(&sphereModel, moonMatrix, moon);
	//drawObjectTexture(&sphereModel, marsMatrix, mars);
	//drawObjectTexture(&sphereModel, merkuryMatrix, merkury);
	//drawObjectTexture(&sphereModel, venusMatrix, venus);
	//drawObjectTexture(&sphereModel, marsMoon1Matrix, moon);
	//drawObjectTexture(&sphereModel, marsMoon2Matrix, moon);
	
	//zrob rotacje ksiezyca wokol siebie, odsun na odleglosc jaka by dzielila go od ziemi, zrob rotacje, przesun w miejsce ziemi, zrob rotacje wokol slonca

	// Macierz statku "przyczepia" go do kamery. Warto przeanalizowac te linijke i zrozumiec jak to dziala.
	//glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0,-0.25f,0)) * glm::rotate(-cameraAngleX + glm::radians(90.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));

	pointCounter++;

	glm::mat4 shipModelMatrix = createTranslationMatrixXYZ(points[pointCounter % 220].x, points[pointCounter % 220].y, points[pointCounter % 220].z);
	drawSunObjectTexture(&shipModel, shipModelMatrix, ship);


	for (int i = 0; i < spaceships.size(); i++)
	{
		spaceships[i].vel += (spaceships[i].pos - glm::vec3(points[pointCounter % 220].x, points[pointCounter % 220].y, points[pointCounter % 220].z))*0.05;
		spaceships[i].pos += spaceships[i].vel * 0.05;

		glm::mat4 shipModelMatrix = glm::translate(spaceships[i].pos) * glm::scale(glm::vec3(0.25f));
		drawSunObjectTexture(&shipModel, shipModelMatrix, ship);

	}


	//drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));
	//drawObjectColor(&sphereModel, glm::translate(glm::vec3(3.825, 0, 3.825)), glm::vec3(0.3f,0.4f,0.5f));
	//drawObjectProceduralTexture(&sphereModel, glm::translate(glm::vec3(-2, 0, -2)), glm::vec3(1.0f, 0.5f, 0.2f));

	//for (int i = 0; i < 220; i++)
	//{
	//	vertexArray[0] = points[i].x;
	//	vertexArray[1] = points[i].y;
	//	vertexArray[2] = points[i].z;
	//	vertexArray[3] = points[i].w;
	//	vertexArray[4] = points[i + 1].x;
	//	vertexArray[5] = points[i + 1].y;
	//	vertexArray[6] = points[i + 1].z;
	//	vertexArray[7] = points[i + 1].w;
	//	vertexArray[8] = points[i].x + 0.1;
	//	vertexArray[9] = points[i].y + 0.1;
	//	vertexArray[10] = points[i].z + 0.1;
	//	vertexArray[11] = points[i].w;
	//	Core::DrawVertexArray(vertexArray, 3, 4);

	//	vertexArray[0] = points[i + 1].x;
	//	vertexArray[1] = points[i + 1].y;
	//	vertexArray[2] = points[i + 1].z;
	//	vertexArray[3] = points[i + 1].w;
	//	vertexArray[4] = points[i].x + 0.1;
	//	vertexArray[5] = points[i].y + 0.1;
	//	vertexArray[6] = points[i].z + 0.1;
	//	vertexArray[7] = points[i].w;
	//	vertexArray[8] = points[i + 1].x + 0.1;
	//	vertexArray[9] = points[i + 1].y + 0.1;
	//	vertexArray[10] = points[i + 1].z + 0.1;
	//	vertexArray[11] = points[i + 1].w;
	//	Core::DrawVertexArray(vertexArray, 3, 4);
	//}

	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	sunTexture = shaderLoader.CreateProgram("shaders/shader_tex_sun.vert", "shaders/shader_tex_sun.frag");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	earth = Core::LoadTexture("textures/earth2.png");
	sun = Core::LoadTexture("textures/sun.png");
	merkury = Core::LoadTexture("textures/mercury.png");
	mars = Core::LoadTexture("textures/mars.png");
	moon = Core::LoadTexture("textures/moon.png");
	venus = Core::LoadTexture("textures/venus.png");
	stars = Core::LoadTexture("textures/stars2.png");
	ship = Core::LoadTexture("textures/spaceship.png");
	drawCurve(glm::vec4(0, 0, 0, 1), glm::vec4(10, 0, 0, 1), glm::vec4(0, 5, 0, 0), glm::vec4(0, -6, 0, 0), 220);
	for (int i = 0; i < 50; i++) {
		Particle x;
		float rx = -2.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0 - (-2.0))));
		float ry = -2.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0 - (-2.0))));
		float rz = -2.0 + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (2.0 - (-2.0))));
		x.pos = glm::vec3(rx, ry, rz);
		x.vel = glm::vec3(1, 1, 1);
		spaceships.push_back(x);
	}
}

void shutdown()
{
	shaderLoader.DeleteProgram(programColor);
	shaderLoader.DeleteProgram(programTexture);
	shaderLoader.DeleteProgram(sunTexture);
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
	glutInitWindowSize(1024,1024);
	glutCreateWindow("Uk³ad S³oneczny");
	glewInit();

	gLight1.intensities = glm::vec3(1, 1, 1);
	gLight1.position = glm::vec3(0.0f, 0.0f, 0.0f);
	gLight1.attenuation = 0.000f;

	gLight2.intensities = glm::vec3(1, 1, 1);
	gLight2.position = glm::vec3(0.0f, -10.0f, 0.0f);
	gLight2.attenuation = 0.000f;

	init();
	glutMotionFunc(mouseMove);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
