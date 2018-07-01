#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec3 vertexTangent;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 modelMatrix;
uniform mat4 lightMatrix;

out vec3 interpNormal;
out vec2 interpTexCoord;
out vec3 vertexWorldPosition;
out vec4 lightPos;
out mat3 TBN;

void main()
{
	gl_Position = modelViewProjectionMatrix * vec4(vertexPosition, 1.0);
	interpNormal = (modelMatrix * vec4(vertexNormal, 0.0)).xyz;
	vertexWorldPosition = (modelMatrix * vec4(vertexPosition, 1.0)).xyz;
	interpTexCoord = vertexTexCoord;
	vec3 T = normalize(vec3(modelMatrix * vec4(vertexTangent,   0.0)));
	vec3 N = normalize(vec3(modelMatrix * vec4(interpNormal,    0.0)));
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);
}
