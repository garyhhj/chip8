#include "SDL.h"
#undef main 

#include <fstream>
#include <iostream>
#include <chrono>



unsigned char sprites[80] = {
0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
0x20, 0x60, 0x20, 0x20, 0x70, // 1
0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
0x90, 0x90, 0xF0, 0x10, 0x10, // 4
0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
0xF0, 0x10, 0x20, 0x40, 0x40, // 7
0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
0xF0, 0x90, 0xF0, 0x90, 0x90, // A
0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
0xF0, 0x80, 0x80, 0x80, 0xF0, // C
0xE0, 0x90, 0x90, 0x90, 0xE0, // D
0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

class chip8 {
public:
	void initialize();
	void emulate();
	void loadROM(const char* filename);


	unsigned char mem[0xFFF]; //memory
	unsigned short I; //register I 
	unsigned short pc; //pc = program counter 

	unsigned short opcode; //opcode
	unsigned char reg[0x10]; //register

	unsigned char dt; //delay timer
	unsigned char st; //sound timer 

	unsigned short stack[0x10]; //stack
	unsigned short sp; //stack pointer

	unsigned short input[0x10]; //input   
	unsigned char graphics[64 * 32]; //display

	bool drawflag; //to boost performance

	int pixelx[64 * 32 + 1];//for double buffering
	int pixely[64 * 32 + 1];
	unsigned short pixelptr; 

	//opcodes 
	void op_00E0();
	void op_00EE(); 

	void op_1nnn(); 
	void op_2nnn(); 
	void op_3xkk(); 
	void op_4xkk(); 
	void op_5xy0(); 
	void op_6xkk();
	void op_7xkk(); 

	void op_8xy0(); 
	void op_8xy1(); 
	void op_8xy2();
	void op_8xy3();
	void op_8xy4();
	void op_8xy5();
	void op_8xy6();
	void op_8xy7();
	void op_8xyE();

	void op_9xy0(); 
	void op_Annn(); 
	void op_Bnnn(); 
	void op_Cxkk(); 
	void op_Dxyn(); 

	void op_Ex9E(); 
	void op_ExA1(); 

	void op_Fx07(); 
	void op_Fx0A(); 
	void op_Fx15(); 
	void op_Fx18(); 
	void op_Fx1E(); 
	void op_Fx29(); 
	void op_Fx33(); 
	void op_Fx55(); 
	void op_Fx65(); 

	void op_null(); 


	void op_table_0()
	{
		if ((opcode & 0x000F) == 0x0) ((*this).*(table0[0]))();  
		else ((*this).*(table0[1]))(); 
	}

	void op_table_8()
	{
		((*this).*(table8[opcode & 0x000F]))(); 
	}

	void op_table_e()
	{
		((*this).*(tablee[opcode & 0x000F]))(); 
	}

	void op_table_f()
	{
		((*this).*(tablef[opcode & 0x00FF]))(); 
	}

	typedef void(chip8::*opcodefunction)();
	opcodefunction table[0xF + 1];
	opcodefunction table0[0xE + 1];
	opcodefunction table8[0xE + 1];
	opcodefunction tablee[0xE + 1]; 
	opcodefunction tablef[0x65 + 1];

	chip8()
	{
		//setting up the table 

		//chip8
		table[0x0] = &chip8::op_table_0; 
		table[0x1] = &chip8::op_1nnn; 
		table[0x2] = &chip8::op_2nnn; 
		table[0x3] = &chip8::op_3xkk;
		table[0x4] = &chip8::op_4xkk; 
		table[0x5] = &chip8::op_5xy0; 
		table[0x6] = &chip8::op_6xkk; 
		table[0x7] = &chip8::op_7xkk; 
		table[0x8] = &chip8::op_table_8;
		table[0x9] = &chip8::op_9xy0; 
		table[0xA] = &chip8::op_Annn; 
		table[0xB] = &chip8::op_Bnnn; 
		table[0xC] = &chip8::op_Cxkk; 
		table[0xD] = &chip8::op_Dxyn; 
		table[0xE] = &chip8::op_table_e; 
		table[0xF] = &chip8::op_table_f; 

		for (int i = 0; i <= 0xE; ++i)
		{
			table0[i] = &chip8::op_null; 
			table8[i] = &chip8::op_null; 
			tablee[i] = &chip8::op_null; 
		}
		table0[0] = &chip8::op_00E0; 
		table0[1] = &chip8::op_00EE; 

		table8[0x0] = &chip8::op_8xy0; 
		table8[0x1] = &chip8::op_8xy1;
		table8[0x2] = &chip8::op_8xy2; 
		table8[0x3] = &chip8::op_8xy3;
		table8[0x4] = &chip8::op_8xy4;
		table8[0x5] = &chip8::op_8xy5;
		table8[0x6] = &chip8::op_8xy6;
		table8[0x7] = &chip8::op_8xy7;
		table8[0xE] = &chip8::op_8xyE;

		tablee[0xE] = &chip8::op_Ex9E; 
		tablee[0x1] = &chip8::op_ExA1; 

		for (int i = 0; i <= 0x65; ++i)
		{
			tablef[i] = &chip8::op_null; 
		}
		tablef[0x07] = &chip8::op_Fx07;
		tablef[0x0A] = &chip8::op_Fx0A; 
		tablef[0x15] = &chip8::op_Fx15; 
		tablef[0x18] = &chip8::op_Fx18; 
		tablef[0x1E] = &chip8::op_Fx1E; 
		tablef[0x29] = &chip8::op_Fx29; 
		tablef[0x33] = &chip8::op_Fx33; 
		tablef[0x55] = &chip8::op_Fx55;
		tablef[0x65] = &chip8::op_Fx65;

	}

	
};

void chip8::initialize() {
	//initialize memory and registers once 
	opcode = 0;
	I = 0;
	pc = 0x200;
	sp = 0;

	//clear display, stack, registers, memory
	for (int i = 0; i < 0xFFF; ++i) mem[i] = 0;
	for (int i = 0; i < 0x10; ++i) reg[i] = 0;
	for (int i = 0; i < 0x10; ++i) stack[i] = 0;
	for (int i = 0; i < 64 * 32; ++i) graphics[i] = 0;

	//load fontset
	const int font_start_mem = 0x50;
	for (int i = 0; i < 80; ++i) mem[font_start_mem + i] = sprites[i]; 

	//reset timer 
	dt = 0;
	st = 0;

	drawflag = false; 

	//double buffering
	pixelptr = 0;
	for (int i = 0; i <= 64 * 32; ++i) pixelx[i] = 0; 
	for (int i = 0; i <= 64 * 32; ++i) pixely[i] = 0; 
}

void chip8::loadROM(const char* filename)
{
	const int rom_start_mem = 0x200;

	std::ifstream in(filename, std::ios::binary | std::ios::ate); 

	if (!in.is_open())
	{
		std::cerr << "file cannot be opened" << std::endl; 
		return;
	}

	std::streampos size = in.tellg();
	char* buffer = new char[size] {};

	//return to the beginning of the file 
	in.seekg(0, std::ios::beg);
	in.read(buffer, size);  

	for (int i = 0; i < size; ++i)
	{
		mem[rom_start_mem + i] = buffer[i]; 
	}

	delete[] buffer; 
}

void chip8::emulate() 
{
	//fetches opcode 
	opcode = (mem[pc] << 8) | mem[pc + 1]; 

	//increment program counter 
	pc += 2; 

	//execute opcode
	((*this).*(table[(opcode >> 12)]))(); 

	if (dt > 0) --dt; //delay timer
	if (st > 0) --st; //sound timer 
}

//opcodes 
void chip8::op_00E0()
{
	for (int i = 0; i < 64 * 32; ++i)
	{
		graphics[i] = 0; 
	}
}
void chip8::op_00EE()
{
	pc = stack[--sp]; 
}

void chip8::op_1nnn()
{
	pc = opcode & 0x0FFF; 
}
void chip8::op_2nnn()
{
	stack[sp] = pc; 
	//stack[++sp] = opcode & 0x0FFF; 
	++sp;
	pc = opcode & 0x0FFF; 
}
void chip8::op_3xkk()
{
	if (reg[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF)) pc += 2; 
}
void chip8::op_4xkk()
{
	if (reg[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF)) pc += 2; 
}
void chip8::op_5xy0()
{
	if (reg[(opcode & 0x0F00) >> 8] == reg[(opcode & 0x00F0) >> 4]) pc += 2; 
}
void chip8::op_6xkk()
{
	reg[(opcode & 0x0F00) >> 8] = (opcode & 0x00FF); 
}
void chip8::op_7xkk()
{
	reg[(opcode & 0x0F00) >> 8] += (opcode & 0x00FF); 
}

void chip8::op_8xy0()
{
	reg[(opcode & 0x0F00) >> 8] = reg[(opcode & 0x00F0) >> 4]; 
}
void chip8::op_8xy1()
{
	reg[(opcode & 0x0F00) >> 8] |= reg[(opcode & 0x00F0) >> 4]; 
}
void chip8::op_8xy2()
{
	reg[(opcode & 0x0F00) >> 8] &= reg[(opcode & 0x00F0) >> 4]; 
}
void chip8::op_8xy3()
{
	reg[(opcode & 0x0F00) >> 8] ^= reg[(opcode & 0x00F0) >> 4]; 
}
void chip8::op_8xy4()
{
	int sum = reg[(opcode & 0x0F00) >> 8] + reg[(opcode & 0x00F0) >> 4]; 

	if (sum > 0xFF) reg[0xF] = 1;
	else reg[0xF] = 0; 
	
	reg[(opcode & 0x0F00) >> 8] = (sum & 0x00FF); 
}
void chip8::op_8xy5()
{
	int x = (opcode & 0x0F00) >> 8;
	int y = (opcode & 0x00F0) >> 4;

	if (reg[x] > reg[y]) reg[0xF] = 1;
	else reg[0xF] = 0; 

	reg[x] -= reg[y]; 
}
void chip8::op_8xy6()
{
	reg[0xF] = (reg[(opcode & 0x0F00) >> 8] & 0x1);
	reg[(opcode & 0x0F00) >> 8] >>= 1; 
}
void chip8::op_8xy7()
{
	int x = (opcode & 0x0F00) >> 8;
	int y = (opcode & 0x00F0) >> 4;

	if (reg[y] > reg[x]) reg[0xF] = 1;
	else reg[0xF] = 0; 

	reg[x] = reg[y] - reg[x]; 
}
void chip8::op_8xyE()
{
	reg[(opcode & 0x0F00) >> 8] = (reg[(opcode & 0x0F00) >> 8] & 0x80) >> 7; 
	reg[(opcode & 0x0F00) >> 8] <<= 1; 
}

void chip8::op_9xy0()
{
	if (reg[(opcode & 0x0F00) >> 8] != reg[(opcode & 0x00F0) >> 4]) pc += 2; 
}
void chip8::op_Annn()
{
	I = opcode & 0x0FFF; 
}
void chip8::op_Bnnn()
{
	pc = reg[0] + (opcode & 0x0FFF); 
}
void chip8::op_Cxkk()
{
	reg[(opcode & 0x0F00) >> 8] = (std::rand() % 255) & (opcode & 0x00FF); 
}
void chip8::op_Dxyn()
{
	drawflag = true; 
	const int screenwidth = 64;
	const int screenheight = 32;

	int vx = reg[(opcode & 0x0F00) >> 8]; 
	int vy = reg[(opcode & 0x00F0) >> 4];

	vx %= screenwidth; 
	vy %= screenheight; 

	int n = opcode & 0x000F; 
	 
	reg[0xF] = 0; 
	for (int i = 0; i < n; ++i)
	{
		char row = mem[I + i]; 
		for (int j = 0; j < 8; ++j)
		{
			unsigned char display_pixel = graphics[(vy + i) * screenwidth + vx + j];
			unsigned char sprit_pixel = (row >> (7 - j)) & 0x0001; 
		
			if (sprit_pixel)
			{
				graphics[(vy + i) * screenwidth + vx + j] ^= sprit_pixel;

				//for double buffering
				pixelx[pixelptr] = vx + j; 
				pixely[pixelptr] = vy + i; 
				++pixelptr;

				
			}

			if (display_pixel && sprit_pixel)
			{
				reg[0xF] = 1; 
			}
		}
	}
}

void chip8::op_Ex9E()
{
	int vx = reg[(opcode & 0x0F00) >> 8];
	if (input[vx]) pc += 2;
}
void chip8::op_ExA1()
{
	int vx = reg[(opcode & 0x0F00) >> 8];
	if (!input[vx]) pc += 2; 
}

void chip8::op_Fx07()
{
	reg[(opcode & 0x0F00) >> 8] = dt;
}
void chip8::op_Fx0A()
{
	for (unsigned char i = 0; i < 16; ++i)
	{
		if (input[i])
		{
			const int x = (opcode & 0x0F00) >> 8;
			reg[x] = i;
			return; 
		}
	}
	pc -= 2;
}
void chip8::op_Fx15()
{
	dt = reg[(opcode & 0x0F00) >> 8];
}
void chip8::op_Fx18()
{
	st = reg[(opcode & 0x0F00) >> 8];
}
void chip8::op_Fx1E()
{
	I += reg[(opcode & 0x0F00) >> 8];
}
void chip8::op_Fx29()
{
	unsigned short vx = reg[(opcode & 0x0F00) >> 8];
	I = 0x50 + (vx * 5); //0x50 is font starting address
}
void chip8::op_Fx33()
{
	int val = reg[(opcode & 0x0F00) >> 8];

	mem[I + 2] = val % 10; 
	val /= 10; 
	mem[I + 1] = val % 10; 
	val /= 10; 
	mem[I] = val % 10; 
}
void chip8::op_Fx55()
{
	int x = (opcode & 0x0F00) >> 8; 
	for (int i = 0; i <= x; ++i)
	{
		mem[I + i] = reg[i]; 
	}
}
void chip8::op_Fx65()
{
	int x = (opcode & 0x0F00) >> 8; 

	for (int i = 0; i <= x; ++i)
	{
		reg[i] = mem[I + i]; 
	}
}

void chip8::op_null()
{
	//do nothing 
}



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

void draw(unsigned char* graphics, SDL_Window* window, SDL_Surface* windowScreen, bool* drawflag, 
		  unsigned short& pixelptr, int* pixelx, int* pixely)
{
	

	*drawflag = false; 
	while (pixelptr)
	{
		--pixelptr; 
		int x = pixelx[pixelptr]; 
		int y = pixely[pixelptr]; 

		SDL_Rect rect;
		rect.x = x * 10;
		rect.y = y * 10;
		rect.w = 10;
		rect.h = 10;

		if (graphics[(y * 64) + x] == 1)
		{
			SDL_FillRect(windowScreen, &rect, SDL_MapRGB(windowScreen->format, 0xff, 0xff, 0xff));
		}
		else //can optimzie 
		{
			SDL_FillRect(windowScreen, &rect, SDL_MapRGB(windowScreen->format, 0x00, 0x00, 0x00));
		}
		SDL_UpdateWindowSurface(window);
	}
	
	
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
	


	chip8* chipptr = new chip8();
	chip8& chip = *chipptr; 
	chip.initialize(); 

	const char* romfilename = "TETRIS";
	chip.loadROM(romfilename);

	std::chrono::high_resolution_clock::time_point start = std::chrono::high_resolution_clock::now();

	bool quit = false; 
	while (!quit)
	{ 
		processInput(chip.input, &quit);
	
		//process the time here 
		std::chrono::high_resolution_clock::time_point end = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
 
		if (time_span.count() > 0.005667)
		{
			start = end;
			chip.emulate(); 

			if (chip.drawflag) draw(chip.graphics, window, windowScreen1, &chip.drawflag, chip.pixelptr, chip.pixelx, chip.pixely); 
		}
		
	}

	delete chipptr;
	SDL_DestroyWindow(window);
	SDL_Quit(); 
	return 0;
}
