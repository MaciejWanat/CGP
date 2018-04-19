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

Core::Shader_Loader shaderLoader;

obj::Model shipModel;
obj::Model sphereModel;

float cameraAngle = 0;
glm::vec3 cameraPos = glm::vec3(-5, 0, 0);
glm::vec3 cameraDir;

glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(1.0f, -0.9f, -1.0f));

//wektor punktów do naszej krzywej
//jeszcze nie porusza siê wzglêdem kierunku
glm::vec4 circle_points[220];
int pointCounter = 0;
glm::vec3 ship_pos;

//nasze piêkne cz¹steczki
struct Particle {
	glm::vec3 pos;
	glm::vec3 vel;
	glm::quat rot;
};

std::vector<Particle> spaceships;

glm::mat4 createTranslationMatrixXYZ(float X, float Y, float Z) {
	glm::mat4 translationMatrix;
	translationMatrix[3][0] = X;
	translationMatrix[3][1] = Y;
	translationMatrix[3][2] = Z;
	return translationMatrix;
}

void keyboard(unsigned char key, int x, int y)
{
	float angleSpeed = 0.1f;
	float moveSpeed = 0.1f;
	switch(key)
	{
	case 'z': cameraAngle -= angleSpeed; break;
	case 'x': cameraAngle += angleSpeed; break;
	case 'w': cameraPos += cameraDir * moveSpeed; break;
	case 's': cameraPos -= cameraDir * moveSpeed; break;
	case 'd': cameraPos += glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	case 'a': cameraPos -= glm::cross(cameraDir, glm::vec3(0,1,0)) * moveSpeed; break;
	}
}

glm::mat4 createCameraMatrix()
{
	// Obliczanie kierunku patrzenia kamery (w plaszczyznie x-z) przy uzyciu zmiennej cameraAngle kontrolowanej przez klawisze.
	cameraDir = glm::vec3(cosf(cameraAngle), 0.0f, sinf(cameraAngle));
	glm::vec3 up = glm::vec3(0,1,0);

	return Core::createViewMatrix(cameraPos, cameraDir, up);
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

void drawObjectTexture(obj::Model * model, glm::mat4 modelMatrix, glm::vec3 color)
{
	GLuint program = programTexture;

	glUseProgram(program);

	glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
	glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

	glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
	glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
	glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

	Core::DrawModel(model);

	glUseProgram(0);
}

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
		circleVerticesY[i] = y + (radius * sin(i * doublePi / numberOfSides));
		circleVerticesZ[i] = z;
	}

	for (int i = 0; i < numberOfVertices; i++)
	{
		circle_points[i].x = circleVerticesX[i];
		circle_points[i].y = circleVerticesY[i];
		circle_points[i].z = circleVerticesZ[i];
		circle_points[i].w = 0;
	}

	//for (int i = 0; i < numberOfVertices; i++)
	//{
	//	printf("%f %f %f \n", circle_points[i].x, circle_points[i].y, circle_points[i].z);
	//}
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
	glm::mat4 shipModelMatrix = createTranslationMatrixXYZ(ship_pos.x, ship_pos.y, ship_pos.z);
	drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));
	pointCounter++;

	//przemieszczanie stateczków
	for (int i = 0; i < spaceships.size(); i++)
	{
		glm::mat4 shipModelMatrix = glm::translate(spaceships[i].pos) * glm::scale(glm::vec3(0.10f));
		drawObjectColor(&shipModel, shipModelMatrix, glm::vec3(0.6f));

		glm::vec3 v1, v2, v3;
		//v1 = rule1: centre_of_mass = ship_pos;
		v1 = (ship_pos - spaceships[i].pos)/100;
		//v2 = rule2: keep a distance away from other objects
		for (int j = 0; j < spaceships.size(); j++)
		{
			v2 = glm::vec3(0, 0, 0);
			if (j != i) //boid which is near by
			{
				if (glm::length(spaceships[j].pos - spaceships[i].pos) < 0.08)
				{
					v2 -= (spaceships[j].pos - spaceships[i].pos);
				}
			}
		}
		//v3 = rule3: match velocity with near boids
		for (int j = 0; j < spaceships.size(); j++)
		{
			if (j != i)
			{
				v3 += spaceships[j].vel;
			}
		}
		v3 = v3 / (spaceships.size() - 1);
		v3 = (v3 - spaceships[i].vel) / 8;

		spaceships[i].vel += v1 + v2 + v3;
		spaceships[i].pos += spaceships[i].vel;
	}

	drawObjectTexture(&sphereModel, glm::translate(glm::vec3(2,0,2)), glm::vec3(0.8f, 0.2f, 0.3f));
	//drawObjectTexture(&sphereModel, glm::translate(glm::vec3(-2,0,-2)), glm::vec3(0.1f, 0.4f, 0.7f));

	glutSwapBuffers();
}

void init()
{
	glEnable(GL_DEPTH_TEST);
	programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
	programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");
	sphereModel = obj::loadModelFromFile("models/sphere.obj");
	shipModel = obj::loadModelFromFile("models/spaceship.obj");
	drawCircle(0, 0, 0, 10, 219);
	initialise_particles(50);
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
	glutInitWindowSize(600, 600);
	glutCreateWindow("OpenGL Pierwszy Program");
	glewInit();

	init();
	glutKeyboardFunc(keyboard);
	glutDisplayFunc(renderScene);
	glutIdleFunc(idle);

	glutMainLoop();

	shutdown();

	return 0;
}
