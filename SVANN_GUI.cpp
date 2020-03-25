// SVANN_GUI.cpp : Defines the entry point for the application.
//

// LAST EDITED BY CHASE ON 3/25
// --> DOING: passing data from memory.h to main.tis

#include "stdafx.h"
#include "SVANN_GUI.h"
#include "memory.h"

#include "sciter-x-window.hpp"

class frame : public sciter::window {
public:
    // INITIAL SETUP OF MEMORY OBJECT
    memory memTest;

    frame() : window(SW_TITLEBAR | SW_RESIZEABLE | SW_CONTROLS | SW_MAIN | SW_ENABLE_DEBUG) {}

    // map of native functions exposed to script:
    BEGIN_FUNCTION_MAP
        FUNCTION_0("nativeMessage", nativeMessage);
    END_FUNCTION_MAP

    // function expsed to script:
    sciter::string  nativeMessage() {
        string s = "0001 0001 0001";
        long long int addr = stoll(s.substr(4, 4));
        int reg = stoi(s.substr(9, 2));
        string address = memTest.decToBinary(addr);
        int set = stoi(address.substr(58, 4), 0, 2);

        sciter::string returnVal = to_wstring(memTest.registers[reg]);

        return returnVal;
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