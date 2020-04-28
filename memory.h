//
// Created by anshc on 2/27/2020.
// Edited by bcbabrich on 2/28/20.
//

#ifndef INC_535ISA1_MEMORY_H
#define INC_535ISA1_MEMORY_H

#include <iostream>
#include <fstream>
#include <string>
using namespace std;

// cache line object contains room for 4 words to be loaded
// all fields are ints?
struct cacheLine {
	int index, LRU;
	long int tag;
	long long int word[4];
};

// TODO:    MAKE SURE ADDRESSES INTO DRAM ARRAY ARE ALWAYS IN BASE 10
//          TEST DRAM READ/WRITE
class memory {
	public:
        // DRAM is a one-dimensional array of words (long long ints)
        long long int DRAM[256];

        // ScratchPad is a one-dimensional array of words (long long ints)
        long long int SPAD[256];

        // registers are an array of strings
        // words are loaded and stored as strings
        // this is so we can subindex into them for instruction params
        // 64 GPR registers, but 0-3 are special and should not be used
        long long int registers[64];

		// cache object is 2D array of cacheLines
		// only 16 sets(4 bit set index) for now
		// 4-way set associative, so second dimension is 4
		// public?
		cacheLine cache[16][4];

        // Book Keepers to know where instructions are
        int instructionsStart = 0; // assume instructions always start at 0
        int instructionsEnd;

        // simple binary to decimal function
        // IN: a binary number of type int with NO leading zeroes
        // OUT: an int in base 10
        int binaryToDecimal(long long int n)
        {
            int num = n;
            int dec_value = 0;

            // Initializing base value to 1, i.e 2^0 
            int base = 1;

            int temp = num;
            while (temp) {
                int last_digit = temp % 10;
                temp = temp / 10;

                dec_value += last_digit * base;

                base = base * 2;
            }

            return dec_value;
        }

        string GetHexFromBin(string sBinary)
    {
        string rest("0x"),tmp,chr = "0000";
        int len = sBinary.length()/4;
        chr = chr.substr(0,len);
        sBinary = chr+sBinary;
        for (int i=0;i<sBinary.length();i+=4)
        {
            tmp = sBinary.substr(i,4);
            if (!tmp.compare("0000"))
            {
                rest = rest + "0";
            }
            else if (!tmp.compare("0001"))
            {
                rest = rest + "1";
            }
            else if (!tmp.compare("0010"))
            {
                rest = rest + "2";
            }
            else if (!tmp.compare("0011"))
            {
                rest = rest + "3";
            }
            else if (!tmp.compare("0100"))
            {
                rest = rest + "4";
            }
            else if (!tmp.compare("0101"))
            {
                rest = rest + "5";
            }
            else if (!tmp.compare("0110"))
            {
                rest = rest + "6";
            }
            else if (!tmp.compare("0111"))
            {
                rest = rest + "7";
            }
            else if (!tmp.compare("1000"))
            {
                rest = rest + "8";
            }
            else if (!tmp.compare("1001"))
            {
                rest = rest + "9";
            }
            else if (!tmp.compare("1010"))
            {
                rest = rest + "A";
            }
            else if (!tmp.compare("1011"))
            {
                rest = rest + "B";
            }
            else if (!tmp.compare("1100"))
            {
                rest = rest + "C";
            }
            else if (!tmp.compare("1101"))
            {
                rest = rest + "D";
            }
            else if (!tmp.compare("1110"))
            {
                rest = rest + "E";
            }
            else if (!tmp.compare("1111"))
            {
                rest = rest + "F";
            }
            else
            {
                continue;
            }
        }
        return rest;
    }

        //binary to 64bit representation as string for indexing;
        string decToBinary(long long int n)
        {
            string dec = "";
            // Size of an integer is assumed to be 32 bits
            for (int i = 63; i >= 0; i--) {
                int k = n >> i;
                if (k & 1)
                    dec = dec + "1";
                else
                    dec = dec + "0";
            }
            return dec;
        }

		// checks if the particular binaryAddress exists in cache
		int hit(string binaryAddress)
		{
            // index for the first dimension of cache is the set index;
            int set = stol(binaryAddress.substr(58, 4), 0, 2);
            //check tag by iterating over set in the cache
            long int tag = stol(binaryAddress.substr(0, 58), 0, 2);
            int i;
            for (i = 0; i < 4; i++)
            {
                if(cache[set][i].tag == tag)
                    return i;
            }
            return i;
        }
        //simulator for the load instruction.
        int load(long long int binary, int regIndex){
            //bool flip = true;
            int clocktime = 10;
            string address = decToBinary(binary);
            int set = stoi(address.substr(58, 4), 0, 2);
            int offset = stoi(address.substr(62, 2), 0, 2);
            int cLine = hit(address);
            if(cLine < 4)
            {
                registers[regIndex] = cache[set][cLine].word[offset];
                for(int i = 0; i < 4; i++)
                {
                    if(i < cLine && cache[set][i].LRU >= 0){
                        cache[set][i].LRU++;
                    }
                    else if(i == cLine){
                        cache[set][i].LRU = 0;
                    }
                }
                clocktime = 100;
            }
            else
            {
                for(int i = 0; i < 4; i++) {
                    if (cache[set][i].LRU < 0) {
                        cache[set][i].LRU = 0;
                        //extract tag
                        long int tag = stol(address.substr(0, 58), 0, 2);
                        //set tag
                        cache[set][i].tag = tag;
                        //get ram addr with 0 offset
                        string x = address.substr(0, 62) + "00";
                        long long int ramAddr = stoll(x, 0, 2);
                        //iterate over word line
                        for (int j = 0; j < 4; j++) {
                            //long long int word = DRAM[binaryToDecimal(ramAddr)];
                            long long int word = DRAM[ramAddr];
                            if (ramAddr == binary) {
                                registers[regIndex] = word;
                            }
                            //set word
                            cache[set][i].word[j] = word;
                            //increment ramAddr
                            ramAddr++;
                        }
                        break;
                    }
                    else if (cache[set][i].LRU == 3) {
                        cache[set][i].LRU = 0;
                        //extract tag
                        long int tag = stol(address.substr(0, 58), 0, 2);
                        //set tag
                        cache[set][i].tag = tag;
                        //get ram addr with 0 offset
                        string x = address.substr(0, 62) + "00";
                        long long int ramAddr = stoll(x, 0, 2);
                        //iterate over word line
                        for (int j = 0; j < 4; j++) {
                            long long int word = DRAM[binaryToDecimal(ramAddr)];
                            if (ramAddr == binary) {
                                registers[regIndex] = word;
                            }
                            //set word
                            cache[set][i].word[j] = word;
                            //increment ramAddr
                            ramAddr++;
                        }
                    }
                    else
                        cache[set][i].LRU++;
                }
            }
            return clocktime;
        }

        string printCache(int set){
            string c = "";
            for (int x = 0; x < 4; x++){
                c = c + GetHexFromBin(decToBinary(cache[set][x].tag));
                c = c + "\t";
                for(int i = 0; i < 4; i++){
                    c = c + GetHexFromBin(decToBinary(cache[set][x].word[i]))+ "\t";
                }
                c = c + to_string(cache[set][x].LRU);
                c = c + "\n";
            }
            return c;
        }

        int store(long long int binary, int regIndex){
            int clocktime = 10;
            string address = decToBinary(binary);
            int set = stoi(address.substr(58, 4), 0, 2);
            int offset = stoi(address.substr(62, 2), 0, 2);
            int cLine = hit(address);
            long long int word = 0;
            if(cLine < 4)
            {
                cache[set][cLine].word[offset] = registers[regIndex];
                DRAM[binaryToDecimal(stoll(address))] = registers[regIndex];
                for (int i = 0; i < 4; i++)
                {
                    if (i < cLine && cache[set][i].LRU >= 0) {
                        cache[set][i].LRU++;
                    }
                    else if (i == cLine) {
                        cache[set][i].LRU = 0;
                    }
                }
                clocktime = 100;
                
            }
            else
            {
                DRAM[binaryToDecimal(stoll(address))] = registers[regIndex];
                cache[set][cLine].word[offset] = registers[regIndex];
                for(int i = 0; i < 4; i++) {
                    if (cache[set][i].LRU < 0) {
                        cache[set][i].LRU = 0;
                        //extract tag
                        long int tag = stol(address.substr(0, 58), 0, 2);
                        //set tag
                        cache[set][i].tag = tag;
                        //get ram addr with 0 offset
                        string x = tag + set + "00";
                        long long int ramAddr = stoll(x, 0, 2);
                        //iterate over word line
                        for (int j = 0; j < 4; j++) {
                            long long int word = DRAM[binaryToDecimal(ramAddr)];
                            if (ramAddr == binary) {
                                registers[regIndex] = word;
                            }
                            //set word
                            cache[set][i].word[j] = word;
                            //increment ramAddr
                            ramAddr++;
                            break;
                        }
                    }
                    else if (cache[set][i].LRU == 3) {

                        cache[set][i].LRU = 0;
                        //extract tag
                        long int tag = stol(address.substr(0, 58), 0, 2);
                        //set tag
                        cache[set][i].tag = tag;
                        //get ram addr with 0 offset
                        long long int ramAddr = stoll(tag + set + "00", 0, 2);
                        //iterate over word line
                        for (int j = 0; j < 4; j++) {
                            long long int word = DRAM[binaryToDecimal(ramAddr)];
                            if (ramAddr == binary) {
                                registers[regIndex] = word;
                            }
                            //set word
                            cache[set][i].word[j] = word;
                            //increment ramAddr
                            ramAddr++;
                        }
                    }
                    else
                        cache[set][i].LRU++;
                }
            }
            return clocktime;
        }

        // MEMORY CONSTRUCTOR
        memory(){
            // warming up the cache
            // set LRU for all cachelines to -1
            // fill all cache lines with 0's
            // might want to change 16 and 4 to length of arrays in future (magic numbers at the moment)
            // could also have these "magic numbers" as global parameters
            for (int i = 0; i < 16; i++) {
                for (int j = 0; j < 4; j++) {
                    for (int k = 0; k < 4; k++) {
                        cache[i][j].word[k] = 0000000000000000000000000000000000000000000000000000000000000000;
                    }
                    cache[i][j].LRU = -4;
                }
            }

            // fill DRAM with all 0's
            for (int i = 0; i < 256; i++) {
                DRAM[i] = 0000000000000000000000000000000000000000000000000000000000000000;
            }

            // fill SPAD with all 0's
            for (int i = 0; i < 256; i++) {
                SPAD[i] = 0000000000000000000000000000000000000000000000000000000000000000;
            }
        }

        //Scratchpad Load:
        void spadLoad(int beginDram, int beginSpad, int size) {
            for (int i = 0; i < size; i++) {
                SPAD[beginSpad + i] = DRAM[beginDram + i];
            }

        }

        void spadStore(int beginDram, int beginSpad, int size) {
            for (int i = 0; i < size; i++) {
                DRAM[beginDram + i] = SPAD[beginSpad + i];
            }

        }

}
;


#endif //INC_535ISA1_MEMORY_H
