// SVANN_GUI.cpp : Defines the entry point for the application.
//

// LAST EDITED BY CHASE ON 3/25
// --> DOING: passing data from memory.h to main.tis

#define _CRT_SECURE_NO_WARNINGS

#include "stdafx.h"
#include "SVANN_GUI.h"
#include "memory.h"
#include <Python.h>
#include <stdio.h>
#include <locale>
#include <codecvt>
#include <string>

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
        FUNCTION_1("uploadFileEvent", uploadFileEvent);
    END_FUNCTION_MAP

    sciter::string grabDataFrom_Cache() {
        
        // all words from cache and place them in a string
        // return the string to .tis and create array based on delimeters
        sciter::string cacheStr = WSTR("");
        for (int i = 0; i < 16; i++) {
            for (int j = 0; j < 4; j++) {
                for (int k = 0; k < 4; k++) {
                    cacheStr += to_wstring(memTest.cache[i][j].word[k]);
                };
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
            dramStr += to_wstring(memTest.DRAM[i]) + WSTR(" ");
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

    // This function is NOT optimized
    sciter::string uploadFileEvent(sciter::value fileName)
    {
        

        // READ INSTRUCTIONS FILE ONCE FOR RETURN TO TIS SCRIPT
        sciter::string fileNameStr = fileName.get(L""); // convert value to string
        sciter::string allInstructions = WSTR("");
        sciter::string instructionLine;
        std::wifstream instructionsFile(fileNameStr);
        if (instructionsFile.is_open()){
            while (getline(instructionsFile, instructionLine)){
                allInstructions += instructionLine + WSTR("-");
            }
            instructionsFile.close();
        }

        // CALL PYTHON SCRIPT TO READ INSTRUCTIONS FILE AND WRITE CONVERTED BINARY FILE
        const char* stdFileStr = aux::w2a(fileNameStr);

        FILE* file;
        int argc;
        const char* argv[3];

        argc = 2;
        argv[0] = WSTR("535Assembler.py");
        argv[1] = fileNameStr;

        Py_SetProgramName(WSTR("535Assembler.py"));
        Py_Initialize();
        PySys_SetArgv(2, {WSTR("535Assembler.py"), fileNameStr});
        file = fopen("mypy.py", "r");
        PyRun_SimpleFile(file, "mypy.py");
        Py_Finalize();


        char filename[] = "535Assembler.py";
        FILE* fp;
        Py_Initialize();
        fp = _Py_fopen(filename, "r");
        PyRun_SimpleFile(fp, filename);
        Py_Finalize();

        return allInstructions;
    }
};

#include "resources.cpp"

int uimain(std::function<int()> run) {

    //sciter::debug_output_console console; - uncomment it if you will need console window

    sciter::archive::instance().open(aux::elements_of(resources)); // bind resources[] (defined in "resources.cpp") with the archive

    frame* pwin = new frame();

    // note: this:://app URL is dedicated to the sciter::archive content associated with the application
    pwin->load(WSTR("this://app/main.htm"));

    pwin->expand();

    return run();
}