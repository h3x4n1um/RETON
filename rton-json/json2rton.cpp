#include <fstream>
#include <iomanip>
#include <regex>

#include "fifo_map.hpp"
#include "json.hpp"

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

const uint8_t signed_int    = 0x20; //int32_t
const uint8_t unsigned_int  = 0x26; //uint32_t
const uint8_t float64       = 0x42;
const uint8_t str           = 0x81; //string 0x81
const uint8_t str_rtid      = 0x82; //string 0x82 (use in RTID)
const uint8_t null          = 0x84;
const uint8_t sub_object    = 0x85;
const uint8_t eoa           = 0xfe; //end of array
const uint8_t eoo           = 0xff; //end of object

const uint16_t rtid         = 0x0383; //rtid 83 03
const uint16_t arr          = 0xfd86; //array 86 fd

//import from main.cpp
extern std::string to_hex_string(uint64_t q);
extern std::ifstream input;
extern std::ofstream debug;
extern std::ofstream output;
extern json debug_js;
//import from RTON_number.cpp
extern std::vector <uint8_t> int2unsigned_RTON_num(uint64_t q);
extern uint64_t unsigned_RTON_num2int(std::vector <uint8_t> q);

int write_RTON(json js);

int json_error(){
    std::cout << "ERROR! THIS FILE IS NOT JSON FORMAT!!!\n";
    debug << std::setw(4) << debug_js;
    exit(1);
}

int write_unsigned_RTON_num(std::vector <uint8_t> a){
    for (uint8_t i : a) output.write(reinterpret_cast<const char*> (&i), sizeof i);
    return 0;
}

int write_RTON_block(json js){
    switch(js.type()){
        //null
        case json::value_t::null:{
            output.write(reinterpret_cast<const char*> (&null), sizeof null);
            break;
        }
        //boolean
        case json::value_t::boolean:{
            bool temp = js.get<bool>();
            output.write(reinterpret_cast<const char*> (&temp), sizeof temp);
            break;
        }
        //string
        case json::value_t::string:{
            std::string temp = js.get<std::string>();
            //rtid
            if (std::regex_match(temp, std::regex("(RTID()(.*)(@)(.*)())"))){
                output.write(reinterpret_cast<const char*> (&rtid), sizeof rtid);
                //delete "RTID(" and ")"
                temp.erase(temp.end() - 1);
                temp.erase(0, 5);
                //get 2 strings
                std::string first_string = temp.substr(temp.find("@") + 1),
                            second_string = temp.substr(0, temp.find("@"));
                write_unsigned_RTON_num(int2unsigned_RTON_num(first_string.size()));
                write_unsigned_RTON_num(int2unsigned_RTON_num(first_string.size()));
                output << first_string;
                write_unsigned_RTON_num(int2unsigned_RTON_num(second_string.size()));
                write_unsigned_RTON_num(int2unsigned_RTON_num(second_string.size()));
                output << second_string;
            }
            //normal string
            else{
                output.write(reinterpret_cast<const char*> (&str), sizeof str);
                write_unsigned_RTON_num(int2unsigned_RTON_num(temp.size()));
                output << temp;
            }
            break;
        }
        //int
        case json::value_t::number_integer:{
            output.write(reinterpret_cast<const char*> (&signed_int), sizeof signed_int);
            int temp = js.get<int>();
            output.write(reinterpret_cast<const char*> (&temp), sizeof temp);
            break;
        }
        //uint
        case json::value_t::number_unsigned:{
            output.write(reinterpret_cast<const char*> (&unsigned_int), sizeof unsigned_int);
            uint32_t temp = js.get<uint32_t>();
            output.write(reinterpret_cast<const char*> (&temp), sizeof temp);
            break;
        }
        //float
        case json::value_t::number_float:{
            output.write(reinterpret_cast<const char*> (&float64), sizeof float64);
            double temp = js.get<double>();
            output.write(reinterpret_cast<const char*> (&temp), sizeof temp);
            break;
        }
        //object
        case json::value_t::object:{
            output.write(reinterpret_cast<const char*> (&sub_object), sizeof sub_object);
            write_RTON(js);
            break;
        }
        //array
        case json::value_t::array:{
            output.write(reinterpret_cast<const char*> (&arr), sizeof arr);
            write_unsigned_RTON_num(int2unsigned_RTON_num(js.size()));
            for (auto i : js) write_RTON_block(i);
            output.write(reinterpret_cast<const char*> (&eoa), sizeof eoa);
            break;
        }
        //error
        default:{
            return json_error();
            break;
        }
    }
}

int write_RTON(json js){
    for (auto i : js.get<std::map <std::string, json> >()){
        write_RTON_block(i.first);
        write_RTON_block(i.second);
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
