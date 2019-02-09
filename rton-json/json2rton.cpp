#include <fstream>
#include <iomanip>

#include "fifo_map.hpp"
#include "json.hpp"

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

const uint8_t str = 0x81; //string 0x81
const uint8_t str_rtid = 0x82; //string 0x82 (use in RTID)
const uint8_t null = 0x84;
const uint8_t sub_object = 0x85;
const uint8_t eoa = 0xfe; //end of array
const uint8_t eoo = 0xff; //end of object
const uint16_t rtid = 0x8303; //array
const uint16_t arr = 0x86fd; //array

//import from main.cpp
extern std::string to_hex_string(uint64_t q);
extern std::ifstream input;
extern std::ofstream debug;
extern std::ofstream output;
extern json debug_js;
//import from RTON_number.cpp
extern uint64_t int2unsigned_RTON_num(uint64_t q);
extern uint64_t unsigned_RTON_num2int(std::vector <uint8_t> q);

int json_error(){
    std::cout << "ERROR! THIS FILE IS NOT JSON FORMAT!!!\n";
    debug << std::setw(4) << debug_js;
    exit(1);
}

int write_RTON(json js){
    for (auto i : js.get<std::map <std::string, json> >()){
        output.write(reinterpret_cast<const char*> (&str), sizeof str);
        output << i.first;
        switch(i.second.type()){
            //null
            case json::value_t::null:{
                output.write(reinterpret_cast<const char*> (&null), sizeof null);
                break;
            }
            //boolean
            case json::value_t::boolean:{
                bool temp = js.at(i.first).get<bool>();
                output.write(reinterpret_cast<const char*> (&temp), sizeof temp);
                break;
            }
            //string
            case json::value_t::string:{
                break;
            }
            //int
            case json::value_t::number_integer:{
                break;
            }
            //uint
            case json::value_t::number_unsigned:{
                break;
            }
            //float
            case json::value_t::number_float:{
                break;
            }
            //object
            case json::value_t::object:{
                output.write(reinterpret_cast<const char*> (&sub_object), sizeof sub_object);
                write_RTON(i.second);
                break;
            }
            //array
            case json::value_t::array:{
                break;
            }
            //error
            default:{
                return json_error();
                break;
            }
        }
    }
    output.write(reinterpret_cast<const char*> (&eoo), 1);
    return 0;
}

int rton_encode(){
    json js;
    try{
        input >> js;
    }
    catch(nlohmann::detail::parse_error){
        return json_error();
    }
    output.write("RTON", 4);
    const int RTON_ver = 1; //not sure if I ever see RTON version higher than 1
    output.write(reinterpret_cast <const char*> (&RTON_ver), sizeof RTON_ver);
    write_RTON(js);
    output.write("DONE", 4);
    return 0;
}
