#include "Texture.h"

#include <fstream>
#include <iterator>
#include <vector>
#include "picopng.h"

typedef unsigned char byte;

GLuint Core::LoadTexture( const char * filepath )
{
	GLuint id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	std::ifstream input( filepath, std::ios::binary );
	std::vector<char> buffer((
		std::istreambuf_iterator<char>(input)), 
		(std::istreambuf_iterator<char>()));

	unsigned long w, h;
	std::vector<unsigned char> decoded;
	decodePNG(decoded, w, h, (unsigned char*)&buffer[0], buffer.size(), true);
	

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &decoded[0]);
	glGenerateMipmap(GL_TEXTURE_2D);

	return id;
}


GLuint Core::setupCubeMap(const char *xpos, const char *xneg, const char *ypos, const char *yneg, const char *zpos, const char *zneg)
{
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_CUBE_MAP);

	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);


	


	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	std::ifstream input(xpos, std::ios::binary);
	std::vector<char> buffer((std::istreambuf_iterator<char>(input)),(std::istreambuf_iterator<char>()));
	unsigned long w, h;
	std::vector<unsigned char> decoded;
	decodePNG(decoded, w, h, (unsigned char*)&buffer[0], buffer.size(), true);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &decoded[0]);
	
	std::ifstream input1(xneg, std::ios::binary);
	std::vector<char> buffer1((std::istreambuf_iterator<char>(input1)),(std::istreambuf_iterator<char>()));
	decodePNG(decoded, w, h, (unsigned char*)&buffer1[0], buffer1.size(), true);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &decoded[0]);

	std::ifstream input2(ypos, std::ios::binary);
	std::vector<char> buffer2((std::istreambuf_iterator<char>(input2)), (std::istreambuf_iterator<char>()));
	decodePNG(decoded, w, h, (unsigned char*)&buffer2[0], buffer2.size(), true);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &decoded[0]);

	std::ifstream input3(yneg, std::ios::binary);
	std::vector<char> buffer3((std::istreambuf_iterator<char>(input3)), (std::istreambuf_iterator<char>()));
	decodePNG(decoded, w, h, (unsigned char*)&buffer3[0], buffer3.size(), true);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &decoded[0]);

	std::ifstream input4(zpos, std::ios::binary);
	std::vector<char> buffer4((std::istreambuf_iterator<char>(input4)), (std::istreambuf_iterator<char>()));
	decodePNG(decoded, w, h, (unsigned char*)&buffer4[0], buffer4.size(), true);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &decoded[0]);

	std::ifstream input5(zneg, std::ios::binary);
	std::vector<char> buffer5((std::istreambuf_iterator<char>(input5)), (std::istreambuf_iterator<char>()));
	decodePNG(decoded, w, h, (unsigned char*)&buffer5[0], buffer5.size(), true);
	glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, &decoded[0]);
	
	return textureID;
}




void Core::SetActiveTexture(GLuint textureID, const char * shaderVariableName, GLuint programID, int textureUnit)
{
	glUniform1i(glGetUniformLocation(programID, shaderVariableName), textureUnit);
	glActiveTexture(GL_TEXTURE0 + textureUnit);
	glBindTexture(GL_TEXTURE_2D, textureID);
}
