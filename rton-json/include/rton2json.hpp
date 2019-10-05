#pragma once

#include <vector>

#include <nlohmann/json.hpp>
#include "lib/fifo_map.hpp"

using namespace std;

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

template <class T>
T read();

vector <uint8_t> read_uRTON_t();

json json_decode();
json read_RTON();
json read_RTON_block();
