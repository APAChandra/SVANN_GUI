// SVANN_GUI.cpp : Defines the entry point for the application.
//

// LAST EDITED BY CHASE ON 3/25
// --> DOING: passing data from memory.h to main.tis

#include "stdafx.h"
#include "SVANN_GUI.h"
#include "memory.h"

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
    END_FUNCTION_MAP

    // function expsed to script:
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