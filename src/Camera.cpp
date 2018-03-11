#include "Camera.h"

float Camera::yaw = 0;
float Camera::pitch = 0;
float Camera::roll = 0;

glm::mat4 Core::createPerspectiveMatrix(float zNear, float zFar)
{
	const float frustumScale = 1.1f;
	glm::mat4 perspective;
	perspective[0][0] = frustumScale;
	perspective[1][1] = frustumScale;
	perspective[2][2] = (zFar + zNear) / (zNear - zFar);
	perspective[3][2] = (2 * zFar * zNear) / (zNear - zFar);
	perspective[2][3] = -1;
	perspective[3][3] = 0;

	return perspective;
}

//glm::mat4 Core::createViewMatrix( glm::vec3 position, glm::vec3 forward, glm::vec3 up )
//{
//	glm::vec3 side = glm::cross(forward, up);
//
//	// Trzeba pamietac o minusie przy ustawianiu osi Z kamery.
//	// Wynika to z tego, ze standardowa macierz perspektywiczna zaklada, ze "z przodu" jest ujemna (a nie dodatnia) czesc osi Z.
//	glm::mat4 cameraRotation;
//	cameraRotation[0][0] = side.x; cameraRotation[1][0] = side.y; cameraRotation[2][0] = side.z;
//	cameraRotation[0][1] = up.x; cameraRotation[1][1] = up.y; cameraRotation[2][1] = up.z;
//	cameraRotation[0][2] = -forward.x; cameraRotation[1][2] = -forward.y; cameraRotation[2][2] = -forward.z;
//
//	glm::mat4 cameraTranslation;
//	cameraTranslation[3] = glm::vec4(-position, 1.0f);
//
//	return cameraRotation * cameraTranslation;
//}

glm::mat4 Core::createViewMatrix(glm::vec3 position, float yaw, float pitch, float roll)
{
	glm::mat4 Rx, Ry, Rz, M, T;
	glm::mat4 Id = glm::mat4();

	glm::quat qPitch = glm::angleAxis(pitch, glm::vec3(1, 0, 0));
	glm::quat qYaw = glm::angleAxis(yaw, glm::vec3(0, 1, 0));
	glm::quat qRoll = glm::angleAxis(roll, glm::vec3(0, 0, 1));

	glm::quat rotQuat = qYaw * qPitch * qRoll;

	glm::mat4 RotMatrix = glm::mat4_cast(rotQuat);

	//Rx = glm::rotate(Id, roll, glm::vec3(0.0f, 0.0f, 1.0f));
	//Ry = glm::rotate(Id, pitch, glm::vec3(0.0f, 1.0f, 0.0f));
	//Rz = glm::rotate(Id, yaw, glm::vec3(1.0f, 0.0f, 0.0f));

	T = glm::translate(Id, position);
	M = RotMatrix * T;

	return M;
}
