#include <stdio.h>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "Main.h"

//������ ��� ����� �������������� :)

unsigned int DeltaTime(unsigned int* lastTick, unsigned int* firstTick)
{
	*firstTick = SDL_GetTicks();
	unsigned int deltaTime = *firstTick - *lastTick;
	*lastTick = *firstTick;
	return deltaTime;
}

SDL_Texture* LoadTexture(const char* filename, int* width, int* height, SDL_Renderer* renderer)
{
	// Loading an image
	// Here the surface is the information about the image. It contains the color data, width, height and other info.
	SDL_Surface* surface = IMG_Load(filename);
	if (!surface)
	{
		printf("Unable to load an image %s. Error: %s", filename, IMG_GetError());
		return nullptr;
	}

	// Now we use the renderer and the surface to create a texture which we later can draw on the screen.
	SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
	if (!texture)
	{
		printf("Unable to create a texture. Error: %s", SDL_GetError());
		return nullptr;
	}

	// In a moment we will get rid of the surface as we no longer need that. But let's keep the image dimensions.
	*width = surface->w;
	*height = surface->h;

	// Bye-bye the surface
	SDL_FreeSurface(surface);

	return texture;
}

int main()
{
	int screenWidth = 1980;
	int screenHeight = 1080;

	float x = screenWidth / 2;
	float y = screenHeight / 2;

	int currX = x;
	int currY = y;

	unsigned int firstTick = 0;
	unsigned int lastTick = 0;

	unsigned int deltaTime = 0;

	float speed = 0.75f;

#pragma region Init
	// Init SDL libraries
	SDL_SetMainReady(); // Just leave it be
	int result = 0;
	result = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO); // Init of the main SDL library
	if (result) // SDL_Init returns 0 (false) when everything is OK
	{
		printf("Can't initialize SDL. Error: %s", SDL_GetError()); // SDL_GetError() returns a string (as const char*) which explains what went wrong with the last operation
		return -1;
	}

	result = IMG_Init(IMG_INIT_PNG); // Init of the Image SDL library. We only need to support PNG for this project
	if (!(result & IMG_INIT_PNG)) // Checking if the PNG decoder has started successfully
	{
		printf("Can't initialize SDL image. Error: %s", SDL_GetError());
		return -1;
	}

	// Creating the window 1920x1080 (could be any other size)
	SDL_Window* window = SDL_CreateWindow("TheFirstGame",
		0, 0,
		screenWidth, screenHeight,
		SDL_WINDOW_SHOWN);

	if (!window)
		return -1;

	// Creating a renderer which will draw things on the screen
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		return -1;

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(renderer, 129, 162, 82, 255);

	const char* image_path = "tfg2.2.png";

	int tex_width = 0;
	int tex_height = 0;

	SDL_Texture* texture = LoadTexture(image_path, &tex_width, &tex_height, renderer);

#pragma endregion
	bool done = false;
	SDL_Event sdl_event;

	// The main loop
	// Every iteration is a frame
	while (!done)
	{
		// Polling the messages from the OS.
		// That could be key downs, mouse movement, ALT+F4 or many others
		while (SDL_PollEvent(&sdl_event))
		{
			if (sdl_event.type == SDL_QUIT) // The user wants to quit
			{
				done = true;
			}
			else if (sdl_event.type == SDL_KEYDOWN) // A key was pressed
			{
				switch (sdl_event.key.keysym.sym) // Which key?
				{
				case SDLK_ESCAPE: // Posting a quit message to the OS queue so it gets processed on the next step and closes the game
					SDL_Event event;
					event.type = SDL_QUIT;
					event.quit.type = SDL_QUIT;
					event.quit.timestamp = SDL_GetTicks();
					SDL_PushEvent(&event);
					break;
					// Other keys here
				default:
					break;
				}
			}
			else if (sdl_event.type == SDL_MOUSEBUTTONDOWN)
			{
				if (sdl_event.button.button == SDL_BUTTON_LEFT)
				{
					SDL_GetMouseState(&currX, &currY);
				}
			}
			// More events here?
		}

		// Clearing the screen
		SDL_RenderClear(renderer);

		// All drawing goes here

		// Let's draw a sample image
		deltaTime = DeltaTime(&lastTick, &firstTick);

		//GoLeft
		if (currX < x)
		{
			x -= speed * deltaTime;
		}
		//GoRight
		if (currX > x)
		{
			x += speed * deltaTime;
		}
		//GoUp
		if (currY > y)
		{
			y += speed * deltaTime;
		}
		//GoDown
		if (currY < y)
		{
			y -= speed * deltaTime;
		}

		// Here is the rectangle where the image will be on the screen
		SDL_Rect rect;
		rect.x = (int)round(x - tex_width / 2); // Counting from the image's center but that's up to you
		rect.y = (int)round(y - tex_height / 2); // Counting from the image's center but that's up to you
		rect.w = (int)tex_width;
		rect.h = (int)tex_height;

		SDL_RenderCopyEx(renderer, // Already know what is that
			texture, // The image
			nullptr, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
			&rect, // The destination rectangle on the screen.
			0, // An angle in degrees for rotation
			nullptr, // The center of the rotation (when nullptr, the rect center is taken)
			SDL_FLIP_NONE); // We don't want to flip the image

// Showing the screen to the player
		SDL_RenderPresent(renderer);

		// next frame...
	}

	SDL_DestroyTexture(texture);
	// If we reached here then the main loop stoped
	// That means the game wants to quit

	// Shutting down the renderer
	SDL_DestroyRenderer(renderer);

	// Shutting down the window
	SDL_DestroyWindow(window);

	// Quitting the Image SDL library
	IMG_Quit();
	// Quitting the main SDL library
	SDL_Quit();

	// Done.
	return 0;
}