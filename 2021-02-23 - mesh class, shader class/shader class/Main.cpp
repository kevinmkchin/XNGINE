#include <stdio.h>
#include <string.h>
#include <cmath>
#include <vector>

#include <gl/glew.h>
#include <SDL.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "mkc.h"
#include "Mesh.h"
#include "Shader.h"

const int32 WIDTH = 1280, HEIGHT = 720;

std::vector<Mesh*> meshes;
std::vector<Shader*> shaders;

// Just testing uniform variable
bool direction = true;
float tri_offset = 0.0f;
float tri_max_offset = 0.7f;
float tri_increment = 0.005f;

float curr_angle_degs = 0.f;

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

	GLfloat vertices[12] = {
		-1.f, -1.f, 0.f,
		0.f, -1.f, 1.f,
		1.f, -1.f, 0.f,
		0.f, 1.f, 0.f
	};

	Mesh* tri = new Mesh();
	tri->create_mesh(vertices, indices, 12, 12);
	meshes.push_back(tri);
	tri = new Mesh();
	tri->create_mesh(vertices, indices, 12, 12);
	meshes.push_back(tri);
}

class Game
{
public:
	// Game start
	int run();
private:
	// Game flow
	bool init();			// Create window, set up OpenGL context, initialize SDL and GLEW
	void loop();			// Game loop with fixed timestep - Input, Logic, Render
		void process_events();	// Process input events
		void update(float dt);	// Tick game logic
		void render();			// Process graphics and render them to the screen
	void clean_up();		// Clear memory and shut down
private:
	bool is_running = true;
	SDL_Window* window = nullptr;
	SDL_GLContext opengl_context;
	int buffer_width, buffer_height;
	glm::mat4 matrix_projection;
};

int Game::run()
{
	if (init() == false) return 1;

	create_triangles();

	Shader* shader_ptr = new Shader();
	shader_ptr->create_from_files(vertex_shader_path, frag_shader_path);
	shaders.push_back(shader_ptr);

	/** Going to create the projection matrix here because we only need to create projection matrix once (as long as fov or aspect ratio doesn't change)
		The model matrix, right now, is in Game::render because we want to be able to update the object's transform on tick. However, ideally, the 
		model matrix creation and transformation should be done in Game::update because that's where we should be updating the object's transformation.
		That matrix can be stored inside the game object class alongside the VAO. Or we could simply update the game object's position, rotation, scale
		fields, then construct the model matrix in Game::render based on those fields. Yeah that's probably better.
	*/
	float aspect_ratio = (float)buffer_width / (float)buffer_height;
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

	// Set context for SDL to use. Let SDL know that this window is the window that the OpenGL context should be tied to; everything that is drawn should be drawn to this window.
	if ((opengl_context = SDL_GL_CreateContext(window)) == nullptr)
	{
		printf("Failed to create OpenGL Context with SDL.");
		return false;
	}

	/**	This makes our Buffer Swap (SDL_GL_SwapWindow) synchronized with the monitor's vertical refresh - basically vsync; 0 = immediate, 1 = vsync, -1 = adaptive vsync.
		Remark: If application requests adaptive vsync and the system does not support it, this function will fail and return -1.
		In such a case, you should probably retry the call with 1 for the interval. */
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

	// NOTE: Enable depth test to see which triangles should be drawn over other triangles
	glEnable(GL_DEPTH_TEST); 

	/** Get the size of window's underlying drawable in pixels (for use with glViewport).
		Remark: This may differ from SDL_GetWindowSize() if we're rendering to a high-DPI drawable, i.e. the window was created with SDL_WINDOW_ALLOW_HIGHDPI
		on a platform with high-DPI support (Apple calls this "Retina"), and not disabled by the SDL_HINT_VIDEO_HIGHDPI_DISABLED hint. */
	SDL_GL_GetDrawableSize(window, &buffer_width, &buffer_height);

	// Setup Viewport
	glViewport(0, 0, buffer_width, buffer_height);

	return true;
}

void Game::clean_up()
{
	SDL_DestroyWindow(window);
	SDL_GL_DeleteContext(opengl_context);
	SDL_Quit();
}

void Game::loop()
{
	/*	There are multiple methods of creating a game loop with a fixed timestep, but I thoroughly believe my method is the best.
	I first employed this method in ADK, and I think it works excellently. My implementation allows the game to catch up if the
	game lags behind for some reason. Let's say our game does 1 tick per second. If, for example, it has been 3 seconds since the
	last tick, then we are behind by 3 ticks. We will execute all 3 ticks before moving on. */
	// Game Loop
	float ms_per_frame = 1000.f / (float)60;
	float last_tick = (float)SDL_GetTicks(); // time (in milliseconds since SDL init) of the last tick
	float ms_since_last_update = 0.f; // time in milliseconds since last update
	while (is_running)
	{
		// Calculate time since last tick
		float this_tick = (float)SDL_GetTicks();
		ms_since_last_update += this_tick - last_tick;
		last_tick = this_tick;

		while (ms_since_last_update > ms_per_frame)
		{
			ms_since_last_update -= ms_per_frame;

			// process events
			process_events();
			if (is_running == false) { break; }

			// update game
			update(ms_per_frame / 1000.f);

			// render game
			render();
		}
	}
}

void Game::process_events()
{
	SDL_Event event;
	if (SDL_PollEvent(&event) > 0)
	{
		switch (event.type)
		{
		case SDL_QUIT:
		{
			is_running = false;
			break;
		}
		case SDL_KEYDOWN:
		{
			if (event.key.keysym.sym == SDLK_ESCAPE)
			{
				is_running = false;
				break;
			}
		}
		}
	}
}

// Delta time is in seconds
void Game::update(float dt)
{
	if (direction)
	{
		tri_offset += tri_increment;
	}
	else
	{
		tri_offset -= tri_increment;
	}

	if (abs(tri_offset) >= tri_max_offset)
	{
		direction = !direction;
	}

	curr_angle_degs += 0.5f;
}

void Game::render()
{
	// Clear opengl context's buffer
	//glClearColor(0.39f, 0.582f, 0.926f, 1.f); // cornflower blue
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shaders[0]->use_shader();

		glm::mat4 matrix_model = glm::mat4(1.f); // Creates a Identity Matrix in R^4
		// order matters!
		// imagine that, in order to go from local/object space to world space, we translate the object away from it's local origin. 
		// this translation is essentially the object's position in the world. 
		// Think of every object, when created, is AT the world's origin, but we need to move it away from the world's origin to their
		// actual location in the world.
		matrix_model = glm::translate(matrix_model, glm::vec3(tri_offset, 0.5f, -1.3f));
		matrix_model = glm::scale(matrix_model, glm::vec3(0.3f, 0.3f, 0.3f)); // scale in each axis by the respective values of the vector
		glUniformMatrix4fv(shaders[0]->get_matrix_model_location_id(), 1, GL_FALSE, glm::value_ptr(matrix_model)); // need to use value_ptr bcs the matrix is not in format that is required
		glUniformMatrix4fv(shaders[0]->get_matrix_projection_location_id(), 1, GL_FALSE, glm::value_ptr(matrix_projection));
		meshes[0]->render_mesh();

		matrix_model = glm::mat4(1.f);
		matrix_model = glm::translate(matrix_model, glm::vec3(-tri_offset, -0.5f, -1.3f));
		matrix_model = glm::scale(matrix_model, glm::vec3(0.3f, 0.3f, 0.3f));
		glUniformMatrix4fv(shaders[0]->get_matrix_model_location_id(), 1, GL_FALSE, glm::value_ptr(matrix_model));
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