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

const uint8_t float64               = 0x42;
const uint8_t uRTON_t               = 0x44;
const uint8_t RTON_t                = 0x45;
const uint8_t null                  = 0x84;
const uint8_t rtid                  = 0x83;
const uint8_t object                = 0x85;
const uint8_t arr                   = 0x86;
const uint8_t ascii                 = 0x90;
const uint8_t ascii_stack           = 0x91;
const uint8_t utf8                  = 0x92;
const uint8_t utf8_stack            = 0x93;
const uint8_t arr_begin             = 0xfd;
const uint8_t arr_end               = 0xfe;
const uint8_t object_end            = 0xff;

//import from main.cpp
std::string to_hex_string(std::vector <uint8_t> a);
std::string to_hex_string(uint64_t q);
extern std::ifstream input;
extern std::ofstream debug;
extern std::ofstream output;
extern json debug_js;

//import from RTON_number.cpp
std::vector <uint8_t> int2unsigned_RTON_num(uint64_t q);
uint64_t unsigned_RTON_num2int(std::vector <uint8_t> q);

std::unordered_map <std::string, uint64_t> map_0x91;
std::unordered_map <std::string, uint64_t> map_0x93;

int write_RTON(json js);

int not_json(){
    std::cerr << "Error! This file is not JSON format!!!" << std::endl;
    debug << std::setw(4) << debug_js;
    return 4;
}

//https://en.wikipedia.org/wiki/UTF-8#Examples
int get_utf8_size(std::string q){
    int utf8_size = 0;
    for (uint8_t i : q){
        if (i <= 0177) utf8_size += 1;
        if (i >= 0302 && i <= 0337) utf8_size += 1;
        if (i >= 0340 && i <= 0357) utf8_size += 1;
        if (i >= 0360 && i <= 0364) utf8_size += 1;
    }
    return utf8_size;
}

int write_unsigned_RTON_num(std::vector <uint8_t> a){
    for (uint8_t i : a) output.write(reinterpret_cast<const char*> (&i), sizeof i);
    return 0;
}

int write_RTON_block(json js){
    switch(js.type()){
        case json::value_t::null:{
            debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(null);
            output.write(reinterpret_cast<const char*> (&null), sizeof null);
            break;
        }
        case json::value_t::boolean:{
            bool temp = js.get<bool>();
            debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(temp);
            output.write(reinterpret_cast<const char*> (&temp), sizeof temp);
            break;
        }
        case json::value_t::string:{
            std::string temp = js.get<std::string>();
            //rtid
            if (std::regex_match(temp, std::regex("RTID(.*@.*)"))){
                output.write(reinterpret_cast<const char*> (&rtid), sizeof rtid);

                //delete "RTID(" and ")"
                temp.erase(temp.end() - 1);
                temp.erase(0, 5);

                std::string first_string = temp.substr(temp.find("@") + 1),
                            second_string = temp.substr(0, temp.find("@"));

                uint8_t subset;
                if (std::regex_match(second_string, std::regex("\\d+\\.\\d+\\.[0-9a-fA-F]+"))) subset = 0x2;
                else subset = 0x3;
                debug_js["RTON stats"]["List of bytecodes"][to_hex_string((uint64_t) output.tellp() - 1)] = to_hex_string(rtid*0x100 + subset);
                output.write(reinterpret_cast<const char*> (&subset), sizeof subset);

                write_unsigned_RTON_num(int2unsigned_RTON_num(get_utf8_size(first_string)));
                write_unsigned_RTON_num(int2unsigned_RTON_num(first_string.size()));
                output << first_string;
                if (subset == 0x2){
                    uint64_t first_uid = strtoull(second_string.c_str(), NULL, 10);
                    second_string = second_string.substr(second_string.find(".") + 1);
                    uint64_t second_uid = strtoull(second_string.c_str(), NULL, 10);
                    second_string = second_string.substr(second_string.find(".") + 1);
                    int32_t third_uid = std::stoi(second_string, nullptr, 16);

                    write_unsigned_RTON_num(int2unsigned_RTON_num(second_uid));
                    write_unsigned_RTON_num(int2unsigned_RTON_num(first_uid));
                    output.write(reinterpret_cast<const char*> (&third_uid), sizeof third_uid);
                }
                else{
                    write_unsigned_RTON_num(int2unsigned_RTON_num(get_utf8_size(second_string)));
                    write_unsigned_RTON_num(int2unsigned_RTON_num(second_string.size()));
                    output << second_string;
                }
            }
            //normal string
            else{
                int utf8_size = get_utf8_size(temp);
                //ascii
                if (utf8_size == temp.size()){
                    if (map_0x91[temp] == 0){
                        debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(ascii);
                        output.write(reinterpret_cast<const char*> (&ascii), sizeof ascii);
                        write_unsigned_RTON_num(int2unsigned_RTON_num(temp.size()));
                        output << temp;

                        debug_js["RTON stats"]["0x91 stack"][to_hex_string(int2unsigned_RTON_num(map_0x91.size() - 1))] = temp;
                        map_0x91[temp] = map_0x91.size();
                    }
                    else{
                        debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(ascii_stack);
                        output.write(reinterpret_cast<const char*> (&ascii_stack), sizeof ascii_stack);
                        write_unsigned_RTON_num(int2unsigned_RTON_num(map_0x91[temp] - 1));
                    }
                }
                //utf-8
                else{
                    if (map_0x93[temp] == 0){
                        debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(utf8);
                        output.write(reinterpret_cast<const char*> (&utf8), sizeof utf8);
                        write_unsigned_RTON_num(int2unsigned_RTON_num(utf8_size));
                        write_unsigned_RTON_num(int2unsigned_RTON_num(temp.size()));
                        output << temp;

                        debug_js["RTON stats"]["0x93 stack"][to_hex_string(int2unsigned_RTON_num(map_0x93.size() - 1))] = temp;
                        map_0x93[temp] = map_0x93.size();
                    }
                    else{
                        debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(utf8_stack);
                        output.write(reinterpret_cast<const char*> (&utf8_stack), sizeof utf8_stack);
                        write_unsigned_RTON_num(int2unsigned_RTON_num(map_0x93[temp] - 1));
                    }
                }
            }
            break;
        }
        case json::value_t::number_integer:{
            debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(RTON_t);
            output.write(reinterpret_cast<const char*> (&RTON_t), sizeof RTON_t);
            int64_t temp = js.get<int64_t>();
            write_unsigned_RTON_num(int2unsigned_RTON_num(temp < 0 ? -2 * temp - 1 : 2 * temp));
            break;
        }
        case json::value_t::number_unsigned:{
            debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(uRTON_t);
            output.write(reinterpret_cast<const char*> (&uRTON_t), sizeof uRTON_t);
            uint64_t temp = js.get<uint64_t>();
            write_unsigned_RTON_num(int2unsigned_RTON_num(temp));
            break;
        }
        case json::value_t::number_float:{
            debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(float64);
            output.write(reinterpret_cast<const char*> (&float64), sizeof float64);
            double temp = js.get<double>();
            output.write(reinterpret_cast<const char*> (&temp), sizeof temp);
            break;
        }
        case json::value_t::object:{
            debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(object);
            output.write(reinterpret_cast<const char*> (&object), sizeof object);
            write_RTON(js);
            break;
        }
        case json::value_t::array:{
            debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(arr);
            output.write(reinterpret_cast<const char*> (&arr), sizeof arr);

            debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(arr_begin);
            output.write(reinterpret_cast<const char*> (&arr_begin), sizeof arr_begin);

            write_unsigned_RTON_num(int2unsigned_RTON_num(js.size()));
            for (auto i : js) write_RTON_block(i);

            debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(arr_end);
            output.write(reinterpret_cast<const char*> (&arr_end), sizeof arr_end);
            break;
        }
        default:{
            throw not_json();
            break;
        }
    }
    return 0;
}

int write_RTON(json js){
    for (auto i : js.get<std::map <std::string, json> >()){
        write_RTON_block(i.first);
        write_RTON_block(i.second);
    }
    debug_js["RTON stats"]["List of bytecodes"][to_hex_string(output.tellp())] = to_hex_string(object_end);
    output.write(reinterpret_cast<const char*> (&object_end), 1);
    return 0;
}

int rton_encode(){
    map_0x91.clear();
    map_0x93.clear();

    json js;
    try{
        js = json::parse(input);
    }
    catch(json::exception &e){
        throw not_json();
    }

    output.write("RTON", 4);
    const int RTON_ver = 1; //not sure if I ever see RTON version higher than 1
    output.write(reinterpret_cast <const char*> (&RTON_ver), sizeof RTON_ver);
    write_RTON(js);
    output.write("DONE", 4);

    return 0;
}