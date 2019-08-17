#include <fstream>
#include <iomanip>

#include <nlohmann/json.hpp>

#include "lib/fifo_map.hpp"

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

extern std::ifstream input;
extern std::ofstream debug;
extern json debug_js;

int bytecode_error(uint8_t bytecode){
    std::cerr << "Error reading bytecode " << std::hex << std::showbase << (int) bytecode << " at " << (uint64_t) input.tellg() - 1 << "!!!" << std::endl;
    debug << std::setw(4) << debug_js;
    return 2;
}

int key_error(){
    std::cerr << "Error! Key at " << std::hex << std::showbase << (uint64_t) input.tellg() - 1 << " is not a string!!!" << std::endl;
    debug << std::setw(4) << debug_js;
    return 3;
}

int out_of_range_error(uint8_t bytecode){
    std::cerr << "Error! " << std::hex << std::showbase << (int) bytecode << " stack overflow at " << (uint64_t) input.tellg() - 1 << std::endl;
    debug << std::setw(4) << debug_js;
    return 4;
}

int eof_error(char footer[5]){
    std::cerr << "Error! End of RTON reached, expected \"DONE\" but found \"" << footer << "\" at " << std::hex << std::showbase << (uint64_t) input.tellg() - 1 << std::endl;
    debug << std::setw(4) << debug_js;
    return 5;
}

int not_supported_json(){
    std::cerr << "Error! This file is a JSON but format is not supported" << std::endl;
    debug << std::setw(4) << debug_js;
    return 6;
}
