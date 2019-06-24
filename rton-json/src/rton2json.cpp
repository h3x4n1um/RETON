#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "fifo_map.hpp"
#include "json.hpp"

//a workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
//https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

//import from main.cpp
std::string to_hex_string(uint64_t q);
std::string to_hex_string(std::vector <uint8_t> a);
extern std::ifstream input;
extern std::ofstream debug;
extern json debug_js;
//import from RTON_number.cpp
std::vector <uint8_t> int2unsigned_RTON_num(uint64_t q);
uint64_t unsigned_RTON_num2int(std::vector <uint8_t> q);

std::vector <std::string> stack_0x91;
std::vector <std::string> stack_0x93;

json read_RTON();
int bytecode_error(uint8_t bytecode);

std::vector <uint8_t> read_RTON_num(){
    std::vector <uint8_t> RTON_num;
    uint8_t sub_num = 0x80;
    while(sub_num > 0x7f){
        input.read(reinterpret_cast <char*> (&sub_num), sizeof sub_num);
        RTON_num.push_back(sub_num);
    }
    return RTON_num;
}

json read_RTON_block(){
    uint8_t bytecode;
    json res;
    //read bytecode
    input.read(reinterpret_cast <char*> (&bytecode), sizeof bytecode);
    //logging
    debug_js["RTON Stats"]["List of Bytecodes"][to_hex_string((uint64_t) input.tellg() - 1)] = to_hex_string(bytecode);
    //split case
    switch (bytecode){
        //false
        case 0x0:{
            res.push_back(false);
            break;
        }
        //true
        case 0x1:{
            res.push_back(true);
            break;
        }
        //int8_t
        case 0x8:{
            int8_t num;
            input.read(reinterpret_cast <char*> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        //0???
        case 0x9:{
            res.push_back(0);
            break;
        }
        //uint8_t
        case 0xa:{
            uint8_t num;
            input.read(reinterpret_cast <char*> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        //0???
        case 0xb:{
            res.push_back(0);
            break;
        }
        //int16_t
        case 0x10:{
            int16_t num;
            input.read(reinterpret_cast <char*> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        //0???
        case 0x11:{
            res.push_back(0);
            break;
        }
        //uint16_t
        case 0x12:{
            uint16_t num;
            input.read(reinterpret_cast <char*> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        //0???
        case 0x13:{
            res.push_back(0);
            break;
        }
        //int32_t
        case 0x20:{
            int32_t num;
            input.read(reinterpret_cast <char*> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        //0???
        case 0x21:{
            res.push_back(0);
            break;
        }
        //float32
        case 0x22:{
            float num;
            input.read(reinterpret_cast <char*> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        //0.0???
        case 0x23:{
            res.push_back(0.0);
            break;
        }
        //unsigned RTON number
        case 0x24:{
            res.push_back(unsigned_RTON_num2int(read_RTON_num()));
            break;
        }
        //RTON number
        case 0x25:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            if (num % 2) num = -(num + 1);
            num /= 2;
            res.push_back(num);
            break;
        }
        //uint32_t
        case 0x26:{
            uint32_t num;
            input.read(reinterpret_cast <char*> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        //0???
        case 0x27:{
            res.push_back(0);
            break;
        }
        //unsigned RTON number???
        case 0x28:{
            res.push_back(unsigned_RTON_num2int(read_RTON_num()));
            break;
        }
        //RTON number???
        case 0x29:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            if (num % 2) num = -(num + 1);
            num /= 2;
            res.push_back(num);
            break;
        }
        //0.0???
        /*TODO: verify 0x41 type in rton*/
        case 0x41:{
            res.push_back(0.0);
            break;
        }
        //float64
        case 0x42:{
            double num;
            input.read(reinterpret_cast <char*> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        /*I guess it is 0.0 in double via previous bytecode in the cheatsheet (type + 0 in that type)???
        but if 0x43 is 0.0 in double then wtf is 0x41???*/
        //0.0???
        case 0x43:{
            res.push_back(0.0);
            break;
        }
        //unsigned RTON number???
        case 0x44:{
            res.push_back(unsigned_RTON_num2int(read_RTON_num()));
            break;
        }
        //RTON number???
        case 0x45:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            if (num % 2) num = -(num + 1);
            num /= 2;
            res.push_back(num);
            break;
        }
        //string
        case 0x81:{
            uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;
            res.push_back(std::string(temp));
            break;
        }
        //utf-8 string
        case 0x82:{
            uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());
            buffer = unsigned_RTON_num2int(read_RTON_num());
            char s[buffer + 1];
            input.read(s, buffer);
            s[buffer] = 0;
            res.push_back(std::string(s));
            break;
        }
        //RTID
        case 0x83:{
            uint8_t subset;
            input.read(reinterpret_cast <char*> (&subset), sizeof subset);
            //subset 0x3
            if (subset == 0x3){
                //get 1st string
                uint64_t s1_buffer = unsigned_RTON_num2int(read_RTON_num());
                s1_buffer = unsigned_RTON_num2int(read_RTON_num());
                char s1[s1_buffer + 1];
                input.read(s1, s1_buffer);
                s1[s1_buffer] = 0;
                //get 2nd string
                uint64_t s2_buffer = unsigned_RTON_num2int(read_RTON_num());
                s2_buffer = unsigned_RTON_num2int(read_RTON_num());
                char s2[s2_buffer + 1];
                input.read(s2, s2_buffer);
                s2[s2_buffer] = 0;
                res.push_back(std::string("RTID(") + s2 + '@' + s1 + ')');
            }
            //subset 0x2
            else if (subset == 0x2){
                uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());
                buffer = unsigned_RTON_num2int(read_RTON_num());
                char s[buffer + 1];
                input.read(s, buffer);
                s[buffer] = 0;

                uint64_t second_uid = unsigned_RTON_num2int(read_RTON_num());
                uint64_t first_uid = unsigned_RTON_num2int(read_RTON_num());
                int32_t third_uid;
                input.read(reinterpret_cast <char *> (&third_uid), sizeof third_uid);

                std::stringstream ss;
                std::string uid;
                ss << std::dec << first_uid << '.' << second_uid << '.' << std::hex << third_uid;
                ss >> uid;
                res.push_back(std::string("RTID(") + uid + '@' + s + ')');
            }
            //unknown
            else throw bytecode_error(subset);
            break;
        }
        //null
        case 0x84:{
            res.push_back(nullptr);
            break;
        }
        //sub-object
        case 0x85:{
            res.push_back(read_RTON());
            break;
        }
        //array
        case 0x86:{
            uint8_t arr_begin;
            input.read(reinterpret_cast <char*> (&arr_begin), sizeof arr_begin);
            if (arr_begin == 0xfd){
                size_t arr_size = unsigned_RTON_num2int(read_RTON_num());
                json arr = json::array();
                for (int i = 0; i < arr_size; ++i) arr.push_back(read_RTON_block()[0]);
                res.push_back(arr);
                //check end of array
                uint8_t arr_end;
                input.read(reinterpret_cast <char*> (&arr_end), sizeof arr_end);
                if (arr_end != 0xfe) throw bytecode_error(arr_end);
            }
            else throw bytecode_error(arr_begin);
            break;
        }
        //cached string
        case 0x90:{
            //get buffer
            uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());
            //read buffer
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;
            //logging
            debug_js["RTON stats"]["0x91 stack"][to_hex_string(int2unsigned_RTON_num(stack_0x91.size()))] = std::string(temp);
            //push to stack_0x91 and write json
            stack_0x91.push_back(temp);
            res.push_back(stack_0x91[stack_0x91.size() - 1]);
            break;
        }
        case 0x91:{
            //recall
            res.push_back(stack_0x91[unsigned_RTON_num2int(read_RTON_num())]);
            break;
        }
        //cached utf-8 string
        case 0x92:{
            //get buffer
            uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());
            buffer = unsigned_RTON_num2int(read_RTON_num());
            //read buffer
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;
            //logging
            debug_js["RTON stats"]["0x93 stack"][to_hex_string(int2unsigned_RTON_num(stack_0x93.size()))] = std::string(temp);
            //push to stack_0x93 and write json
            stack_0x93.push_back(temp);
            res.push_back(stack_0x93[stack_0x93.size() - 1]);
            break;
        }
        case 0x93:{
            //recall
            res.push_back(stack_0x93[unsigned_RTON_num2int(read_RTON_num())]);
            break;
        }
        //end of object
        case 0xFF:{
            break;
        }
        //else just exit error
        default:{
            throw bytecode_error(bytecode);
            break;
        }
    }
    return res;
}

json read_RTON(){
    //decoding
    json res;
    while(true){
        std::string key;
        json js_key;
        js_key = read_RTON_block();
        if (js_key.size() == 0) return res;
        else{
            if (!js_key[0].is_string()){
                std::cerr << "Error! Key is not a string!!!" << std::endl;
                debug << std::setw(4) << debug_js;
                throw 3;
            }
            key = js_key[0];
        }
        //prevent push entire array lol
        json value = read_RTON_block()[0];
        res[key] = value;
    }
}

json json_decode(){
    stack_0x91.clear();
    stack_0x93.clear();
    input.seekg((uint64_t) input.tellg() + 4); //skip RTON

    uint32_t RTON_ver;
    input.read(reinterpret_cast <char*> (&RTON_ver), sizeof RTON_ver);

    json js;
    js = read_RTON();

    //check footer
    char footer[5];
    input.read(footer, 4);
    footer[4] = 0;
    if (strcmp(footer, "DONE") != 0) std::clog << "Missing \"DONE\" at EOF?" << std::endl;
    return js;
}

int bytecode_error(uint8_t bytecode){
    std::cerr << "Error reading bytecode " << std::hex << std::showbase << (int) bytecode << " at " << (uint64_t) input.tellg() - 1 << "!!!" << std::endl;
    debug << std::setw(4) << debug_js;
    return 2;
}
