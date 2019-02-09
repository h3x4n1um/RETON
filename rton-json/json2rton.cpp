#include "fifo_map.hpp"
#include "json.hpp"

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

//import from main.cpp
extern std::string to_hex_string(uint64_t q);
extern std::ifstream input;
extern std::ofstream debug;
extern json debug_js;
//import from RTON_number.cpp
extern uint64_t int2unsigned_RTON_num(uint64_t q);
extern uint64_t unsigned_RTON_num2int(std::vector <uint8_t> q);

std::string rton_encode(){
    std::string res;
    return res;
}
