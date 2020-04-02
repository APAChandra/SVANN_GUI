//created by anshc on 26/03/2020
//header file to manage the pipeline module of the ISA

#include <string>
#include "memory.h"
#include <queue>

class pipeline {
public:

	memory mem;
	queue <int> ins_track;
	long long int reg[5];
	long long int Imm;
	long long int IR;
	long long int ALUo;
	int opcode;
	int type;
	int A, B, C;
	int Awb, Bwb, Cwb;
	int cbit;
	int cbit2;
	int cbitwb;
	long long int ALUowb;
	long long int npc;
	long long int lmd;
	bool regHaz[64];
	bool FUstate[5];
	bool branch;
	bool RAW;
	bool cond;
	int condind;
	int RAWind;
	int branchind;
	int ldr;
	int cycle;
	int wb;
	int clock;
	int next;
	bool cache;
	int npcex;
	int npcmem;
	int npcwb;

	pipeline(memory& in) {
		mem = in;
		Imm = 0;
		IR = 0LL;
		ALUo = 0;
		A = 0;
		B = 0;
		C = 0;
		cache = true;
		for (int i = 0; i < 64; i++) {
			regHaz[i] = false;
		}
		for (int i = 0; i < 5; i++) {
			FUstate[i] = true;
				reg[i] = 0;
		}
	}

	/*Function to extract k bits from p position
	and returns the extracted value as integer
	Input: number = 171
	k = 5
	p = 2
	Output : The extracted number is 21
	171 is represented as 0101011 in binary,
	so, you should get only 10101 i.e. 21.*/

	int bitExtracted(long long int number, int p, int k)
	{
		p = p + 1;

		return (((1 << k) - 1) & (number >> (p - 1)));
	}

	void IF() {
		IR = mem.DRAM[mem.registers[1]];
		npc = mem.registers[1];
		ins_track.push(npc);
		mem.registers[1]++;
	}

	void ID() {
		type = bitExtracted(IR, 0, 3);
		cbit = bitExtracted(IR, 3, 1);
		opcode = bitExtracted(IR, 4, 5);
		for (int i = 0; i < 5; i++) {
			reg[i] = bitExtracted(IR, 9 + (i * 6), 6);
		}
		if (regHaz[reg[0]] || regHaz[reg[1]]) {
			RAW = true;
			RAWind = mem.registers[1];
		}
		Imm = bitExtracted(IR, 40, 23);
		int sign = bitExtracted(IR, 39, 1);
		if (sign == 1)
			Imm = 0 - Imm;
		if (type == 0) {
			branch = true;
			branchind = npc;
		}
		if (type == 3 && opcode == 10) {
			cond = true;
			condind = npc;
		}

		IF();
	}

	void EX() {
		A = reg[0]; B = reg[2]; C = reg[3]; cbit2 = cbit;
		if (cbit == 0 || (cbit == 1 && mem.registers[3])) {
			if (type == 3) {
				switch (opcode)
				{
				case 0:
					ALUo = mem.registers[reg[0]] + mem.registers[reg[1]] + Imm;

					break;
				case 1:
					ALUo = mem.registers[reg[0]] - mem.registers[reg[1]] + Imm;
					regHaz[reg[2]] == true;
					break;
				case 2:
					ALUo = mem.registers[reg[0]] * mem.registers[reg[1]];
					regHaz[reg[2]] == true;
					break;
				case 4:
					ALUo = mem.registers[reg[0]] / mem.registers[reg[1]];
					regHaz[reg[2]] == true;
					break;
				case 10:
					int cmp = bitExtracted(reg[2], 3, 3);
					regHaz[3] = true;
					switch (cmp)
					{
					case 0:
						ALUo = mem.registers[reg[0]] == mem.registers[reg[1]];
						break;
					case 1:
						ALUo = mem.registers[reg[0]] != mem.registers[reg[1]];
						break;
					case 2:
						ALUo = mem.registers[reg[0]] <= mem.registers[reg[1]];
						break;
					case 3:
						ALUo = mem.registers[reg[0]] >= mem.registers[reg[1]];
						break;
					case 4:
						ALUo = mem.registers[reg[0]] < mem.registers[reg[1]];
						break;
					case 5:
						ALUo = mem.registers[reg[0]] > mem.registers[reg[1]];
						break;

					default:
						break;
					}
				}
			}
			else if (type == 1) {
				switch (opcode)
				{
				case 7:
					ALUo = mem.registers[reg[1]] + Imm;
					regHaz[reg[0]] == true;
					break;
				case 8:
					ALUo = mem.registers[reg[1]] + Imm;
					regHaz[reg[0]] == true;
					break;
				default:
					break;
				}
			}
			else if (type == 0) {
				mem.registers[1] = npc + Imm;
				IF();
			}
		}
		if (!branch) {
			ID();
		}
	}

	void MEM() {
		Awb = A; Bwb = B; Cwb = C; cbitwb = cbit2; ALUowb = ALUo;
		if (cbit2 == 0 || (cbit2 == 1 && mem.registers[3])) {
			if (type == 1) {
				switch (opcode)
				{
				case 7:
					if (cache == false) {
						mem.registers[mem.registers[A]] = mem.DRAM[ALUo];
					}
					else {
						mem.load(ALUo, A);
					}
					regHaz[A] = false;
					break;
				case 8:
					if (cache == false) {
						mem.DRAM[ALUo] = mem.registers[A];
					}
					else {
						mem.store(ALUo, A);
					}

					regHaz[A] = false;
					break;
				default:
					break;
				}
			}
			if (type == 0) {
				mem.registers[1] = ALUo;
			}
			else {
				mem.registers[1] = npc;
			}
		}
		else {
			mem.registers[1] = npc;
		}
		if (branch && ins_track.front() == branchind) {
			ID();
		}
		else if ((RAW && RAWind == ins_track.front()) || (cond && ins_track.front() == condind)) {
			next = 1;
		}
		else {
			EX();
		}

	}

	void WB() {
		if (cbitwb == 0 || (cbitwb == 1 && mem.registers[3])) {
			if (type == 3) {
				switch (opcode)
				{
				case 0:
				case 1:
				case 2:
				case 4:
					mem.registers[Cwb] = ALUowb;
					regHaz[Cwb] = false;
					break;
				case 10:
					mem.registers[3] = ALUowb;
					regHaz[3] = false;
				default:
					break;
				}
			}
		}
		if (branch && ins_track.front() == branchind) {
			branch = false;
			ins_track.pop();
			EX();
			next = 1;
		}
		else if (RAW && RAWind == ins_track.front()) {
			RAW = true;
			ins_track.pop();
			EX();
			next = 1;
		}
		else if (cond && ins_track.front() == condind) {
			cond = true;
			ins_track.pop();
			EX();
			next = 1;
		}
		else {
			ins_track.pop();
			MEM();

			next = 0;
		}
	}

	void runPipeline(int startAddr, int endAddr) {
		mem.registers[1] = startAddr;
		IF();
		clock++;
		ID();
		clock++;
		EX();
		clock++;
		MEM();
		clock++;
		WB();
		while (mem.registers[1] < endAddr) {
			if (next == 1) {
				MEM();
				clock++;
				WB();
				clock++;
			}
			else if (next == 0) {
				WB();
				clock++;
			}
		}
	}

	void runWithoutPipeLine(int startAddr, int endAddr) {
		for (int i = startAddr; i < endAddr; i++) {
			runPipeline(i, i);
		}
	}

};