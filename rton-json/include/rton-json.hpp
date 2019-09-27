#pragma once

#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include <nlohmann/json.hpp>
#include "lib/fifo_map.hpp"

#include "version.hpp"

using namespace std;

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

const string ver = to_string(VERSION_MAJOR) + '.' + to_string(VERSION_MINOR) + '.' + to_string(VERSION_PATCH);
const string architecture = ARCHITECTURE;

extern ifstream input;
extern ofstream output, debug;
extern json debug_js, rton_list, json_list;

extern string to_hex_string(uint64_t q);
extern string to_hex_string(vector <uint8_t> a);
