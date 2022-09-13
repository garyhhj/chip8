#pragma once


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
	unsigned char* graphicsptr;




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

	typedef void(chip8::* opcodefunction)();
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
