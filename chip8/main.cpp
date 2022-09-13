#include "SDL.h"
#undef main 

#include <chrono>
#include <iostream>


#include "chip8.h"


void processInput(unsigned short* input, bool* quit)
{
	SDL_Event event; 
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_QUIT:
		{
			*quit = true;
		}break;

		case SDL_KEYDOWN:
		{
			//something about keydown 
			if (event.key.keysym.sym == SDLK_x) input[0] = 1;
			else if (event.key.keysym.sym == SDLK_1) input[1] = 1;
			else if (event.key.keysym.sym == SDLK_2) input[2] = 1;
			else if (event.key.keysym.sym == SDLK_3) input[3] = 1;

			else if (event.key.keysym.sym == SDLK_q) input[4] = 1;
			else if (event.key.keysym.sym == SDLK_w) input[5] = 1;
			else if (event.key.keysym.sym == SDLK_e) input[6] = 1;
			else if (event.key.keysym.sym == SDLK_a) input[7] = 1;

			else if (event.key.keysym.sym == SDLK_s) input[8] = 1;
			else if (event.key.keysym.sym == SDLK_d) input[9] = 1;
			else if (event.key.keysym.sym == SDLK_z) input[10] = 1;
			else if (event.key.keysym.sym == SDLK_c) input[11] = 1;

			else if (event.key.keysym.sym == SDLK_4) input[12] = 1;
			else if (event.key.keysym.sym == SDLK_r) input[13] = 1;
			else if (event.key.keysym.sym == SDLK_f) input[14] = 1;
			else if (event.key.keysym.sym == SDLK_v) input[15] = 1;
		}break;

		case SDL_KEYUP:
		{
			//something about keyup 
			if (event.key.keysym.sym == SDLK_x) input[0] = 0;
			else if (event.key.keysym.sym == SDLK_1) input[1] = 0;
			else if (event.key.keysym.sym == SDLK_2) input[2] = 0;
			else if (event.key.keysym.sym == SDLK_3) input[3] = 0;

			else if (event.key.keysym.sym == SDLK_q) input[4] = 0;
			else if (event.key.keysym.sym == SDLK_w) input[5] = 0;
			else if (event.key.keysym.sym == SDLK_e) input[6] = 0;
			else if (event.key.keysym.sym == SDLK_a) input[7] = 0;

			else if (event.key.keysym.sym == SDLK_s) input[8] = 0;
			else if (event.key.keysym.sym == SDLK_d) input[9] = 0;
			else if (event.key.keysym.sym == SDLK_z) input[10] = 0;
			else if (event.key.keysym.sym == SDLK_c) input[11] = 0;

			else if (event.key.keysym.sym == SDLK_4) input[12] = 0;
			else if (event.key.keysym.sym == SDLK_r) input[13] = 0;
			else if (event.key.keysym.sym == SDLK_f) input[14] = 0;
			else if (event.key.keysym.sym == SDLK_v) input[15] = 0;
		}break;

		}
	}
	
}

void draw(unsigned char* graphicsptr, SDL_Window* window, SDL_Surface* windowScreen)
{
	for (int i = 0; i < 32; ++i)
	{
		for (int j = 0; j < 64; ++j)
		{
			const SDL_Rect rect{ .x = j * 10, .y = i * 10, .w = 10, .h = 10 }; 
			const Uint8 color = (graphicsptr[(i * 64) + j] == 1 ? 0xff : 0x00);
			SDL_FillRect(windowScreen, &rect, SDL_MapRGB(windowScreen->format, color, color, color));
		}
	}
	SDL_UpdateWindowSurface(window);
}


int main()
{

	//create the window 
	SDL_Init(SDL_INIT_EVERYTHING);
	SDL_Window* window; 
	window = SDL_CreateWindow("video",
		SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		640, 320,
		SDL_WINDOW_RESIZABLE);
	
	if (window == NULL) std::cerr << "there is an error with creating window" << std::endl;

	//create screen binding it to window
	SDL_Surface* windowScreen1 = SDL_GetWindowSurface(window);
	


	chip8* chip = new chip8();
	chip->initialize(); 

	const char* romfilename = "INVADERS";
	chip->loadROM(romfilename);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();
	std::chrono::high_resolution_clock::time_point end;
	std::chrono::duration<double> time_span;

	bool quit = false; 
	while (!quit)
	{ 
		processInput(chip->input, &quit);
	
		//process the time here 
		end = std::chrono::high_resolution_clock::now();
		time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
 
		if (time_span.count() > 0.0015667)
		{
			start = end;
			chip->emulate(); 
			draw(chip->graphicsptr, window, windowScreen1); 
		}
	}

	delete chip;
	SDL_DestroyWindow(window);
	SDL_Quit(); 
	return 0;
}
