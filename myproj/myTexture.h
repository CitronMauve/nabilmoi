#pragma once
#include <GL/glew.h>
#include <string>

class myShader;

class myTexture
{
public:
	GLuint texture_id;
	GLenum texture_type;

	int width, height;

	myTexture();
	myTexture(std::string filename, GLenum type = GL_TEXTURE_2D);
	~myTexture();
	
	bool myTexture::readTexture(std::string filename, GLenum type = GL_TEXTURE_2D);
	void myTexture::bind(myShader *shader, std::string name, GLuint texture_offset = 0);
};

