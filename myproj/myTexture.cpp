#include "myTexture.h"
#include "helperFunctions.h"
#include "myShader.h"

#include <stdint.h>
#define STB_IMAGE_IMPLEMENTATION
#include "STB/stb_image.h"

myTexture::myTexture()
{
	texture_id = 0;
	texture_type = GL_TEXTURE_2D;
}

myTexture::myTexture(std::string filename, GLenum type)
{
	readTexture(filename, type);
}

myTexture::~myTexture()
{
	glDeleteTextures(1, &texture_id);
}


bool myTexture::readTexture(std::string filename, GLenum type)
{
	int size;
	GLubyte *mytexture = stbi_load(filename.c_str(), &width, &height, &size, 4);

	texture_type = type;
	glGenTextures(1, &texture_id);
	glBindTexture(texture_type, texture_id);

	glTexParameterf(texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	glTexParameterf(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(texture_type, 0, GL_RGBA, static_cast<GLuint>(width), static_cast<GLuint>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, mytexture);

	glGenerateMipmap(texture_type);

	delete[] mytexture;
	return true;
}


/*
bool myTexture::readTexture(std::string filename, GLenum type)
{
	GLubyte *mytexture = readPPMfile(filename.c_str(), width, height);

	texture_type = type;
	glGenTextures(1, &texture_id);
	glBindTexture(texture_type, texture_id);

	glTexParameterf(texture_type, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameterf(texture_type, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(texture_type, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf(texture_type, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(texture_type, 0, GL_RGBA, static_cast<GLuint>(width), static_cast<GLuint>(height), 0, GL_RGBA, GL_UNSIGNED_BYTE, mytexture);

	delete[] mytexture;
	return true;
}
*/

void myTexture::bind(myShader *shader, std::string name, GLuint texture_offset )
{
	glActiveTexture(GL_TEXTURE0 + texture_offset);
	glBindTexture(texture_type, texture_id);

	shader->setUniform(name, static_cast<int>(texture_offset));
}
