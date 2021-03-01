/**	OpenGL 3D Renderer

	TODO
	- Text rendering to textured quads
	- Quake-style console with extensible commands
	- Phong Lighting

*/

#include <stdio.h>
#include <string>
#include <cmath>
#include <vector>
#include <iostream>
#include <fstream>

#include <Windows.h>
#include <gl/glew.h>
#include <SDL.h>
#include <SDL_syswm.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "mkc.h" // defines and typedefs

// --- global variables  --- note: static variables are initialized to their default values
GLOBAL_VAR uint32 g_buffer_width;
GLOBAL_VAR uint32 g_buffer_height;

GLOBAL_VAR const uint8* g_keystate = nullptr;		// Stores keyboard state this frame. Access via g_keystate[SDL_Scancode].

GLOBAL_VAR int32 g_last_mouse_pos_x = INDEX_NONE;	// Stores mouse state this frame. mouse_pos is not updated when using SDL RelativeMouseMode.
GLOBAL_VAR int32 g_last_mouse_pos_y = INDEX_NONE;
GLOBAL_VAR int32 g_curr_mouse_pos_x = INDEX_NONE;
GLOBAL_VAR int32 g_curr_mouse_pos_y = INDEX_NONE;
GLOBAL_VAR int32 g_mouse_delta_x;
GLOBAL_VAR int32 g_mouse_delta_y;

// -------------------------

#include "camera.cpp"
#include "mesh.cpp"
#include "shader.cpp"
#include "texture.cpp"

const int32 WIDTH = 1280, HEIGHT = 720;

std::vector<Mesh*> meshes;
std::vector<Shader*> shaders;
Camera camera;
Texture tex_brick;
Texture tex_dirt;

static const char* vertex_shader_path = "shaders/default.vert";
static const char* frag_shader_path = "shaders/default.frag";

void create_triangles()
{
	uint32 indices[12] = { 
		0, 3, 1,
		1, 3, 2,
		2, 3, 0,
		0, 1, 2
	};

	GLfloat vertices[20] = {
	//	x 	  y     z    u    v
		-1.f, -1.f, 0.f, 0.f, 0.f,
		0.f, -1.f, 1.f, 0.5f, 0.f,
		1.f, -1.f, 0.f, 1.f, 0.f,
		0.f, 1.f, 0.f, 0.5f, 1.f
	};

	Mesh* tri = new Mesh();
	tri->create_mesh(vertices, indices, 20, 12);
	meshes.push_back(tri);
	tri = new Mesh();
	tri->create_mesh(vertices, indices, 20, 12);
	meshes.push_back(tri);
}

class Game
{
public:
	int8 run();					// Game start
private:
	bool init();				// Create window, set up OpenGL context, initialize SDL and GLEW
	void loop();				// Game loop with fixed timestep - Input, Logic, Render
		void process_events();	// Process input and SDL events
		void update(float dt);	// Tick game logic. Delta time is in seconds.
		void render();			// Process graphics and render them to the screen
	void clean_up();			// Clear memory and shut down
private:
	bool is_running = true;
	SDL_Window* window = nullptr;
	SDL_GLContext opengl_context = nullptr;
	HWND whandle = nullptr;	// Win32 API Window Handle
	glm::mat4 matrix_projection;
};

int8 Game::run()
{
	if (init() == false) return 1;

	create_triangles();

	camera = Camera();
	camera.set_position(0.f, 0.f, 0.f);
	camera.set_rotation(0.f, 270.f, 0.f);
	Shader* shader_ptr = new Shader();
	shader_ptr->create_from_files(vertex_shader_path, frag_shader_path);
	shaders.push_back(shader_ptr);
	tex_brick = Texture("data/textures/brick.png");
	tex_brick.load_texture();
	tex_dirt = Texture("data/textures/dirt.png");
	tex_dirt.load_texture();

	/** Going to create the projection matrix here because we only need to create projection matrix once (as long as fov or aspect ratio doesn't change)
		The model matrix, right now, is in Game::render because we want to be able to update the object's transform on tick. However, ideally, the 
		model matrix creation and transformation should be done in Game::update because that's where we should be updating the object's transformation.
		That matrix can be stored inside the game object class alongside the VAO. Or we could simply update the game object's position, rotation, scale
		fields, then construct the model matrix in Game::render based on those fields. Yeah that's probably better.
	*/
	float aspect_ratio = (float)g_buffer_width / (float)g_buffer_height;
	matrix_projection = glm::perspective(45.f, aspect_ratio, 0.1f, 1000.f);

	loop();
	clean_up();

	return 0;
}

bool Game::init()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("SDL failed to initialize.");
		return false;
	}

	// OpenGL Context Attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // version major
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3); // version minor
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE); // core means not backward compatible. not using deprecated code.
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG); // allow forward compatibility
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); // double buffering is on by default, but let's just call this anyway
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // depth buffer precision of 24 bits 

	// Setup SDL Window
	if ((window = SDL_CreateWindow(
		"test win",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WIDTH,
		HEIGHT,
		SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN
	)) == nullptr)
	{
		printf("SDL window failed to create.");
		return false;
	}

	//SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);

	/** GRABBING WINDOW INFORMATION - https://wiki.libsdl.org/SDL_GetWindowWMInfo
	*	Remarks: You must include SDL_syswm.h for the declaration of SDL_SysWMinfo. The info structure must 
		be initialized with the SDL version, and is then filled in with information about the given window, 
		as shown in the Code Example. */
	SDL_SysWMinfo sys_windows_info;
	SDL_VERSION(&sys_windows_info.version);
	if (SDL_GetWindowWMInfo(window, &sys_windows_info)) 
	{
		const char* subsystem = "an unknown system!";
		switch (sys_windows_info.subsystem) {
			case SDL_SYSWM_UNKNOWN:   break;
			case SDL_SYSWM_WINDOWS:   subsystem = "Microsoft Windows(TM)";  break;
			case SDL_SYSWM_X11:       subsystem = "X Window System";        break;
#if SDL_VERSION_ATLEAST(2, 0, 3)
			case SDL_SYSWM_WINRT:     subsystem = "WinRT";                  break;
#endif
			case SDL_SYSWM_DIRECTFB:  subsystem = "DirectFB";               break;
			case SDL_SYSWM_COCOA:     subsystem = "Apple OS X";             break;
			case SDL_SYSWM_UIKIT:     subsystem = "UIKit";                  break;
#if SDL_VERSION_ATLEAST(2, 0, 2)
			case SDL_SYSWM_WAYLAND:   subsystem = "Wayland";                break;
			case SDL_SYSWM_MIR:       subsystem = "Mir";                    break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 4)
			case SDL_SYSWM_ANDROID:   subsystem = "Android";                break;
#endif
#if SDL_VERSION_ATLEAST(2, 0, 5)
			case SDL_SYSWM_VIVANTE:   subsystem = "Vivante";                break;
#endif
		}

		SDL_Log("This program is running SDL version %u.%u.%u on %s", sys_windows_info.version.major, 
			sys_windows_info.version.minor, sys_windows_info.version.patch, subsystem);
	}
	else 
	{
		SDL_LogError(SDL_LOG_CATEGORY_ERROR, "Couldn't get window information: %s", SDL_GetError());
	}
	// Grab HWND from window info
	whandle = sys_windows_info.info.win.window;

	// Set context for SDL to use. Let SDL know that this window is the window that the OpenGL context should be tied to; everything that is drawn should be drawn to this window.
	if ((opengl_context = SDL_GL_CreateContext(window)) == nullptr)
	{
		printf("Failed to create OpenGL Context with SDL.");
		return false;
	}

	/**	This makes our Buffer Swap (SDL_GL_SwapWindow) synchronized with the monitor's vertical refresh - basically vsync; 
		0 = immediate, 1 = vsync, -1 = adaptive vsync. Remark: If application requests adaptive vsync and the system does 
		not support it, this function will fail and return -1. In such a case, you should probably retry the call with 1 
		for the interval. */
	if (SDL_GL_SetSwapInterval(-1) == -1)
	{
		SDL_GL_SetSwapInterval(1);
	}

	// Initialize GLEW
	glewExperimental = GL_TRUE; // Enable us to access modern opengl extension features
	if (glewInit() != GLEW_OK)
	{
		printf("GLEW failed to initialize.");
		clean_up();
		return false;
	}

	/** Get the size of window's underlying drawable in pixels (for use with glViewport).
		Remark: This may differ from SDL_GetWindowSize() if we're rendering to a high-DPI drawable, i.e. the window was created 
		with SDL_WINDOW_ALLOW_HIGHDPI on a platform with high-DPI support (Apple calls this "Retina"), and not disabled by the 
		SDL_HINT_VIDEO_HIGHDPI_DISABLED hint. */
	SDL_GL_GetDrawableSize(window, (int*)&g_buffer_width, (int*)&g_buffer_height);

	// Setup Viewport
	glViewport(0, 0, g_buffer_width, g_buffer_height);

	// NOTE: Enable depth test to see which triangles should be drawn over other triangles
	glEnable(GL_DEPTH_TEST); 
	// Lock mouse to window
	SDL_SetRelativeMouseMode(SDL_TRUE);
	// Grab keystate array
	g_keystate = SDL_GetKeyboardState(nullptr);

	return true;
}

void Game::clean_up()
{
	tex_brick.clear_texture();
	tex_dirt.clear_texture();
	// clear shaders and delete shaders
	for (size_t i = shaders.size() - 1; i > -1; --i)
	{
		shaders[i]->clear_shader();
		delete shaders[i];
	}
	shaders.clear();

	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(opengl_context);
	SDL_Quit();
}

void Game::loop()
{
	float last_tick = (float)SDL_GetTicks(); 	// time (in milliseconds since SDL init) of the last tick
	while (is_running)
	{
		process_events(); 						// process events
		if (is_running == false) { break; }
		float this_tick = (float)SDL_GetTicks();// Calculate time since last tick
		float delta_time_ms = this_tick - last_tick;
		update(delta_time_ms / 1000.f); 		// update game
		last_tick = this_tick;
		render();								// render game
	}
}

void Game::process_events()
{
	// Store Mouse state
	SDL_bool b_relative_mouse = SDL_GetRelativeMouseMode();
	if(b_relative_mouse)
	{
		SDL_GetRelativeMouseState(&g_mouse_delta_x, &g_mouse_delta_y);
	}
	else
	{
		g_last_mouse_pos_x = g_curr_mouse_pos_x;
		g_last_mouse_pos_y = g_curr_mouse_pos_y;
		SDL_GetMouseState(&g_curr_mouse_pos_x, &g_curr_mouse_pos_y);
		if (g_last_mouse_pos_x >= 0) { g_mouse_delta_x = g_curr_mouse_pos_x - g_last_mouse_pos_x; } else { g_mouse_delta_x = 0; }
		if (g_last_mouse_pos_y >= 0) { g_mouse_delta_y = g_curr_mouse_pos_y - g_last_mouse_pos_y; } else { g_mouse_delta_y = 0; }
	}

	// SDL Events
	SDL_Event event;
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
			case SDL_QUIT:
			{
				is_running = false;
			} break;

			case SDL_KEYDOWN:
			{
				// TODO register keydown for text input into dropdown console

				if (event.key.keysym.sym == SDLK_ESCAPE)
				{
					is_running = false;
					break;
				} 

				if (event.key.keysym.sym == SDLK_z)
				{
					SDL_SetRelativeMouseMode((SDL_bool) !b_relative_mouse);
					printf("mouse grab = %s\n", !b_relative_mouse ? "true" : "false");
				} 
			} break;
		}
	}
}

void Game::update(float dt)
{
	camera.update(dt);
}

void Game::render()
{
	// Clear opengl context's buffer
	//glClearColor(0.39f, 0.582f, 0.926f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// TODO Probably should make own shader for wireframe draws so that wireframe fragments aren't affected by lighting or textures
	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // GL_LINE for wireframe, GL_FILL to fill interior of polygon

	shaders[0]->use_shader();

		glUniformMatrix4fv(shaders[0]->get_matrix_view_location_id(), 1, GL_FALSE, glm::value_ptr(camera.calculate_viewmatrix()));
		glUniformMatrix4fv(shaders[0]->get_matrix_projection_location_id(), 1, GL_FALSE, glm::value_ptr(matrix_projection));

		glm::mat4 matrix_model = glm::mat4(1.f);
		matrix_model = glm::translate(matrix_model, glm::vec3(0.f, 0.5f, -1.3f));
		matrix_model = glm::scale(matrix_model, glm::vec3(0.3f, 0.3f, 0.3f)); // scale in each axis by the respective values of the vector
		glUniformMatrix4fv(shaders[0]->get_matrix_model_location_id(), 1, GL_FALSE, glm::value_ptr(matrix_model));
		tex_brick.use_texture();
		meshes[0]->render_mesh();

		matrix_model = glm::mat4(1.f);
		matrix_model = glm::translate(matrix_model, glm::vec3(0.f, -0.5f, -1.3f));
		matrix_model = glm::scale(matrix_model, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(shaders[0]->get_matrix_model_location_id(), 1, GL_FALSE, glm::value_ptr(matrix_model));
		tex_dirt.use_texture();
		meshes[1]->render_mesh();

	glUseProgram(0);

	/* Swap our buffer to display the current contents of buffer on screen. This is used with double-buffered OpenGL contexts, which are the default. */
	SDL_GL_SwapWindow(window);
}

int main(int argc, char* argv[]) // Our main entry point MUST be in this form when using SDL
{
	Game game;
	return game.run();
}
