#include "include/rton-json.hpp"

#include "include/error.hpp"

int bytecode_error(uint8_t bytecode){
    cerr << "Error reading bytecode " << hex << showbase << (int) bytecode << " at " << (uint64_t) input.tellg() - 1 << "!!!" << endl;
    debug << setw(4) << debug_js;
    return 2;
}

int key_error(){
    cerr << "Error! Key at " << hex << showbase << (uint64_t) input.tellg() - 1 << " is not a string!!!" << endl;
    debug << setw(4) << debug_js;
    return 3;
}

int out_of_range_error(uint8_t bytecode){
    cerr << "Error! " << hex << showbase << (int) bytecode << " stack overflow at " << (uint64_t) input.tellg() - 1 << endl;
    debug << setw(4) << debug_js;
    return 4;
}

int eof_error(char footer[5]){
    cerr << "Error! End of RTON reached, expected \"DONE\" but found \"" << footer << "\" at " << hex << showbase << (uint64_t) input.tellg() - 1 << endl;
    debug << setw(4) << debug_js;
    return 5;
}

int not_supported_json(){
    cerr << "Error! This file is a JSON but format is not supported" << endl;
    debug << setw(4) << debug_js;
    return 6;
}
