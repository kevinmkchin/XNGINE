#include <stdio.h>

#include <gl/glew.h>
#include <SDL.h>
#undef main

const GLint WIDTH = 1280, HEIGHT = 720;

class Game
{
public:
	int run();
private:
	int init();
	void loop();
	void clean_up();
	void process_events();
	void update(float dt);
	void render();
private:
	bool is_running = true;
	SDL_Window* window = nullptr;
	SDL_GLContext opengl_context;
	int buffer_width, buffer_height;
};

int Game::run()
{
	int init_val = init();
	if (init_val != 0) { return init_val; }
	loop();
	clean_up();

	return 0;
}

int Game::init()
{
	// Initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf("SDL failed to initialize.");
		return 1;
	}
	// OpenGL Context Attributes
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3); // version
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3); // version
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
		return 1;
	}

	// Set context for SDL to use. Let SDL know that this window is the window that the OpenGL context should be tied to; everything that is drawn should be drawn to this window.
	if ((opengl_context = SDL_GL_CreateContext(window)) == nullptr)
	{
		printf("Failed to create SDL GL Context.");
		return 1;
	}

	/*	This makes our Buffer Swap (SDL_GL_SwapWindow) synchronized with the monitor's vertical refresh - basically vsync; 0 = immediate, 1 = vsync, -1 = adaptive vsync.
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
		return 1;
	}

	/*  Get the size of window's underlying drawable in pixels (for use with glViewport).
		Remark: This may differ from SDL_GetWindowSize() if we're rendering to a high-DPI drawable, i.e. the window was created with SDL_WINDOW_ALLOW_HIGHDPI
		on a platform with high-DPI support (Apple calls this "Retina"), and not disabled by the SDL_HINT_VIDEO_HIGHDPI_DISABLED hint. */
	SDL_GL_GetDrawableSize(window, &buffer_width, &buffer_height);

	// Setup Viewport
	glViewport(0, 0, buffer_width, buffer_height);

	return 0;
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

}

void Game::render()
{
	// Clear opengl context's buffer
	glClearColor(0.39f, 0.582f, 0.926f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT);

	/* Swap our buffer to display the current contents of buffer on screen. This is used with double-buffered OpenGL contexts, which are the default. */
	SDL_GL_SwapWindow(window);
}

int main()
{
	Game game;
	return game.run();
}