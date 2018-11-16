#include <iostream>
#include <iomanip>
#include <fstream>

#include "json.hpp"
#include "fifo_map.hpp"

using namespace std;
/// A workaround to give to use fifo_map as map, we are just ignoring the 'less' compare
/// https://github.com/nlohmann/json/issues/485
template<class K, class V, class dummy_compare, class A>
using workaround_fifo_map = nlohmann::fifo_map<K, V, nlohmann::fifo_map_compare<K>, A>;
using json = nlohmann::basic_json<workaround_fifo_map>;

extern ifstream input;
extern ofstream debug;
vector <string> prev_stack;

json json_decode();
void byte_code_error();

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

template <class T>
void push_array_mode(json &js, string &key, const T val, const bool array_mode){
    if (!array_mode){
        js[key] = val;
        key.clear();
    }
    else js[key].push_back(val);
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

json read_block_RTON(){
    uint8_t byte_code;
    json res;
    debug << hex << showbase << input.tellg() << endl;
    ///split case
    input.read(reinterpret_cast <char *> (&byte_code), sizeof byte_code);
    switch (byte_code){
        ///false
        case 0x0:{
            res["read_block_RTON"] = false;
            break;
        }
        ///true
        case 0x1:{
            res["read_block_RTON"] = true;
            break;
        }
        ///uint8_t
        case 0xa:{
            uint8_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res["read_block_RTON"] = num;
            break;
        }
        ///0???
        case 0xb:{
            res["read_block_RTON"] = 0;
            break;
        }
        ///int16_t
        case 0x10:{
            int16_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res["read_block_RTON"] = num;
            break;
        }
        ///0???
        case 0x11:{
            res["read_block_RTON"] = 0;
            break;
        }
        ///uint16_t
        case 0x12:{
            uint16_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res["read_block_RTON"] = num;
            break;
        }
        ///int32_t
        case 0x20:{
            int32_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res["read_block_RTON"] = num;
            break;
        }
        ///0???
        case 0x21:{
            res["read_block_RTON"] = 0;
            break;
        }
        ///float32
        case 0x22:{
            float num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res["read_block_RTON"] = num;
            break;
        }
        ///0.0???
        case 0x23:{
            res["read_block_RTON"] = 0.0;
            break;
        }
        ///unsigned RTON number
        case 0x24:{
            uint64_t num = unsigned_RTON_num2int(read_RTON_num());
            res["read_block_RTON"] = num;
            break;
        }
        ///signed RTON number
        case 0x25:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            if (num % 2 == 1) num = (num + 1) / -2;
            else num /= 2;
            res["read_block_RTON"] = num;
            break;
        }
        ///uint32_t
        case 0x26:{
            uint32_t num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res["read_block_RTON"] = num;
            break;
        }
        ///0???
        case 0x27:{
            res["read_block_RTON"] = 0;
            break;
        }
        ///unsigned RTON number???
        case 0x28:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            res["read_block_RTON"] = num;
            break;
        }
        ///0.0???
        case 0x41:{
            res["read_block_RTON"] = 0.0;
            break;
        }
        ///float64
        case 0x42:{
            double num;
            input.read(reinterpret_cast <char *> (&num), sizeof num);
            res["read_block_RTON"] = num;
            break;
        }
        ///unsigned RTON number???
        case 0x44:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            res["read_block_RTON"] = num;
            break;
        }
        ///signed RTON number???
        case 0x45:{
            int64_t num = unsigned_RTON_num2int(read_RTON_num());
            if (num % 2 == 1) num = (num + 1) / -2;
            else num /= 2;
            res["read_block_RTON"] = num;
            break;
        }
        ///string
        case 0x81:{
            ///get buffer
            uint8_t buffer;
            input.read(reinterpret_cast <char *> (&buffer), sizeof buffer);

            ///read buffer
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;
            res["read_block_RTON"] = string(temp);
            break;
        }
        case 0x82:{
            ///get string buffer
            uint16_t s_buffer;
            char * s;
            input.read(reinterpret_cast <char * > (&s_buffer), sizeof s_buffer);
            if (s_buffer / 0x100 == s_buffer % 0x100){
                s_buffer /= 0x100;
                s = new char [s_buffer + 1];
                input.read(s, s_buffer);
                s[s_buffer] = 0;
                res["read_block_RTON"] = string(s);
            }
            else byte_code_error();
            break;
        }
        ///RTID
        case 0x83:{
            uint8_t check;
            input.read(reinterpret_cast <char *> (&check), sizeof check);
            ///check for 03
            if (check == 0x3){
                ///get 1st string
                uint16_t s1_buffer;
                char * s1;
                input.read(reinterpret_cast <char * > (&s1_buffer), sizeof s1_buffer);
                if (s1_buffer / 0x100 == s1_buffer % 0x100){
                    s1_buffer /= 0x100;
                    s1 = new char [s1_buffer + 1];
                    input.read(s1, s1_buffer);
                    s1[s1_buffer] = 0;
                }
                else byte_code_error();
                ///get 2nd string
                uint16_t s2_buffer;
                char * s2;
                input.read(reinterpret_cast <char * > (&s2_buffer), sizeof s2_buffer);
                if (s2_buffer / 0x100 == s2_buffer % 0x100){
                    s2_buffer /= 0x100;
                    s2 = new char [s2_buffer + 1];
                    input.read(s2, s2_buffer);
                    s2[s2_buffer] = 0;
                }
                else byte_code_error();
                res["read_block_RTON"] = string() + "RTID(" + s2 + '@' + s1 + ')';
            }
            else byte_code_error();
            break;
        }
        ///null
        case 0x84:{
            res["read_block_RTON"] = nullptr;
            break;
        }
        ///sub-object
        case 0x85:{
            res["read_block_RTON"] = json_decode();
            break;
        }
        ///array
        case 0x86:{
            ///check for fd
            uint8_t check;
            input.read(reinterpret_cast <char *> (&check), sizeof check);
            if (check == 0xfd){
                size_t num_elements = unsigned_RTON_num2int(read_RTON_num());
                res["read_block_RTON"] = json::array();
                for (int i = 0; i < num_elements; ++i) res["read_block_RTON"].push_back(*read_block_RTON().begin());
                uint8_t check_end;
                input.read(reinterpret_cast <char *> (&check_end), sizeof check_end);
                if (check_end != 0xfe) byte_code_error();
            }
            else byte_code_error();
            break;
        }
        ///substitute
        case 0x90:{
            ///get buffer
            uint8_t buffer;
            input.read(reinterpret_cast <char *> (&buffer), sizeof buffer);

            ///read buffer
            char temp[buffer + 1];
            input.read(temp, buffer);
            temp[buffer] = 0;

            ///push to prev_stack and write json
            debug << "prev_stack[" << showbase << prev_stack.size() << "] = " << temp << endl;
            prev_stack.push_back(temp);
            res["read_block_RTON"] = prev_stack[prev_stack.size() - 1];
            break;
        }
        case 0x91:{
            ///get pos
            uint64_t pos = unsigned_RTON_num2int(read_RTON_num());

            ///substitute
            res["read_block_RTON"] = prev_stack[pos];
            break;
        }
        ///end of object
        case 0xFF:{
            res["read_block_RTON"] = 0xff;
            break;
        }
        ///else just exit error
        default: byte_code_error();
    }
    return res;
}

json json_decode(){
    json res;
    while(true){
        string key;
        json js_key = read_block_RTON();
        if (js_key["read_block_RTON"].is_string() == false) return res;
        else key = js_key["read_block_RTON"].get<string>();
        json value = read_block_RTON();
        res[key] = *value.begin();
    }
}

void byte_code_error(){
    uint8_t byte_code;
    input.seekg(input.tellg() - 1);
    input.read(reinterpret_cast <char *> (&byte_code), sizeof byte_code);
    cout << "\nERROR READING BYTE CODE " << hex << showbase << (int)byte_code << " AT " << input.tellg() - 1 << "!!!\n";
    exit(1);
}
