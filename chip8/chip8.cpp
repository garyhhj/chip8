#include "chip8.h"
#include <ctime>
#include <iostream>
#include <fstream>

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
	graphicsptr = graphics;

	//load fontset
	const int font_start_mem = 0x50;
	for (int i = 0; i < 80; ++i) mem[font_start_mem + i] = sprites[i];

	//reset timer 
	dt = 0;
	st = 0;

	//drawflag = false; 
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
	srand(time(NULL));
	reg[(opcode & 0x0F00) >> 8] = (std::rand() % 255) & (opcode & 0x00FF);
}
void chip8::op_Dxyn()
{
	const int screenwidth = 64;
	const int screenheight = 32;
	const int vx = reg[(opcode & 0x0F00) >> 8] % screenwidth;
	const int vy = reg[(opcode & 0x00F0) >> 4] % screenheight;
	const int n = opcode & 0x000F;

	constexpr int COLLISION_INDEX = 0xf;
	reg[COLLISION_INDEX] = 0;

	for (int i = 0; i < n; ++i)
	{
		const char row = mem[I + i];
		for (int j = 0; j < 8; ++j)
		{
			const int display_pixel_cor = (vy + i) * screenwidth + vx + j;
			const unsigned char display_pixel = graphicsptr[display_pixel_cor];
			const unsigned char sprit_pixel = (row >> (7 - j)) & 0x0001;

			graphicsptr[display_pixel_cor] ^= sprit_pixel;

			if (display_pixel && sprit_pixel) reg[COLLISION_INDEX] = 1; //0xF is collision index 
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

