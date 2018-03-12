#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include "myVAO.h"
#include "myMaterial.h"
#include "myTexture.h"

class myShader;

class mySubObject
{
public:
	size_t start, end;
	myMaterial *material;
	myTexture *color_texture;
	myTexture *bump_texture;

	std::string name;

	mySubObject(myMaterial *m, size_t s, size_t e, std::string n) ;
	mySubObject(size_t s, size_t e, std::string);
	void setColorTexture(myTexture *t);
	void setBumpTexture(myTexture *t);
	~mySubObject();

	void displaySubObject(myVAO *vao, myShader *);
	void displayNormals(myVAO *vao, myShader *);
};
