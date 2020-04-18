//created by anshc on 26/03/2020
//header file to manage the pipeline module of the ISA

#include <string>
#include "memory.h"
#include <deque>

class pipeline {
public:

	memory mem;
	deque <int> ins_track;
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
	int typemem;
	int opcodemem;
	int typewb;
	int opcodewb;

	pipeline(memory& in) {
		mem = in;
		Imm = 0;
		IR = 0LL;
		ALUo = 0;
		npc = 0;
		branchind = 0;
		A = 0;
		B = 0;
		C = 0;
		clock = 0;
		cache = true;
		branch = false;
		RAW = false;
		cond = false;
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

	int secElem(deque<int> x) {
		deque<int> y(x);
		if (x.size() >= 2) {
			int a = y.front();
			y.pop_front();
			int b = y.front();
			return b;
		}
		else {
			return -1;
		}
	}

	int bitExtracted(long long int number, int p, int k)
	{
		string num = mem.decToBinary(number);
		return stoi(num.substr(p, k), nullptr, 2);
	}

	int bitExtractedcmp(long long int number, int p, int k)
	{
		string num = mem.decToBinary(number);
		return stoi(num.substr(61, k), nullptr, 2);
	}

	void IF() {
		IR = mem.DRAM[mem.registers[1]];
		npc = mem.registers[1];
		ins_track.push_back(npc);
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
			RAWind = npc;
		}
		Imm = bitExtracted(IR, 40, 24);
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
		if (type == 7) {
			next = 4;
			return;
		}

		IF();
	}

	void EX() {
		A = reg[0]; B = reg[1]; C = reg[2]; cbit2 = cbit; typemem = type; opcodemem = opcode;
		if (type == 7) {
			next = 4;
			return;
		}
		if (cbit == 0 || (cbit == 1 && mem.registers[3] == 1)) {
			if (type == 3) {
				switch (opcode)
				{
				case 0:
					ALUo = mem.registers[reg[0]] + mem.registers[reg[1]] + Imm;
					regHaz[reg[2]] = true;
					break;
				case 1:
					ALUo = mem.registers[reg[0]] - mem.registers[reg[1]] + Imm;
					regHaz[reg[2]] = true;
					break;
				case 2:
					ALUo = mem.registers[reg[0]] * mem.registers[reg[1]];
					regHaz[reg[2]] = true;
					break;
				case 4:
					ALUo = mem.registers[reg[0]] / mem.registers[reg[1]];
					regHaz[reg[2]] = true;
					break;
				case 10:
					int cmp = bitExtractedcmp(reg[2], 3, 3);
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
					regHaz[reg[0]] = true;
					break;
				case 8:
					ALUo = mem.registers[reg[1]] + Imm;
					regHaz[reg[0]] = true;
					break;
				default:
					break;
				}
			}
			else if (type == 0) {
				mem.registers[1] = (npc - 1) + Imm;
				ins_track.pop_back();
				IF();
				return;
			}
		}

		// ignore branch hazards when jumps are not taken
		if (type == 0) {
			branch = false;
		}

		ID();
	}

	void MEM() {
		Awb = A; Bwb = B; Cwb = C; cbitwb = cbit2; ALUowb = ALUo; typewb = typemem; opcodewb = opcodemem;
		if (typemem == 7) {
			next = 4;
			return;
		}
		if (cbit2 == 0 || (cbit2 == 1 && mem.registers[3])) {
			if (typemem == 1) {
				switch (opcodemem)
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
		}
		else {
			mem.registers[1] = npc;
		}

		int z = secElem(ins_track);
		if (branch && ins_track.front() == branchind) {
			ID();
		}
		else if ((RAW && RAWind == z) || (cond && ins_track.front() == condind)) {
			next = 1;
		}
		else {
			EX();
		}

	}

	void WB() {
		if (typewb == 7) {
			next = 4;
			return;
		}
		if (cbitwb == 0 || (cbitwb == 1 && mem.registers[3])) {
			if (typewb == 3) {
				switch (opcodewb)
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
			ins_track.pop_front();
			EX();
			next = 1;
		}
		else if (RAW && RAWind == secElem(ins_track)) {
			RAW = false;
			ins_track.pop_front();
			EX();
			next = 1;
		}
		else if (cond && ins_track.front() == condind) {
			cond = false;
			ins_track.pop_front();
			EX();
			next = 1;
		}
		else {
			ins_track.pop_front();
			MEM();

			next = 0;
		}
	}

	memory runPipeline(int startAddr, int endAddr) {
		mem.registers[1] = startAddr;

		// if the instruction at endAddr is not already the exit instruction,
		// temporarily place an exit instruction at endAddr
		long long int tempInstr;
		if (mem.DRAM[endAddr] != -2305843009213693952) {
			// save instruction that sits there so it can be replaced later
			tempInstr = mem.DRAM[endAddr];
			mem.DRAM[endAddr] = -2305843009213693952;
		}
		else {
			tempInstr = -2305843009213693952;
		}

		IF();
		clock++;
		ID();
		clock++;
		EX();
		clock++;
		MEM();
		clock++;
		WB();
		while (next != 4) {
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

		// perform instruction replacement for exit instruction
		mem.DRAM[endAddr] = tempInstr;

		return mem;
	}

	memory runWithoutPipeLine(int startAddr, int endAddr) {
		for (int i = startAddr; i < endAddr; i++) {
			runPipeline(i, i);
		}

		return mem;
	}

};