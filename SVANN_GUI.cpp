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

#include "sciter-x-window.hpp"
#include "value.hpp"

class frame : public sciter::window {
public:
    // INITIAL SETUP OF MEMORY OBJECT
    memory memTest;

    frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS | SW_MAIN | SW_ENABLE_DEBUG) {}

    // map of native functions exposed to script:
    BEGIN_FUNCTION_MAP
        FUNCTION_0("grabDataFrom_Cache", grabDataFrom_Cache);
        FUNCTION_0("grabDataFrom_DRAM", grabDataFrom_DRAM);
        FUNCTION_0("grabDataFrom_Registers", grabDataFrom_Registers);
        FUNCTION_1("uploadInstructionsFile", uploadInstructionsFile);
        FUNCTION_1("saveProgState", saveProgState);
        FUNCTION_1("readInProgState", readInProgState);
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

            dramStr += str + WSTR(" ");
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
        // Open file and read all instructions into one big sciter::string
        sciter::string fileNameWStr = fileName.get(L""); // convert value to string
        sciter::string allInstructions = WSTR(""); // this is what gets returned to the TIS script
        sciter::string instructionLine;
        std::wifstream instructionsFile(fileNameWStr);
        if (instructionsFile.is_open()){
            while (getline(instructionsFile, instructionLine)){
                allInstructions += instructionLine + WSTR("-");
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
        system("python 535Assembler.py instructionsTest.txt");

        // read binary instructions file and place instructions in DRAM
        std::string binInstrs = "";
        std::string assemInsrLine;
        std::string tmp1 = stdFileName;
        std::string binInstrFileName = stdFileName.substr(0, stdFileName.length() - 4) + "Binary.txt";
        ifstream binInstrFile(binInstrFileName);
        int i = 0;
        if (binInstrFile.is_open())
        {
            while (getline(binInstrFile, assemInsrLine))
            {
                if (assemInsrLine.compare("") != 0) {
                    memTest.DRAM[i] = stoll(assemInsrLine, 0 ,2);
                    i++;
                }
            }
            binInstrFile.close();
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