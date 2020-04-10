// SVANN_GUI.cpp : Defines the entry point for the application.
//

// LAST EDITED BY CHASE ON 3/25
// --> DOING: passing data from memory.h to main.tis

#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "SVANN_GUI.h"
#include "memory.h"
#include "aux-cvt.h"
#include <stdlib.h>
#include <string>
#include "Python.h"
#include "stdio.h"
#include "pipeline.h"

#include "sciter-x-window.hpp"
#include "value.hpp"

class frame : public sciter::window {
public:
    // INITIAL SETUP OF MEMORY AND PIPELINE OBJECTS
    memory memTest;
    pipeline globalPipeline = pipeline(memTest); // reinitialize pipeTestObject


    frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS | SW_MAIN | SW_ENABLE_DEBUG) {}

    // map of native functions exposed to script:
    BEGIN_FUNCTION_MAP
        FUNCTION_0("grabDataFrom_Cache", grabDataFrom_Cache);
        FUNCTION_0("grabDataFrom_DRAM", grabDataFrom_DRAM);
        FUNCTION_0("grabDataFrom_Registers", grabDataFrom_Registers);
        FUNCTION_1("uploadInstructionsFile", uploadInstructionsFile);
        FUNCTION_1("saveProgState", saveProgState);
        FUNCTION_1("readInProgState", readInProgState);
        FUNCTION_0("getClockCount", getClockCount);
        FUNCTION_1("runInstsructionsFor", runInstsructionsFor);
    END_FUNCTION_MAP

    sciter::string grabDataFrom_Cache() {
        
        // all words from cache and place them in a string
        // return the string to .tis and create array based on delimeters
        sciter::string cacheStr = WSTR("");
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    cacheStr += to_wstring(memTest.cache[i][j].word[k]);
                }
                cacheStr += WSTR(" ");
            }
        }

        return cacheStr;
    }

    sciter::string grabDataFrom_DRAM() {

        // all words from cache and place them in a string
        // return the string to .tis and create array based on delimeters
        sciter::string dramStr = WSTR("");
        for (int i = 0; i < 256; i++) {
            // convert binary long long int to wstring
            wstring str(64, '0');
            for (int j = 0; j < 64; j++)
            {
                if ((1ll << j) & memTest.DRAM[i])
                    str[63 - j] = '1';
            }

            // convert binary string to hex string
            wstring sBinary = str;
            wstring rest(WSTR("0x")), tmp, chr = WSTR("0000");
            int len = sBinary.length() / 4;
            chr = chr.substr(0, len);
            sBinary = chr + sBinary;
            for (int i = 0; i < sBinary.length(); i += 4)
            {
                tmp = sBinary.substr(i, 4);
                if (!tmp.compare(WSTR("0000")))
                {
                    rest = rest + WSTR("0");
                }
                else if (!tmp.compare(WSTR("0001")))
                {
                    rest = rest + WSTR("1");
                }
                else if (!tmp.compare(WSTR("0010")))
                {
                    rest = rest + WSTR("2");
                }
                else if (!tmp.compare(WSTR("0011")))
                {
                    rest = rest + WSTR("3");
                }
                else if (!tmp.compare(WSTR("0100")))
                {
                    rest = rest + WSTR("4");
                }
                else if (!tmp.compare(WSTR("0101")))
                {
                    rest = rest + WSTR("5");
                }
                else if (!tmp.compare(WSTR("0110")))
                {
                    rest = rest + WSTR("6");
                }
                else if (!tmp.compare(WSTR("0111")))
                {
                    rest = rest + WSTR("7");
                }
                else if (!tmp.compare(WSTR("1000")))
                {
                    rest = rest + WSTR("8");
                }
                else if (!tmp.compare(WSTR("1001")))
                {
                    rest = rest + WSTR("9");
                }
                else if (!tmp.compare(WSTR("1010")))
                {
                    rest = rest + WSTR("A");
                }
                else if (!tmp.compare(WSTR("1011")))
                {
                    rest = rest + WSTR("B");
                }
                else if (!tmp.compare(WSTR("1100")))
                {
                    rest = rest + WSTR("C");
                }
                else if (!tmp.compare(WSTR("1101")))
                {
                    rest = rest + WSTR("D");
                }
                else if (!tmp.compare(WSTR("1110")))
                {
                    rest = rest + WSTR("E");
                }
                else if (!tmp.compare(WSTR("1111")))
                {
                    rest = rest + WSTR("F");
                }
                else
                {
                    continue;
                }
            }
            dramStr += rest + WSTR(" ");
        }

        return dramStr;
    }

    sciter::string grabDataFrom_Registers() {

        // all words from cache and place them in a string
        // return the string to .tis and create array based on delimeters
        sciter::string regStr = WSTR("");
        for (int i = 0; i < 64; i++) {
            regStr += to_wstring(memTest.registers[i]) + WSTR(" ");
        }

        return regStr;
    }

    // UPLOAD INSTRUCTIONS FILE
    // In: sciter::value file name of .txt instructions file to open
    //       - .txt file assumed to sit in source files folder
    // Out: sciter::string concatenation of all instructions, delimited by character '-'
    //       - sciter seems to require strings be passed using their sciter:: type
    // uploadInstructionsFile's purpose is to read an instruction file and do two things:
    //       - write the instructions in binary as ints to the memory.DRAM array
    //       - return the instructions in assembly as one big string so the UI can display them
    // This function is not optimized AT ALL. I had heck of a time dealing with strings.
    sciter::string uploadInstructionsFile(sciter::value fileName)
    {
        sciter::string fileNameWStr = fileName.get(L""); // convert value to string
        

        

        // Open file and read all instructions into one big sciter::string
        
        sciter::string allInstructions = WSTR(""); // this is what gets returned to the TIS script
        sciter::string instructionLine;
        std::wifstream instructionsFile(fileNameWStr);
        if (instructionsFile.is_open()){
            while (getline(instructionsFile, instructionLine)){
                allInstructions += instructionLine + WSTR("X");
            }
            instructionsFile.close();
        }

        // CONVERT INSTRUCTIONS TO BINARY AND WRITE THEM TO DRAM OBJECT
        // first convert the fileName sciter::string to a std::string
        // I KNOW THIS IS NOT THE RIGHT WAY TO DO THIS
        // But I couldn't figure out any other way to convert a sciter::value all the way to a std::string...

        // Write to file and read it back 
        wofstream myWFile;
        myWFile.open("instrsTemp.txt");
        myWFile << fileNameWStr;
        myWFile.close();

        // Open file and read fileName back as std::string
        std::string stdFileName = "";
        std::string stdLine;
        ifstream mySFile("instrsTemp.txt");
        if (mySFile.is_open())
        {
            while (getline(mySFile, stdLine))
            {
                stdFileName = stdLine; // done! ... sort of.
            }
            mySFile.close();
        }

        // Call Python script that will write binary instructions file
        //      - This temporary variable is apparently extremely important!
        //      - without it C++ will throw an error on the sysCall declaration
        std::string tmp = stdFileName;
        std::string sysCall = "python 535Assembler.py" + stdFileName;
        system("python 535Assembler.py instructionsTest.txt"); // won't work currently

        // read binary instructions file and place instructions in DRAM
        std::string binInstrs = "";
        std::string assemInsrLine;
        std::string tmp1 = stdFileName;
        std::string binInstrFileName = stdFileName.substr(0, stdFileName.length() - 4) + "Binary.txt";
        ifstream binInstrFile(binInstrFileName);
        int i = 0;
        memTest.instructionsStart = 0;
        if (binInstrFile.is_open())
        {
            while (getline(binInstrFile, assemInsrLine))
            {
                if (assemInsrLine.compare("") != 0) {
                    if (fileNameWStr.compare(WSTR("demoInstructions.txt")) != 0) {
                        memTest.DRAM[i] = stoll(assemInsrLine, 0, 2);
                    }
                    i++;
                }
            }
            binInstrFile.close();
        }
        memTest.instructionsEnd = i;

        // for demo, manually fill instructions in DRAM (shh...)
        int x = 0;
        if (fileNameWStr.compare(WSTR("demoInstructions.txt")) == 0) {
            memTest.DRAM[230] = 0;
            memTest.DRAM[231] = 2;
            memTest.DRAM[232] = 40;
            memTest.DRAM[233] = 1;
            memTest.registers[7] = 2;
            memTest.registers[8] = 3;
            memTest.DRAM[0] = 0b0010001110001010000000000000000000000000000000000000000011100110; //R5 = 0
            memTest.DRAM[1] = 0b0010001110001100000000000000000000000000000000000000000011100111; //R6 = 2
            memTest.DRAM[2] = 0b0010001110010100000000000000000000000000000000000000000011101000; //R10 = 40
            memTest.DRAM[3] = 0b0010001110010110000000000000000000000000000000000000000011101001; //R11 = 1
            memTest.DRAM[4] = 0b0110010100001100001010001010000000000000000000000000000000000000; //R3(CR) = R6 > R5
            memTest.DRAM[5] = 0b0110000000001110010000010010000000000000000000000000000000000000; //R9 = R7 + R8
            memTest.DRAM[6] = 0b0010010000010010010100000000000000000000000000000000000000000000; //DRAM[R10] = R9
            memTest.DRAM[7] = 0b0110000000001010010110001010000000000000000000000000000000000000; //R5 = R11 + R5
            memTest.DRAM[8] = 0b0001000000000000000000000000000000000001000000000000000000000100; //PC = PC - 4
        }

        return allInstructions;
    }

    sciter::string saveProgState(sciter::value fileName) {
        sciter::string fileNameWStr = fileName.get(L""); // convert value to string

        // Initialize program state stream and file
        wofstream progStateFile;
        progStateFile.open(fileNameWStr);

        // First serialize DRAM
        for (int i = 0; i < 256; i++) {
            progStateFile << memTest.DRAM[i] << "\n"; // stored as decimal values!
        }

        // Next serialize registers
        for (int i = 0; i < 64; i++) {
            progStateFile << memTest.registers[i] << "\n";
        }

        // Next serialize cache
        for (int i = 0; i < 16; i++){  // there are 16 cache sets
            for (int j = 0; j < 4; j++) { // each set has four cachelines
                progStateFile << "BEGINLINE_";
                progStateFile << "INDEX_" << memTest.cache[i][j].index << "_LRU_" << memTest.cache[i][j].LRU << "_TAG_" << memTest.cache[i][j].tag << "_BEGIN_WORDS_";
                for (int k = 0; k < 4; k++) { // and each cacheline contains four words
                    progStateFile << "A" << memTest.cache[i][j].word[k] << "B";
                }
                progStateFile << "_ENDLINE_";
            }
            progStateFile << "\n";
        }

        // Everything in the memory object should now be serialized to that .txt file
        progStateFile.close();

        return fileNameWStr;
    }

    sciter::string readInProgState(sciter::value fileName) {
        // convert sciter::value fileName to wstring
        sciter::string fileNameWStr = fileName.get(L""); // convert value to string

        // instantiate file reading objects
        wstring progStateLine;
        wifstream progStateFile(fileNameWStr);

        if (progStateFile.is_open())
        {
            int i = 0; // line bookkeeper
            while (getline(progStateFile, progStateLine)){
                if (0 <= i && i < 256) { // First read in DRAM data
                    memTest.DRAM[i] = stoll(progStateLine);
                } else if (256 <= i && i < (256 + 64)) { // next read in register data
                    memTest.registers[i - 256] = stoll(progStateLine);
                } else if ((256 + 64) <= i && i < (256 + 64 + 16)) { // now read in cache data (16 cache sets)
                    wstring cacheSet = progStateLine; // rename for readability
                    
                    for (int j = 0; j < 4; j++) { // four cachelines in each cache set
                        // Read in index
                        int indxPosStrt = cacheSet.find(WSTR("_INDEX_")) + 7;
                        int indxPosLen = cacheSet.find(WSTR("_LRU_")) - indxPosStrt;
                        int INDEX = stoi(cacheSet.substr(indxPosStrt, indxPosLen));
                        memTest.cache[i - (256 + 64)][j].index = INDEX;

                        // Read in LRU
                        int lruPosStrt = cacheSet.find(WSTR("_LRU_")) + 5;
                        int lruPosLen = (cacheSet.find(WSTR("_TAG_"))) - lruPosStrt;
                        int LRU = stoi(cacheSet.substr(lruPosStrt, lruPosLen));
                        memTest.cache[i - (256 + 64)][j].tag = LRU;

                        // Read in LRU
                        int tagPosStrt = cacheSet.find(WSTR("_TAG_")) + 5;
                        int tagPosLen = cacheSet.find(WSTR("_BEGIN_WORDS_")) - tagPosStrt;
                        long int TAG = stol(cacheSet.substr(tagPosStrt, tagPosLen));
                        memTest.cache[i - (256 + 64)][j].tag = TAG;

                        // Read in words
                        int allWordsPosStrt = cacheSet.find(WSTR("_BEGIN_WORDS_")) + 13;
                        int allWordsPosLen = cacheSet.find(WSTR("_ENDLINE_")) - allWordsPosStrt;
                        wstring allWords = cacheSet.substr(allWordsPosStrt, allWordsPosLen);
                        for (int k = 0; k < 4; k++) {
                            // Read in single word
                            int wordPosStrt = allWords.find(WSTR("A")) + 1;
                            int wordPosLen = allWords.find(WSTR("B")) - wordPosStrt;
                            long long int curWord = stoll(allWords.substr(wordPosStrt, wordPosLen));
                            memTest.cache[i - (256 + 64)][j].word[k] = curWord;

                            // chop off word we just read from allWords
                            int newAllWordsSrt = allWords.find(WSTR("B")) + 1;
                            int newAllWordsLen = allWords.length() - newAllWordsSrt;
                            allWords = allWords.substr(newAllWordsSrt, newAllWordsLen);
                        }
                        // chop off cache line we just read from cacheSet
                        int curCacheLineEndPos = cacheSet.find(WSTR("_ENDLINE_")) + 9;
                        int restOfCacheSetLen = cacheSet.length() - curCacheLineEndPos;
                        cacheSet = cacheSet.substr(curCacheLineEndPos, restOfCacheSetLen);
                    }
                }
                i++; // don't forget to increment your variables, kids!
            }
            progStateFile.close();
        }

        return fileNameWStr;
    }

    sciter::string runInstsructionsFor(sciter::value scitInstrsArgs) {
        memTest.instructionsStart = 0; // assume instructions are at beginning of DRAM for now
        globalPipeline = pipeline(memTest); // refresh pipeline object

        // convert sciter value to string
        wstring instrsArgs = scitInstrsArgs.get(L"");

        // parse out number of steps, cache bool, and pipeline bool
        int cacheBoolStrt = instrsArgs.find(WSTR("cacheBool_"))+10;
        if (instrsArgs.substr(cacheBoolStrt, 1).compare(WSTR("0")) == 0) {
            globalPipeline.cache = false;
        }
        bool pipeBool = true;
        int pipeBoolStrt = instrsArgs.find(WSTR("pipeBool_")) + 9;
        if (instrsArgs.substr(cacheBoolStrt, 1).compare(WSTR("0")) == 0) {
            pipeBool = false;
        }
        int numSteps = stoi(instrsArgs.substr(0, cacheBoolStrt));

        // tell pipeline where to stop
        memTest.instructionsEnd = memTest.instructionsStart + numSteps;

        // run pipeline
        if (pipeBool) {
            memTest = globalPipeline.runPipeline(memTest.instructionsStart, memTest.instructionsEnd);
        }
        else {
            memTest = globalPipeline.runWithoutPipeLine(memTest.instructionsStart, memTest.instructionsEnd);
        }
        
        
        // previous instructions have now been ran
        memTest.instructionsStart += numSteps;

        // reset instructions start once all have been ran
        if (memTest.instructionsStart == memTest.instructionsEnd) {
            memTest.instructionsStart = 0;
        }

        return WSTR("");
    }

    // This function is probably broken right now
    // most likely is behind on actual clockcount
    sciter::string getClockCount() {
        return to_wstring(globalPipeline.clock);
    }
};

#include "resources.cpp"

int uimain(std::function<int()> run) {

    sciter::debug_output_console console;// - uncomment it if you will need console window

    sciter::archive::instance().open(aux::elements_of(resources)); // bind resources[] (defined in "resources.cpp") with the archive

    frame* pwin = new frame();

    // note: this:://app URL is dedicated to the sciter::archive content associated with the application
    pwin->load(WSTR("this://app/main.htm"));

    pwin->expand();

    return run();
}