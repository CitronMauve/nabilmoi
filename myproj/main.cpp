#include <fstream>
#include <string>
#include <vector>

#include <GL/glew.h>

#include <SDL2/SDL_main.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>
#undef main


#include "helperFunctions.h"

#include "myShader.h"
#include "myCamera.h"
#include "mySubObject.h"

#include <glm/glm.hpp>
#include <iostream>
#include "myObject.h"
#include "myLights.h"
#include "myFBO.h"
#include "default_constants.h"
#include "myScene.h"
#include "myShaders.h"

using namespace std;

// SDL variables
SDL_Window* window;
SDL_GLContext glContext;

int mouse_position[2];
bool mouse_button_pressed = false;
bool quit = false;
bool windowsize_changed = true;
bool crystalballorfirstperson_view = false;
float movement_stepsize = DEFAULT_KEY_MOVEMENT_STEPSIZE * 100;

// Camera parameters.
myCamera *cam1;

// All the meshes 
myScene scene;

//Triangle to draw to illustrate picking
size_t picked_triangle_index = 0;
myObject *picked_object = nullptr;

// Process the event.  
void processEvents(SDL_Event current_event)
{
	switch (current_event.type)
	{
		// window close button is pressed
	case SDL_QUIT:
	{
		quit = true;
		break;
	}
	case SDL_KEYDOWN:
	{
		if (current_event.key.keysym.sym == SDLK_ESCAPE)
			quit = true;
		if (current_event.key.keysym.sym == SDLK_r)
			cam1->reset();
		if (current_event.key.keysym.sym == SDLK_UP || current_event.key.keysym.sym == SDLK_w)
			cam1->moveForward(movement_stepsize);
		if (current_event.key.keysym.sym == SDLK_DOWN || current_event.key.keysym.sym == SDLK_s)
			cam1->moveBack(movement_stepsize);
		if (current_event.key.keysym.sym == SDLK_DOWN || current_event.key.keysym.sym == SDLK_a)
			cam1->moveLeft(movement_stepsize);
		if (current_event.key.keysym.sym == SDLK_DOWN || current_event.key.keysym.sym == SDLK_d)
			cam1->moveRight(movement_stepsize);
		if (current_event.key.keysym.sym == SDLK_LEFT || current_event.key.keysym.sym == SDLK_q)
			cam1->turnLeft(DEFAULT_LEFTRIGHTTURN_MOVEMENT_STEPSIZE * 15);
		if (current_event.key.keysym.sym == SDLK_RIGHT || current_event.key.keysym.sym == SDLK_e)
			cam1->turnRight(DEFAULT_LEFTRIGHTTURN_MOVEMENT_STEPSIZE * 15);
		if (current_event.key.keysym.sym == SDLK_v)
			crystalballorfirstperson_view = !crystalballorfirstperson_view;
		else if (current_event.key.keysym.sym == SDLK_o)
		{
			//nfdchar_t *outPath = NULL;
			//nfdresult_t result = NFD_OpenDialog("obj", NULL, &outPath);
			//if (result != NFD_OKAY) return;

			//myObject *obj_tmp = new myObject();
			//if (!obj_tmp->readObjects(outPath))
			//{
			//	delete obj_tmp;
			//	return;
			//}
			//delete obj1;
			//obj1 = obj_tmp;
		}
		break;
	}
	case SDL_MOUSEBUTTONDOWN:
	{
		mouse_position[0] = current_event.button.x;
		mouse_position[1] = current_event.button.y;
		mouse_button_pressed = true;

		const Uint8 *state = SDL_GetKeyboardState(nullptr);
		if (state[SDL_SCANCODE_LCTRL])
		{
			glm::vec3 ray = cam1->constructRay(mouse_position[0], mouse_position[1]);
			scene.closestObject(ray, cam1->camera_eye, picked_object, picked_triangle_index);
		}
		break;
	}
	case SDL_MOUSEBUTTONUP:
	{
		mouse_button_pressed = false;
		break;
	}
	case SDL_MOUSEMOTION:
	{
		int x = current_event.motion.x;
		int y = current_event.motion.y;

		int dx = x - mouse_position[0];
		int dy = y - mouse_position[1];

		mouse_position[0] = x;
		mouse_position[1] = y;

		if ((dx == 0 && dy == 0) || !mouse_button_pressed) return;

		if ((SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)) && crystalballorfirstperson_view)
			cam1->crystalball_rotateView(dx, dy);
		else if ((SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_LEFT)) && !crystalballorfirstperson_view)
			cam1->firstperson_rotateView(dx, dy);
		else if (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON(SDL_BUTTON_RIGHT))
			cam1->panView(dx, dy);

		break;
	}
	case SDL_WINDOWEVENT:
	{
		if (current_event.window.event == SDL_WINDOWEVENT_RESIZED)
			windowsize_changed = true;
		break;
	}
	case SDL_MOUSEWHEEL:
	{
		if (current_event.wheel.y < 0)
			cam1->moveBack(DEFAULT_MOUSEWHEEL_MOVEMENT_STEPSIZE);
		else if (current_event.wheel.y > 0)
			cam1->moveForward(DEFAULT_MOUSEWHEEL_MOVEMENT_STEPSIZE);
		break;
	}
	default:
		break;
	}
}

int main(int argc, char *argv[])
{
	// Initialize video subsystem
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);

	// Using OpenGL 3.1 core
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
	SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);

	// Create window
	window = SDL_CreateWindow("IN4I24", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	// Create OpenGL context
	glContext = SDL_GL_CreateContext(window);

	// Initialize glew
	glewInit();

	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);

	glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glEnable(GL_MULTISAMPLE);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);

	cam1 = new myCamera();
	SDL_GetWindowSize(window, &cam1->window_width, &cam1->window_height);


	checkOpenGLInfo(true);


	/**************************INITIALIZING LIGHTS ***************************/
	scene.lights = new myLights();
	scene.lights->lights.push_back(new myLight(glm::vec3(1, 0, 0), glm::vec3(0.5, 0.5, 0.5), myLight::POINTLIGHT));
	scene.lights->lights.push_back(new myLight(glm::vec3(0, 1, 0), glm::vec3(0.6, 0.6, 0.6), myLight::POINTLIGHT));


	/**************************INITIALIZING OBJECTS THAT WILL BE DRAWN ***************************/
	myObject *obj;
	obj = new myObject();
	if (!obj->readObjects("models/SYLT_Business_Wom-13_highpoly.obj", true, false))
		cout << "obj3 readScene failed.\n";
	obj->createmyVAO();
	scene.addObjects(obj, "ChristmasChallenge3");

	/**************************SETTING UP OPENGL SHADERS ***************************/
	myShaders shaders;
	shaders.addShader(new myShader("shaders/phong-vertexshader.glsl", "shaders/phong-fragmentshader.glsl"), "shader_phong");

	myShader *curr_shader;
	// display loop
	while (!quit)
	{
		if (windowsize_changed)
		{
			SDL_GetWindowSize(window, &cam1->window_width, &cam1->window_height);
			windowsize_changed = false;
		}

		//Computing transformation matrices. Note that model_matrix will be computed and set in the displayScene function for each object separately
		glViewport(0, 0, cam1->window_width, cam1->window_height);
		glm::mat4 projection_matrix = cam1->projectionMatrix();
		glm::mat4 view_matrix = cam1->viewMatrix();
		glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(view_matrix)));

		//Setting uniform variables for each shader
		for (unsigned int i=0;i<shaders.size();i++)
		{
			curr_shader = shaders[i];
			curr_shader->start();
			curr_shader->setUniform("myprojection_matrix", projection_matrix);
			curr_shader->setUniform("myview_matrix", view_matrix);
			scene.lights->setUniform(curr_shader, "Lights");
		}

		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		curr_shader = shaders["shader_phong"];
		curr_shader->start();
		scene["ChristmasChallenge3"]->displayObjects(curr_shader, view_matrix);


		SDL_GL_SwapWindow(window);

		SDL_Event current_event;
		while (SDL_PollEvent(&current_event) != 0)
			processEvents(current_event);
	}

	// Freeing resources before exiting.
	// Destroy window
	if (glContext) SDL_GL_DeleteContext(glContext);
	if (window) SDL_DestroyWindow(window);

	// Quit SDL subsystems
	SDL_Quit();

	return 0;
}