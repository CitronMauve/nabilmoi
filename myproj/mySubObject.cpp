#include <math.h>
#include <iostream>

#include <GL/glew.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp> 
#include <glm/gtc/type_ptr.hpp>    

#include "mySubObject.h"
#include "myShader.h"

using namespace std;

mySubObject::mySubObject(myMaterial *m, size_t s, size_t e, string n = "noname")
{
	start = s;
	end = e;
	name = n;

	if (m != nullptr) material = new myMaterial(m);
	else material = new myMaterial();

	color_texture = nullptr;
	bump_texture = nullptr;
}

mySubObject::mySubObject(size_t s, size_t e, string n) : mySubObject(nullptr, s, e, n) { }

void mySubObject::setColorTexture(myTexture* t)
{
	color_texture = t;
}

void mySubObject::setBumpTexture(myTexture* t)
{
	bump_texture = t;
}


mySubObject::~mySubObject()
{

}

void mySubObject::displaySubObject(myVAO *vao, myShader *shader)
{
	if (end <= start) return;

	if (material != nullptr) material->setUniform(shader, "material");

	if (color_texture != nullptr)
	{
		shader->setUniform("totexture", 1);
		color_texture->bind(shader, "tex", 1);
	}
	else
	{
		shader->setUniform("tex", 11);
		shader->setUniform("totexture", 0);
	}

	if (bump_texture != nullptr) bump_texture->bind(shader, "bumptex", 2);
	else shader->setUniform("bumptex", static_cast<int>(11));
	
	if (vao != nullptr) vao->draw(start, end);
}

void mySubObject::displayNormals(myVAO* vao, myShader*)
{
}


