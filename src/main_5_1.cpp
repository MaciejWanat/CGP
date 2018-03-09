#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
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

float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));


float yaw = 0.0;
float pitch = 0.0;
float roll = 0.0;

void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	float moveSpeedUpDown = 0.3f;
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
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'c': cameraPos += glm::cross(cameraDir, glm::vec3(1, 0, 0)) * moveSpeedUpDown; break;
	case 'v': yaw  += 0.1; break;

	}
}

void mouse(int button, int state, int x, int y) {
	if (button == GLUT_MIDDLE_BUTTON && state == GLUT_DOWN)
	{
		//store the x,y value where the click happened
		puts("Middle button clicked");
	}

}

glm::mat4 createCameraMatrix()
{

	return Core::createViewMatrix(cameraPos,yaw, 1.0f, 1.0f);
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

	//zrob rotacje ksiezyca wokol siebie, odsun na odleglosc jaka by dzielila go od ziemi, zrob rotacje, przesun w miejsce ziemi, zrob rotacje wokol slonca

	// Macierz statku "przyczepia" go do kamery. Warto przeanalizowac te linijke i zrozumiec jak to dziala.
	glm::mat4 shipModelMatrix = glm::translate(cameraPos + cameraDir * 0.5f + glm::vec3(0,-0.25f,0)) * glm::rotate(-cameraAngle + glm::radians(90.0f), glm::vec3(0,1,0)) * glm::scale(glm::vec3(0.25f));
	//drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));
	//drawObjectColor(&sphereModel, glm::translate(glm::vec3(3.825, 0, 3.825)), glm::vec3(0.3f,0.4f,0.5f));
	
	drawObjectTexture(&shipModel, shipModelMatrix, ship);
	drawObjectTexture(&sphereModel, earthMatrix,  earth);
	drawSunObjectTexture(&sphereModel, sunMatrix, sun);
	drawObjectTexture(&sphereModel, moonMatrix, moon);
	drawObjectTexture(&sphereModel, marsMatrix, mars);
	drawObjectTexture(&sphereModel, merkuryMatrix, merkury);
	drawObjectTexture(&sphereModel, venusMatrix, venus);
	drawObjectTexture(&sphereModel, marsMoon1Matrix, moon);
	drawObjectTexture(&sphereModel, marsMoon2Matrix, moon);
	//drawObjectProceduralTexture(&sphereModel, glm::translate(glm::vec3(-2, 0, -2)), glm::vec3(1.0f, 0.5f, 0.2f));


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
	gLight1.attenuation = 0.005f;

	gLight2.intensities = glm::vec3(1, 1, 1);
	gLight2.position = glm::vec3(0.0f, -10.0f, 0.0f);
	gLight2.attenuation = 0.005f;

	init();
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
