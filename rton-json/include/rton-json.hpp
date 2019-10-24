#pragma once

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include "lib/fifo_map.hpp"

using namespace std;

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

#if INTPTR_MAX == INT64_MAX
// 64-bit
#  define ARCHITECTURE "64-bit"
#elif INTPTR_MAX == INT32_MAX
// 32-bit
#  define ARCHITECTURE "32-bit"
#else
#  define ARCHITECTURE "Unknown"
#endif
const string architecture = ARCHITECTURE;

const string ver = "2.7.7";

extern ifstream input;
extern ofstream output, debug;
extern json debug_js, rton_list, json_list;

inline string to_hex_string(uint64_t q){
    stringstream ss;
    ss << "0x" << hex << q;
    return ss.str();
}

inline string to_hex_string(vector <uint8_t> a){
    stringstream ss;
    ss << "0x";
    for (uint8_t i : a){
        ss << setfill('0') << setw(2) << hex << (int) i;
    }
    return ss.str();
}
