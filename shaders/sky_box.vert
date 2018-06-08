#version 330 core
layout (location = 0) in vec4 aPos;

out vec3 TexCoords;

uniform mat4 modelViewProjectionMatrix;
uniform mat4 view;

void main()
{
    TexCoords = aPos.xyz;
    gl_Position = modelViewProjectionMatrix * aPos;
}  