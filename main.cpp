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
#include "myPhysics.h"
#include "myShaders.h"

using namespace std;

// SDL variables
SDL_Window* window;
SDL_GLContext glContext;

int mouse_position[2];
bool mouse_button_pressed = false;
bool quit = false;
bool windowsize_changed = true;
// bool crystalballorfirstperson_view = false;
float movement_stepsize = DEFAULT_KEY_MOVEMENT_STEPSIZE * 70;


// Camera parameters.
myCamera *cam1;

// All the meshes 
myScene scene;
myPhysics physics;

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
		{
			cam1->reset();
		}
		if (current_event.key.keysym.sym == SDLK_UP || current_event.key.keysym.sym == SDLK_w) {
			cam1->moveForward(movement_stepsize);
			((btRigidBody *)physics[scene["camera"]])->setLinearVelocity(
				btVector3(
					cam1->camera_forward.x * movement_stepsize,
					((btRigidBody *)physics[scene["camera"]])->getLinearVelocity().getY(),
					cam1->camera_forward.z * movement_stepsize
				)
			);
		}
		if (current_event.key.keysym.sym == SDLK_DOWN || current_event.key.keysym.sym == SDLK_s) {
			((btRigidBody *)physics[scene["camera"]])->setLinearVelocity(
				btVector3(
					-cam1->camera_forward.x * movement_stepsize,
					((btRigidBody *)physics[scene["camera"]])->getLinearVelocity().getY(),
					-cam1->camera_forward.z * movement_stepsize
				)
			);
		}
		if (current_event.key.keysym.sym == SDLK_LEFT || current_event.key.keysym.sym == SDLK_a) {
			((btRigidBody *)physics[scene["camera"]])->setLinearVelocity(
				btVector3(
					-glm::cross(cam1->camera_forward, cam1->camera_up).x * movement_stepsize,
					((btRigidBody *)physics[scene["camera"]])->getLinearVelocity().getY(),
					-glm::cross(cam1->camera_forward, cam1->camera_up).z * movement_stepsize
				)
			);
		}
		if (current_event.key.keysym.sym == SDLK_RIGHT || current_event.key.keysym.sym == SDLK_d) {
			((btRigidBody *)physics[scene["camera"]])->setLinearVelocity(
				btVector3(
					glm::cross(cam1->camera_forward, cam1->camera_up).x * movement_stepsize,
					((btRigidBody *)physics[scene["camera"]])->getLinearVelocity().getY(),
					glm::cross(cam1->camera_forward, cam1->camera_up).z * movement_stepsize
				)
			);
		}
		if (current_event.key.keysym.sym == SDLK_SPACE){
			if ((int)((btRigidBody *)physics[scene["camera"]])->getLinearVelocity().getY() == 0) {
				((btRigidBody *)physics[scene["camera"]])->setLinearVelocity(
					btVector3(
					((btRigidBody *)physics[scene["camera"]])->getLinearVelocity().getX(),
					cam1->camera_up.y * 10,
					((btRigidBody *)physics[scene["camera"]])->getLinearVelocity().getZ()
					)
				);
			}
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
		cam1->firstperson_rotateView(current_event.motion.xrel, current_event.motion.yrel);

		break;
	}
	case SDL_WINDOWEVENT:
	{
		if (current_event.window.event == SDL_WINDOWEVENT_RESIZED)
			windowsize_changed = true;
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
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
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
	
	// Lock the mouse within the window
	SDL_SetRelativeMouseMode(SDL_TRUE);

	// Initialize glew
	glewInit();

	{
		GLint major_version, minor_version;
		glGetIntegerv(GL_MAJOR_VERSION, &major_version);
		glGetIntegerv(GL_MAJOR_VERSION, &minor_version);
		cout << "OpenGL version available: " << major_version << "." << minor_version << endl;
	}

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

	//the big christmas scene.
	obj = new myObject();
	if (!obj->readObjects("models/ChristmasChallenge3.obj", true, false))
		cout << "obj3 readScene failed.\n";
	obj->createmyVAO();
	scene.addObject(obj, "ChristmasChallenge3");
	physics.addObject(obj, myPhysics::CONCAVE, btCollisionObject::CF_STATIC_OBJECT, 0.0f, 0.7f);


	// camera
	obj = new myObject();
	if (!obj->readObjects("models/basketball.obj", true, false))
		cout << "obj3 readScene failed.\n";
	obj->scaleObject(10.0f, 10.0f, 10.0f);
	obj->createmyVAO();
	obj->translate(glm::vec3(-20, 60,0));
	scene.addObject(obj, "camera");
	physics.addObject(obj, myPhysics::CONVEX, btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK, 1.0f, .0f);


	//ball
	obj = new myObject();
	obj->readObjects("models/basketball.obj", true, false);
	obj->scaleObject(10.0f, 10.0f, 10.0f);
	obj->createmyVAO();
	obj->translate(0.0f, 50.0f, 10.0f);
	scene.addObject(obj, "ball");
	physics.addObject(obj, myPhysics::CONVEX, btCollisionObject::CF_CUSTOM_MATERIAL_CALLBACK, 12.0f, 1.0f);




	/**************************SETTING UP OPENGL SHADERS ***************************/
	myShaders shaders;
	shaders.addShader(new myShader("shaders/basic-vertexshader.glsl", "shaders/basic-fragmentshader.glsl"), "shader_basic");
	shaders.addShader(new myShader("shaders/phong-vertexshader.glsl", "shaders/phong-fragmentshader.glsl"), "shader_phong");
	shaders.addShader(new myShader("shaders/texture-vertexshader.glsl", "shaders/texture-fragmentshader.glsl"), "shader_texture");
	shaders.addShader(new myShader("shaders/texture+phong-vertexshader.glsl", "shaders/texture+phong-fragmentshader.glsl"), "shader_texturephong");
	shaders.addShader(new myShader("shaders/bump-vertexshader.glsl", "shaders/bump-fragmentshader.glsl"), "shader_bump");
	shaders.addShader(new myShader("shaders/imageprocessing-vertexshader.glsl", "shaders/imageprocessing-fragmentshader.glsl"), "shader_imageprocessing");
	shaders.addShader(new myShader("shaders/environmentmap-vertexshader.glsl", "shaders/environmentmap-fragmentshader.glsl"), "shader_environmentmap");

	myShader *curr_shader;
	physics.setTime(SDL_GetPerformanceCounter() / static_cast<double>(SDL_GetPerformanceFrequency()));
	// display loop
	while (!quit)
	{
		if (windowsize_changed)
		{
			SDL_GetWindowSize(window, &cam1->window_width, &cam1->window_height);
			windowsize_changed = false;
		}

		physics.stepSimulation(SDL_GetPerformanceCounter() / static_cast<double>(SDL_GetPerformanceFrequency()));


		// camera
		physics.getModelMatrix(scene["camera"]);

		glm::vec4 tmp_vec = scene["camera"]->model_matrix * glm::vec4(0, 0, 0, 1.0f);
		cam1->camera_eye = glm::vec3(tmp_vec[0] / tmp_vec[3], tmp_vec[1] / tmp_vec[3], tmp_vec[2] / tmp_vec[3]);
		((btRigidBody *)physics[scene["camera"]])->setLinearVelocity(btVector3(0, ((btRigidBody *)physics[scene["camera"]])->getLinearVelocity().getY(), 0));

		physics.getModelMatrix(scene["ChristmasChallenge3"]);

		physics.getModelMatrix(scene["ball"]);
		

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
		scene["ball"]->displayObjects(curr_shader, view_matrix);


		curr_shader = shaders["shader_texturephong"];
		curr_shader->start();


		if (picked_object != nullptr)
		{
			curr_shader->setUniform("mymodel_matrix", picked_object->model_matrix);

			curr_shader->setUniform("input_color", glm::vec4(0, 1, 0, 1));
			picked_object->displayObjects(curr_shader, view_matrix);

			curr_shader->setUniform("input_color", glm::vec4(1, 0, 0, 1));
			glBegin(GL_TRIANGLES);
			{
				glVertex3fv(&(picked_object->vertices[picked_object->indices[picked_triangle_index][0]][0]));
				glVertex3fv(&(picked_object->vertices[picked_object->indices[picked_triangle_index][1]][0]));
				glVertex3fv(&(picked_object->vertices[picked_object->indices[picked_triangle_index][2]][0]));
			}
			glEnd();
		}
		/*-----------------------*/


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