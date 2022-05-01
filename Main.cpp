#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "Main.h"

//This project will be continue :)

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

struct Vector_2
{
	int x;
	int y;
};

struct Barrier
{
	SDL_Texture* texture;
	Vector_2 size;
	void Init(SDL_Texture* tex, int width, int height);
	void Rendering(SDL_Renderer* renderer, double x, double y);
	void Destroy();
};

void Barrier::Init(SDL_Texture* tex, int width, int height)
{
	texture = tex;
	size.x = width;
	size.y = height;
}

void Barrier::Rendering(SDL_Renderer* renderer, double x, double y)
{
	SDL_Rect rect;
	rect.x = (int)round(x * 1920 / 15);
	rect.y = (int)round(y * 1080 / 11);
	rect.w = (int)size.x;
	rect.h = (int)size.y;

	SDL_RenderCopyEx(renderer,
		texture,
		nullptr,
		&rect,
		0,
		nullptr,
		SDL_FLIP_NONE);
}

void Barrier::Destroy()
{
	SDL_DestroyTexture(texture);
}


struct Player
{
	Barrier Barrier;
	Vector_2 pos;
	void Init(SDL_Texture* tex, int width, int height, int PositionX, int PositionY);
	void Rendering(SDL_Renderer* renderer);
	void Destroy();
};

void Player::Init(SDL_Texture* tex, int width, int height, int PositionX, int PositionY)
{
	Barrier.Init(tex, width, height);
	pos.x = PositionX;
	pos.y = PositionY;
}

void Player::Rendering(SDL_Renderer* renderer)
{
	Barrier.Rendering(renderer, pos.x, pos.y);
}

void Player::Destroy()
{
	SDL_DestroyTexture(Barrier.texture);
}

struct List
{
	List* PastElement;
	unsigned char X;
	unsigned char Y;
};

struct Stack
{
	List* LastElement = nullptr;
	void AddElement(int x, int y);
	void DeleteLastElement();
	void Clear();
};

void Stack::AddElement(int x, int y)
{
	List* NewElement = (List*)malloc(sizeof(List));
	NewElement->PastElement = LastElement;
	NewElement->X = x;
	NewElement->Y = y;
	LastElement = NewElement;
}

void Stack::DeleteLastElement()
{
	if (LastElement)
	{
		List* PastElement = LastElement->PastElement;
		free(LastElement);
		LastElement = PastElement;
	}
	else
	{
		printf("You Delete non-existent element!");
		abort();
	}
}

void Stack::Clear()
{
	while (LastElement)
	{
		DeleteLastElement();
	}
}


int main()
{
#pragma region InitSDL
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

#pragma region DisplaySize

	SDL_DisplayMode display;

	for (int i = 0; i < SDL_GetNumVideoDisplays(); ++i) {

		int should_be_zero = SDL_GetCurrentDisplayMode(i, &display);

		if (should_be_zero != 0)
			// In case of error...
			SDL_Log("Could not get display mode for video display #%d: %s", i, SDL_GetError());

		else
			// On success, print the current display mode.
			SDL_Log("Display #%d: current display mode is %dx%dpx @ %dhz.", i, display.w, display.h, display.refresh_rate);

	}
#pragma endregion

	// Creating the window 1920x1080 (could be any other size)
	SDL_Window* window = SDL_CreateWindow("TheFirstGame",
		0, 0,
		display.w, display.h,
		SDL_WINDOW_SHOWN);

	if (!window)
		return -1;
	// Creating a renderer which will draw things on the screen
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	if (!renderer)
		return -1;

	// Setting the color of an empty window (RGBA). You are free to adjust it.
	SDL_SetRenderDrawColor(renderer, 129, 162, 82, 255);

#pragma endregion

	int screenWidth = display.w;
	int screenHeight = display.h;

	int x = screenWidth / 15;
	int y = screenHeight / 11;

	int currX = x;
	int currY = y;

	unsigned int firstTick = 0;
	unsigned int lastTick = 0;

	unsigned int deltaTime = 0;

	float speed = 0.5f;

	const char* image_path = "tfg2.2.png";

	int tex_width = 0;
	int tex_height = 0;

	int widthInPixels = display.w / 15;
	int heightInPixels = display.h / 11;

	unsigned char playGround[11][15];

	bool done = false;
	bool finding = false;
	bool pathGenerate = false;

	Barrier barricade;
	Player player;

	SDL_Texture* texture = LoadTexture(image_path, &tex_width, &tex_height, renderer);


	SDL_Surface* temp;
	temp = SDL_CreateRGBSurface(0, widthInPixels, heightInPixels, 32, 0, 0, 0, 0);

	SDL_FillRect(temp, NULL, SDL_MapRGB(temp->format, 255, 0, 0));

	barricade.Init(SDL_CreateTextureFromSurface(renderer, temp), widthInPixels, heightInPixels);

	player.Init(texture, widthInPixels, heightInPixels, 5, 9);
	SDL_FreeSurface(temp);

	Stack stack_1, stack_2;
	Stack* ñheckNow = &stack_1;
	Stack* chechNext = &stack_2;
	Stack path;

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
					ñheckNow->Clear();
					chechNext->Clear();
					path.Clear();
					ñheckNow->AddElement(player.pos.x, player.pos.y);
					for (int i = 0; i < 11; i++)
					{
						for (int j = 0; j < 15; j++)
						{
							playGround[i][j] = 1;
						}
					}
					playGround[player.pos.y][player.pos.x] = 2;
					playGround[currY / heightInPixels][currX / widthInPixels] = 255;
					finding = true;
					break;
				}
			}
		}

		// Clearing the screen
		SDL_RenderClear(renderer);

		// All drawing goes here

		playGround[2][7] = 0;
		barricade.Rendering(renderer, 7, 2);

		//for (int i = 0; i < 5; i++)
		//{
		//	int x = rand() % 15;
		//	int y = rand() % 11;
		//	
		//	playGround[y][x] = 0;
		//	barricade.Rendering(renderer, x, y);
		//}

		// Let's draw a sample image
		deltaTime = DeltaTime(&lastTick, &firstTick);

		while (finding)
		{
			unsigned char x = ñheckNow->LastElement->X;
			unsigned char y = ñheckNow->LastElement->Y;
			if (x - 1 >= 0 && (playGround[y][x - 1] == 1 || playGround[y][x - 1] == 255))		// LEFT POINT
			{
				if (playGround[y][x - 1] != 255)
				{
					chechNext->AddElement(x - 1, y);
				}
				else
				{
					path.AddElement(x - 1, y);
					pathGenerate = true;
				}
				playGround[y][x - 1] = playGround[y][x] + 1;
			}


			if (y - 1 >= 0 && (playGround[y - 1][x] == 1 || playGround[y - 1][x] == 255))		//UP POINT
			{
				if (playGround[y - 1][x] != 255)
				{
					chechNext->AddElement(x, y - 1);
				}
				else
				{
					path.AddElement(x, y - 1);
					pathGenerate = true;
				}
				playGround[y - 1][x] = playGround[y][x] + 1;
			}

			if (x + 1 < 15 && (playGround[y][x + 1] == 1 || playGround[y][x + 1] == 255))			//RIGTH POINT
			{

				if (playGround[y][x + 1] != 255)
				{
					chechNext->AddElement(x + 1, y);
				}
				else
				{
					path.AddElement(x + 1, y);
					pathGenerate = true;
				}
				playGround[y][x + 1] = playGround[y][x] + 1;
			}

			if (y + 1 < 11 && (playGround[y + 1][x] == 1 || playGround[y + 1][x] == 255))		//DOWN POINT
			{
				if (playGround[y + 1][x] != 255)
				{
					chechNext->AddElement(x, y + 1);
				}
				else
				{
					path.AddElement(x, y + 1);
					pathGenerate = true;
				}
				playGround[y + 1][x] = playGround[y][x] + 1;
			}
			ñheckNow->DeleteLastElement();
			if (!ñheckNow->LastElement)
			{
				if (chechNext->LastElement)
				{
					Stack* temp = ñheckNow;
					ñheckNow = chechNext;
					chechNext = temp;
				}
				else
				{
					finding = false;
					break;
				}
			}


			while (pathGenerate)
			{

				unsigned char x = path.LastElement->X;
				unsigned char y = path.LastElement->Y;
				if (playGround[y][x] <= 3)
				{
					finding = false;
					pathGenerate = false;
					break;
				}
				if (x - 1 >= 0 && playGround[y][x - 1] == playGround[y][x] - 1)
				{
					path.AddElement(x - 1, y);
				}
				else if (x + 1 < 15 && playGround[y][x + 1] == playGround[y][x] - 1)
				{
					path.AddElement(x + 1, y);
				}
				else if (y - 1 >= 0 && playGround[y - 1][x] == playGround[y][x] - 1)
				{
					path.AddElement(x, y - 1);
				}
				else if (y + 1 < 11 && playGround[y + 1][x] == playGround[y][x] - 1)
				{
					path.AddElement(x, y + 1);
				}

			}
		}

		if (path.LastElement)
		{
			player.pos.x = path.LastElement->X;
			player.pos.y = path.LastElement->Y;
			path.DeleteLastElement();
			Sleep(100);
		}
		////GoLeft
		//if (currX < x)
		//{
		//	x -= speed * deltaTime;
		//}
		////GoRight
		//if (currX > x)
		//{
		//	x += speed * deltaTime;
		//}
		////GoUp
		//if (currY > y)
		//{
		//	y += speed * deltaTime;
		//}
		////GoDown
		//if (currY < y)
		//{
		//	y -= speed * deltaTime;
		//}


		// Here is the rectangle where the image will be on the screen
		//SDL_Rect rect;
		//rect.x = (int)round(x - tex_width / 2); // Counting from the image's center but that's up to you
		//rect.y = (int)round(y - tex_height / 2); // Counting from the image's center but that's up to you
		//rect.w = (int)tex_width;
		//rect.h = (int)tex_height;

		//SDL_RenderCopyEx(renderer, // Already know what is that
		//	texture, // The image
		//	nullptr, // A rectangle to crop from the original image. Since we need the whole image that can be left empty (nullptr)
		//	&rect, // The destination rectangle on the screen.
		//	0, // An angle in degrees for rotation
		//	nullptr, // The center of the rotation (when nullptr, the rect center is taken)
		//	SDL_FLIP_NONE); // We don't want to flip the image

		player.Rendering(renderer);
// Showing the screen to the player
		SDL_RenderPresent(renderer);

		// next frame...
	}
	//Some problems with free array ^_^
	//free(playGround);
	//barricade.Destroy();
	//player.Destroy();

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