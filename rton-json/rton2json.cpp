#include <conio.h>
#include <fstream>
#include <iomanip>
#include <iostream>

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
void bytecode_error();

int not_RTON(){
    std::clog << "ERROR! THIS FILE IS NOT RTON FORMAT!!!" << std::endl;
    debug << std::setw(4) << debug_js;
    getch();
    return 1;
}

std::vector <uint8_t> read_RTON_num(){
    std::vector <uint8_t> RTON_num;
    uint8_t sub_num;
    input.read(reinterpret_cast <char*> (&sub_num), sizeof sub_num);
    RTON_num.push_back(sub_num);
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
    debug_js["RTON Stats"]["List of Bytecodes"].push_back(to_hex_string(input.tellg() - 1) + ": " + to_hex_string(bytecode));
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
        //signed RTON number
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
        //signed RTON number???
        case 0x29:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            if (num % 2) num = -(num + 1);
            num /= 2;
            res.push_back(num);
            break;
        }
        //0.0???
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
        //unsigned RTON number???
        case 0x44:{
            res.push_back(unsigned_RTON_num2int(read_RTON_num()));
            break;
        }
        //signed RTON number???
        case 0x45:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            if (num % 2) num = -(num + 1);
            num /= 2;
            res.push_back(num);
            break;
        }
        //string
        case 0x81:{
            //get buffer
            uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());

            //read buffer
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;
            res.push_back(std::string(temp));
            break;
        }
        case 0x82:{
            //get string buffer
            uint64_t s_buffer = unsigned_RTON_num2int(read_RTON_num());
            s_buffer = unsigned_RTON_num2int(read_RTON_num());
            char s[s_buffer +1];
            input.read(s, s_buffer);
            s[s_buffer] = 0;
            res.push_back(std::string(s));
            break;
        }
        //RTID
        case 0x83:{
            uint8_t check;
            input.read(reinterpret_cast <char*> (&check), sizeof check);
            //check for 03
            if (check == 0x3){
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
                res.push_back(std::string() + "RTID(" + s2 + '@' + s1 + ')');
            }
            else bytecode_error();
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
            //check for fd
            uint8_t check;
            input.read(reinterpret_cast <char*> (&check), sizeof check);
            if (check == 0xfd){
                size_t num_elements = unsigned_RTON_num2int(read_RTON_num());
                json arr = json::array();
                for (int i = 0; i < num_elements; ++i) arr.push_back(read_RTON_block()[0]);
                res.push_back(arr);
                //check end of array
                uint8_t check_end;
                input.read(reinterpret_cast <char*> (&check_end), sizeof check_end);
                if (check_end != 0xfe) bytecode_error();
            }
            else bytecode_error();
            break;
        }
        //substitute
        case 0x90:{
            //get buffer
            uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());

            //read buffer
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;

            //logging
            debug_js["RTON Stats"]["0x91 Stack"].push_back(to_hex_string(int2unsigned_RTON_num(stack_0x91.size())) + ": " + std::string(temp));

            //push to stack_0x91 and write json
            stack_0x91.push_back(temp);
            res.push_back(stack_0x91[stack_0x91.size() - 1]);
            break;
        }
        case 0x91:{
            //substitute
            res.push_back(stack_0x91[unsigned_RTON_num2int(read_RTON_num())]);
            break;
        }
        //substitute
        case 0x92:{
            //get buffer
            uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());
            buffer = unsigned_RTON_num2int(read_RTON_num());

            //read buffer
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;

            //logging
            debug_js["RTON Stats"]["0x93 Stack"].push_back(to_hex_string(int2unsigned_RTON_num(stack_0x93.size())) + ": " + std::string(temp));

            //push to stack_0x92 and write json
            stack_0x93.push_back(temp);
            res.push_back(stack_0x93[stack_0x93.size() - 1]);
            break;
        }
        case 0x93:{
            //substitute
            res.push_back(stack_0x93[unsigned_RTON_num2int(read_RTON_num())]);
            break;
        }
        //end of object
        case 0xFF:{
            break;
        }
        //else just exit error
        default:{
            bytecode_error();
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
        json js_key = read_RTON_block();
        //TODO: check "DONE" at end of RTON file
        if (js_key.size() == 0) return res;
        else{
            if (!js_key[0].is_string()) bytecode_error();
            key = js_key[0];
        }
        //prevent push entire array lol
        json value = read_RTON_block()[0];
        res[key] = value;
    }
}

json json_decode(){
    //check header
    char header[5];
    input.read(header, 4);
    header[4] = 0;
    if (strcmp(header, "RTON") != 0) exit(not_RTON());
    uint32_t RTON_ver;
    input.read(reinterpret_cast <char*> (&RTON_ver), sizeof RTON_ver);

    json js = read_RTON();
    //check footer
    char footer[5];
    input.read(footer, 4);
    footer[4] = 0;
    if (strcmp(footer, "DONE") != 0) std::clog << R"(MISSING "DONE" AT EOF?)" << std::endl;
    return js;
}

void bytecode_error(){
    uint8_t bytecode;
    input.seekg(input.tellg() - 1);
    input.read(reinterpret_cast <char*> (&bytecode), sizeof bytecode);
    std::clog << std::endl << "ERROR READING BYTECODE " << std::hex << std::showbase << (int)bytecode << " AT " << input.tellg() - 1 << "!!!" << std::endl;
    debug << std::setw(4) << debug_js;
    getch();
    exit(1);
}
