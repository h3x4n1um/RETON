#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>

#include "json.hpp"
#include "fifo_map.hpp"

constexpr double log256(double q){
    return log2(q) / 8;
}

using namespace std;
/// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
/// https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

extern ifstream input;
extern ofstream debug;
extern json debug_js;
vector <string> prev_stack;

json json_decode();
void bytecode_error();

uint64_t int2unsigned_RTON_num(uint64_t q){
    if (q <= 0x7f) return q;
    uint64_t first_byte = q % 0x100;
    q /= 0x100;
    uint64_t second_byte = q * 2;
    if (first_byte > 0x7f) ++second_byte;
    else first_byte += 0x80; //reverse & 0x7f
    uint64_t new_second_byte = int2unsigned_RTON_num(second_byte);
    return first_byte * pow(0x100, ceil(log256(new_second_byte)) ? ceil(log256(new_second_byte)) : 1) + new_second_byte;
}

uint64_t unsigned_RTON_num2int(vector <uint8_t> q){
    if (q.size() == 1){
        if (q[0] > 0x7f) return UINT_MAX; //return max when RTON number has 1 byte and > 0x7f
        else return q[0];
    }
    uint64_t near_last_byte, last_byte = q[q.size() - 1];
    q.pop_back();
    while(q.size() > 0){
        near_last_byte = q[q.size() - 1];
        if (last_byte % 2 == 0) near_last_byte &= 0x7f;
        near_last_byte += last_byte / 2 * 0x100;
        last_byte = near_last_byte;
        q.pop_back();
    }
    return last_byte;
}

vector <uint8_t> read_RTON_num(){
    vector <uint8_t> RTON_num;
    uint8_t sub_num;
    input.read(reinterpret_cast <char *> (&sub_num), sizeof sub_num);
    RTON_num.push_back(sub_num);
    while(sub_num > 0x7f){
        input.read(reinterpret_cast <char *> (&sub_num), sizeof sub_num);
        RTON_num.push_back(sub_num);
    }
    return RTON_num;
}

string to_hex_string(uint64_t q){
    if (q == 0) return "0x0";
    string s;
    stringstream ss;
    ss << hex << showbase << q;
    ss >> s;
    return s;
}

json read_block_RTON(){
    uint8_t bytecode;
    json res;
    ///read bytecode
    input.read(reinterpret_cast <char *> (&bytecode), sizeof bytecode);
    ///logging
    debug_js["List of Bytecodes"].push_back(json::object({
        {"Offset", to_hex_string(input.tellg() - 1)},
        {"Bytecode", to_hex_string(bytecode)}
    }));
    ///split case
    switch (bytecode){
        ///false
        case 0x0:{
            res.push_back(false);
            break;
        }
        ///true
        case 0x1:{
            res.push_back(true);
            break;
        }
        ///uint8_t
        case 0xa:{
            uint8_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        ///0???
        case 0xb:{
            res.push_back(0);
            break;
        }
        ///int16_t
        case 0x10:{
            int16_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        ///0???
        case 0x11:{
            res.push_back(0);
            break;
        }
        ///uint16_t
        case 0x12:{
            uint16_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        ///int32_t
        case 0x20:{
            int32_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        ///0???
        case 0x21:{
            res.push_back(0);
            break;
        }
        ///float32
        case 0x22:{
            float num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        ///0.0???
        case 0x23:{
            res.push_back(0.0);
            break;
        }
        ///unsigned RTON number
        case 0x24:{
            res.push_back(unsigned_RTON_num2int(read_RTON_num()));
            break;
        }
        ///signed RTON number
        case 0x25:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            if (num % 2 == 1) num = (num + 1) / -2;
            else num /= 2;
            res.push_back(num);
            break;
        }
        ///uint32_t
        case 0x26:{
            uint32_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        ///0???
        case 0x27:{
            res.push_back(0);
            break;
        }
        ///unsigned RTON number???
        case 0x28:{
            res.push_back(unsigned_RTON_num2int(read_RTON_num()));
            break;
        }
        ///0.0???
        case 0x41:{
            res.push_back(0.0);
            break;
        }
        ///float64
        case 0x42:{
            double num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res.push_back(num);
            break;
        }
        ///unsigned RTON number???
        case 0x44:{
            res.push_back(unsigned_RTON_num2int(read_RTON_num()));
            break;
        }
        ///signed RTON number???
        case 0x45:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            if (num % 2 == 1) num = (num + 1) / -2;
            else num /= 2;
            res.push_back(num);
            break;
        }
        ///string
        case 0x81:{
            ///get buffer
            uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());

            ///read buffer
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;
            res.push_back(string(temp));
            break;
        }
        case 0x82:{
            ///get string buffer
            uint64_t s_buffer = unsigned_RTON_num2int(read_RTON_num());
            uint64_t s_buffer_check = unsigned_RTON_num2int(read_RTON_num());
            char * s;
            if (s_buffer == s_buffer_check){
                s = new char [s_buffer + 1];
                input.read(s, s_buffer);
                s[s_buffer] = 0;
                res.push_back(string(s));
            }
            else bytecode_error();
            break;
        }
        ///RTID
        case 0x83:{
            uint8_t check;
            input.read(reinterpret_cast <char *> (&check), sizeof check);
            ///check for 03
            if (check == 0x3){
                ///get 1st string
                uint64_t s1_buffer = unsigned_RTON_num2int(read_RTON_num());
                uint64_t s1_buffer_check = unsigned_RTON_num2int(read_RTON_num());
                char * s1;
                if (s1_buffer == s1_buffer_check){
                    s1 = new char [s1_buffer + 1];
                    input.read(s1, s1_buffer);
                    s1[s1_buffer] = 0;
                }
                else bytecode_error();
                ///get 2nd string
                uint64_t s2_buffer = unsigned_RTON_num2int(read_RTON_num());
                uint64_t s2_buffer_check = unsigned_RTON_num2int(read_RTON_num());
                char * s2;
                if (s2_buffer == s2_buffer_check){
                    s2 = new char [s2_buffer + 1];
                    input.read(s2, s2_buffer);
                    s2[s2_buffer] = 0;
                }
                else bytecode_error();
                res.push_back(string() + "RTID(" + s2 + '@' + s1 + ')');
            }
            else bytecode_error();
            break;
        }
        ///null
        case 0x84:{
            res.push_back(nullptr);
            break;
        }
        ///sub-object
        case 0x85:{
            res.push_back(json_decode());
            break;
        }
        ///array
        case 0x86:{
            ///check for fd
            uint8_t check;
            input.read(reinterpret_cast <char *> (&check), sizeof check);
            if (check == 0xfd){
                size_t num_elements = unsigned_RTON_num2int(read_RTON_num());
                json arr = json::array();
                for (int i = 0; i < num_elements; ++i) arr.push_back(read_block_RTON()[0]);
                res.push_back(arr);
                ///check end of array
                uint8_t check_end;
                input.read(reinterpret_cast <char *> (&check_end), sizeof check_end);
                if (check_end != 0xfe) bytecode_error();
            }
            else bytecode_error();
            break;
        }
        ///substitute
        case 0x90:{
            ///get buffer
            uint64_t buffer = unsigned_RTON_num2int(read_RTON_num());

            ///read buffer
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;

            ///logging
            debug_js["0x91"].push_back(json::object({
                {to_hex_string(int2unsigned_RTON_num(prev_stack.size())), string(temp)}
            }));

            ///push to prev_stack and write json
            prev_stack.push_back(temp);
            res.push_back(prev_stack[prev_stack.size() - 1]);
            break;
        }
        case 0x91:{
            ///substitute
            res.push_back(prev_stack[unsigned_RTON_num2int(read_RTON_num())]);
            break;
        }
        ///end of object
        case 0xFF:{
            break;
        }
        ///else just exit error
        default: bytecode_error();
    }
    return res;
}

json json_decode(){
    json res;
    while(true){
        string key;
        json js_key = read_block_RTON();
        if (js_key.size() == 0) return res;
        else{
            if (!js_key[0].is_string()) bytecode_error();
            key = js_key[0];
        }
        json value = read_block_RTON()[0];
        res[key] = value;
    }
}

void bytecode_error(){
    uint8_t bytecode;
    input.seekg(input.tellg() - 1);
    input.read(reinterpret_cast <char *> (&bytecode), sizeof bytecode);
    cout << "\nERROR READING BYTECODE " << hex << showbase << (int)bytecode << " AT " << input.tellg() - 1 << "!!!\n";
    debug << setw(4) << debug_js;
    exit(1);
}
