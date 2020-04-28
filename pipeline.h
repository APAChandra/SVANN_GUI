//created by anshc on 26/03/2020
//header file to manage the pipeline module of the ISA

#include <string>
#include "memory.h"
#include <deque>
#include <math.h>
#include <vector>

class pipeline {
public:

	memory mem;
	deque <long long int> ins_track;
	deque <long long int> spadTrack;
	vector <long long int> ALUom;
	vector <long long int> ALUomwb;
	long long int reg[5];
	long long int Imm;
	//long long int mem.registers[63];
	long long int ALUo;
	int opcode;
	int type;
	int A, B, C, D, E;
	int Awb, Bwb, Cwb, Dwb, Ewb;
	int cbit;
	int cbit2;
	int cbitwb;
	long long int ALUowb;
	long long int npc;
	long long int lmd;
	bool regHaz[64];
	bool FUstate[5];
	bool branch;
	bool singleStepBranch = false; // bool added to check if jump occured during single step
	bool branchSkipped = false; // bool added to check if jump skipped during single step (possibly redundant)
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
		//mem.registers[63] = 0LL;
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

	bool checkRangeHazard(int startAdress, int size) {
		int endAddr = startAdress + size;
		if (spadTrack.size() > 0) {
			for (int i = 0; i < spadTrack.size(); i+=2) {
				if ((endAddr < spadTrack[i+1] && endAddr > spadTrack[i]) || 
					(startAdress > spadTrack[i] && startAdress < spadTrack[i+1]) ||
					(startAdress < spadTrack[i] && endAddr > spadTrack[i+1])) {
					return true;
				}
			}
				
		}

		return false;
	}

	int secElem(deque<long long int> x) {
		deque<long long int> y(x);
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

	void IF(boolean pipe = true) {
		if (cache) {
			clock += mem.load(mem.registers[1], 63);
		}
		else {
			mem.registers[63] = mem.DRAM[mem.registers[1]];
			clock += 100;
		}

//		mem.registers[63] = mem.DRAM[mem.registers[1]];
			
		npc = mem.registers[1];
		ins_track.push_back(npc);
		mem.registers[1]++;
	}

	void ID(boolean pipe = true) {
		type = bitExtracted(mem.registers[63], 0, 3);
		if (type == 7) {
			cout << "gang";
		}
		cbit = bitExtracted(mem.registers[63], 3, 1);
		opcode = bitExtracted(mem.registers[63], 4, 5);
		for (int i = 0; i < 5; i++) {
			reg[i] = bitExtracted(mem.registers[63], 9 + (i * 6), 6);
		}

		// debugging, trying to detect when a jump is NOT taken
		int z = 0;
		if (cbit == 1 && type == 0 && mem.registers[3] == 0) {
			branchSkipped = true;
		}

		if (type == 1 && opcode == 10) {
			if (regHaz[reg[0]] || regHaz[reg[1]] || regHaz[reg[2]] || regHaz[reg[3]]) {
				RAW = true;
				RAWind = npc;
			}
			if (checkRangeHazard(mem.registers[reg[3]], mem.registers[reg[1]] * mem.registers[reg[2]])) {
				RAW = true;
				RAWind = npc;
			}
		}

		if (type == 4 && (opcode == 0)) {
			if (regHaz[reg[0]] || regHaz[reg[1]] || regHaz[reg[2]] || regHaz[reg[3]] || regHaz[reg[4]]) {
				RAW = true;
				RAWind = npc;
			}

			if (checkRangeHazard(mem.registers[reg[2]], mem.registers[reg[1]] * mem.registers[reg[4]])) {
				RAW = true;
				RAWind = npc;
			}
			if (checkRangeHazard(mem.registers[reg[3]], mem.registers[reg[4]])) {
				RAW = true;
				RAWind = npc;
			}
		}

		if (type == 4 && (opcode == 6 || opcode == 7 || opcode == 8 || opcode == 9 )) {
			if (regHaz[reg[0]] || regHaz[reg[1]] || regHaz[reg[2]] || regHaz[reg[3]]) {
				RAW = true;
				RAWind = npc;
			}

			if (checkRangeHazard(mem.registers[reg[1]], mem.registers[reg[2]])) {
				RAW = true;
				RAWind = npc;
			}
			if (checkRangeHazard(mem.registers[reg[3]], mem.registers[reg[2]])) {
				RAW = true;
				RAWind = npc;
			}
		}

		if (type == 4 && (opcode == 10 || opcode == 11 || opcode == 12 || opcode == 13)) {
			if (regHaz[reg[0]] || regHaz[reg[1]] || regHaz[reg[2]] || regHaz[reg[3]]) {
				RAW = true;
				RAWind = npc;
			}

			if (checkRangeHazard(mem.registers[reg[1]], mem.registers[reg[2]])) {
				RAW = true;
				RAWind = npc;
			}
		}

		if (regHaz[reg[0]] || regHaz[reg[1]]) {
			RAW = true;
			RAWind = npc;
		}
		Imm = bitExtracted(mem.registers[63], 40, 24);
		int sign = bitExtracted(mem.registers[63], 39, 1);
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
		if(pipe == true)
			IF();
	}

	void EX(boolean pipe = true) {
		A = reg[0]; B = reg[1]; C = reg[2]; D = reg[3]; E = reg[4]; cbit2 = cbit; typemem = type; opcodemem = opcode;
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
				case 0:
					ALUo = mem.registers[reg[1]];
					//TODO
					//Figure out RAW Hazards for scratchpad
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[B]);
					break;
				case 1:
					ALUo = mem.registers[reg[1]];
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[B]);
					//TODO
					//Figure out RAW Hazards for scratchpad
					break;
				case 10:
					ALUo = mem.registers[reg[1]] * mem.registers[reg[2]];
					spadTrack.push_back(mem.registers[D]);
					spadTrack.push_back(mem.registers[D] + (mem.registers[B] * mem.registers[C] -1));
					//spadTrack.push_back({ mem.registers[A], mem.registers[A] + (mem.registers[B] * mem.registers[C])});
					//TODO
					//Figure out RAW Hazards for scratchpad
					break;
				case 11:
					ALUo = mem.registers[reg[1]]*mem.registers[reg[2]];
					spadTrack.push_back(mem.registers[D]);
					spadTrack.push_back(mem.registers[D] + (mem.registers[B] * mem.registers[C])-1);
					//spadTrack.push_back({ mem.registers[A], mem.registers[A] + (mem.registers[B] * mem.registers[C]) });
					//TODO
					//Figure out RAW Hazards for scratchpad
					break;
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
				if (pipe) {
					mem.registers[1] = (npc - 1) + Imm;
					singleStepBranch = true;
					ins_track.pop_back();
					clock -= 200;
					IF();
					return;
				}
				else {
					mem.registers[1] = mem.registers[1] - 1 + Imm;
					return;
				}
			}
			else if (type == 4) {
				switch (opcode) 
				{
				case 0:
					ALUom.clear();
					for (int i = 0; i < mem.registers[B]; i++) {
						int value = 0;
						for (int j = 0; j < mem.registers[E]; j++) {
							value += mem.SPAD[(mem.registers[C] + j * mem.registers[B])+i] * mem.SPAD[mem.registers[D]+j];
						}
						ALUom.push_back(value);
					}
				
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[B] -1);
					break;
				case 6:
					ALUom.clear();
					for (int i = 0; i < mem.registers[C];i++) {
						ALUom.push_back(mem.SPAD[mem.registers[B]+i]+ mem.SPAD[mem.registers[D] + i]);
					}
					spadTrack.push_back(mem.registers[A]); 
					spadTrack.push_back(mem.registers[A] + mem.registers[C]);
					break;
				case 7:
					ALUom.clear();
					for (int i = 0; i < mem.registers[C]; i++) {
						ALUom.push_back(mem.SPAD[mem.registers[B] + i] - mem.SPAD[mem.registers[D] + i]);
					}
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[C]);
					break;
				case 8:
					ALUom.clear();
					for (int i = 0; i < mem.registers[C]; i++) {
						ALUom.push_back(mem.SPAD[mem.registers[B] + i] * mem.SPAD[mem.registers[D] + i]);
					}
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[C]);
					break;
				case 9:
					ALUom.clear();
					for (int i = 0; i < mem.registers[C]; i++) {
						ALUom.push_back(mem.SPAD[mem.registers[B] + i] / mem.SPAD[mem.registers[D] + i]);
					}
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[C]);
					break;
				case 10:
					ALUom.clear();
					for (int i = 0; i < mem.registers[C]; i++) {
						ALUom.push_back(pow((double)mem.SPAD[mem.registers[B] + i], (double)mem.registers[D]));
					}
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[C]);
					break;
				case 11:
					ALUom.clear();
					for (int i = 0; i < mem.registers[C]; i++) {
						ALUom.push_back(log((double)mem.SPAD[mem.registers[B] + i])/ log((double)mem.registers[D]));
					}
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[C]);
					break;
				case 12:
					ALUom.clear();
					for (int i = 0; i < mem.registers[C]; i++) {
						ALUom.push_back(log((double)mem.SPAD[mem.registers[B] + i]) + ((double)mem.registers[D])+Imm);
					}
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[C]);
					break;
				case 13:
					ALUom.clear();
					for (int i = 0; i < mem.registers[C]; i++) {
						ALUom.push_back(exp((double)mem.SPAD[mem.registers[B] + i])+Imm);
					}
					spadTrack.push_back(mem.registers[A]);
					spadTrack.push_back(mem.registers[A] + mem.registers[C]);
					break;


				default:
					break;
				}

			}
		}

		// ignore branch hazards when jumps are not taken
		if (type == 0) {
			branch = false;
		}
		if(pipe)
		ID();
	}

	void MEM(boolean pipe = true) {
		Awb = A; Bwb = B; Cwb = C; Dwb = D; Ewb = E; cbitwb = cbit2; ALUowb = ALUo; typewb = typemem; opcodewb = opcodemem; ALUomwb = ALUom;
		if (typemem == 7) {
			next = 4;
			return;
		}
		if (cbit2 == 0 || (cbit2 == 1 && mem.registers[3])) {
			if (typemem == 1) {
				switch (opcodemem)
				{
				case 0:
					mem.spadLoad(mem.registers[C], mem.registers[A], ALUo);
					clock += 100;
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 1:
					mem.spadStore(mem.registers[A], mem.registers[C], ALUo);
					clock += 100;
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 10:
					mem.spadLoad(mem.registers[A], mem.registers[D], ALUo);
					clock += 100;
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 11:
					mem.spadStore(mem.registers[A], mem.registers[D], ALUo);
					clock += 100;
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 7:
					if (cache == false) {
						mem.registers[A] = mem.DRAM[ALUo];
						clock += 100;
					}
					else {
						clock+=mem.load(ALUo, A);

					}
					regHaz[A] = false;
					break;
				case 8:
					if (cache == false) {
						mem.DRAM[ALUo] = mem.registers[A];
					}
					else {
						clock+=mem.store(ALUo, A);
					}

					regHaz[A] = false;
					break;
				default:
					break;
				}
			}
			else if (typemem == 4) {


			}
		}
		else {
			if(pipe)
				mem.registers[1] = npc;
		}

		int z = secElem(ins_track);
		if (branch && ins_track.front() == branchind && pipe) {
			ID();
		}
		else if (((RAW && RAWind == z) || (cond && ins_track.front() == condind)) && pipe) {
			next = 1;
		}
		else if(pipe) {
			EX();
		}

	}

	void WB(boolean pipe = true) {
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
			else if (typewb == 4) {
				switch (opcode)
				{
				case 0:
					for (int i = 0; i < mem.registers[Bwb]; i++) {
						mem.SPAD[mem.registers[Awb] + i] = ALUomwb[i];
					}
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 6:
					for (int i = 0; i < mem.registers[Cwb]; i++) {
						mem.SPAD[mem.registers[Awb] + i] = ALUomwb[i];
					}
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 7:
					for (int i = 0; i < mem.registers[Cwb]; i++) {
						mem.SPAD[mem.registers[Awb] + i] = ALUomwb[i];
					}
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 8:
					for (int i = 0; i < mem.registers[Cwb]; i++) {
						mem.SPAD[mem.registers[Awb] + i] = ALUomwb[i];
					}
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 9:
					for (int i = 0; i < mem.registers[Cwb]; i++) {
						mem.SPAD[mem.registers[Awb] + i] = ALUomwb[i];
					}
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 10:
					for (int i = 0; i < mem.registers[Cwb]; i++) {
						mem.SPAD[mem.registers[Awb] + i] = ALUomwb[i];
					}
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 11:
					for (int i = 0; i < mem.registers[Cwb]; i++) {
						mem.SPAD[mem.registers[Awb] + i] = ALUomwb[i];
					}
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 12:
					for (int i = 0; i < mem.registers[Cwb]; i++) {
						mem.SPAD[mem.registers[Awb] + i] = ALUomwb[i];
					}
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;
				case 13:
					for (int i = 0; i < mem.registers[Cwb]; i++) {
						mem.SPAD[mem.registers[Awb] + i] = ALUomwb[i];
					}
					spadTrack.pop_front();
					spadTrack.pop_front();
					break;


				default:
					break;
				}
			}
		}
		if (branch && ins_track.front() == branchind && pipe) {
			branch = false;
			ins_track.pop_front();
			EX();
			next = 1;
		}
		else if (RAW && RAWind == secElem(ins_track) && pipe) {
			RAW = false;
			ins_track.pop_front();
			EX();
			next = 1;
		}
		else if (cond && ins_track.front() == condind && pipe) {
			cond = false;
			ins_track.pop_front();
			EX();
			next = 1;
		}
		else if(pipe){
			ins_track.pop_front();
			MEM();

			next = 0;
		}
	}

	memory runPipeline(int startAddr, int endAddr) {
		int y = 0;
		if (startAddr == 10) {
			y++;
		}

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
		clock++;
		// if a single step is being taken AND a jump has just occured,
		// we want to return immediately
		if (startAddr = endAddr + 1 && singleStepBranch) {
			// perform exit instruction replacement now
			mem.DRAM[endAddr] = tempInstr;

			// set instructions start and end for GUI relative to current PC
			mem.instructionsStart = mem.registers[1] - 3;

			// this might break a few things...
			mem.registers[1] = mem.registers[1] - 3;

			return mem;
		}

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

		// set instructions start for GUI relative to current PC
		// if jump instruction has been skipped and single stepping, we're not off by one (why?)
		if (startAddr = endAddr + 1 && branchSkipped) {
			mem.instructionsStart = mem.registers[1];
		}
		else {
			mem.instructionsStart = mem.registers[1] - 1;
		}

		// debugging
		int x = 0;
		if (cbit == 1 && mem.registers[3] == 0) {
			x++;
		}

		return mem;
	}

	memory runWithoutPipeLine(int startAddr, int endAddr) {
		while (next!=4) {
			IF(false);
			clock++;
			ID(false);
			clock++;
			EX(false);
			clock++;
			MEM(false);
			clock++;
			WB(false);
			clock++;
		}

		return mem;
	}

};