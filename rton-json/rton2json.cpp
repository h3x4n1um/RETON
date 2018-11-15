#include <iostream>
#include <fstream>
#include "json.hpp"

using namespace std;

constexpr double log256(double x){
    return log2(x) / 8;
}

extern ifstream input;
extern ofstream debug;
vector <string> prev_stack;

void byte_code_error(int pos);

uint64_t RTON_num2int(vector <uint8_t> q){
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
void push_array_mode(nlohmann::json &js, string &key, const T val, const bool array_mode){
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

nlohmann::json json_decode(){
    ///init
    nlohmann::json res;
    uint8_t byte_code;
    string key;
    bool array_mode = false;

    while(true){
        debug << hex << showbase << input.tellg() << endl;
        ///split case
        input.read(reinterpret_cast <char *> (&byte_code), sizeof byte_code);
        switch (byte_code){
            ///false
            case 0x0:{
                push_array_mode(res, key, false, array_mode);
                break;
            }
            ///true
            case 0x1:{
                push_array_mode(res, key, true, array_mode);
                break;
            }
            ///uint8_t
            case 0xa:{
                uint8_t num;
                input.read(reinterpret_cast <char *> (&num), sizeof num);
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///0???
            case 0xb:{
                push_array_mode(res, key, 0, array_mode);
                break;
            }
            ///int16_t
            case 0x10:{
                int16_t num;
                input.read(reinterpret_cast <char *> (&num), sizeof num);
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///0???
            case 0x11:{
                push_array_mode(res, key, 0, array_mode);
                break;
            }
            ///uint16_t
            case 0x12:{
                uint16_t num;
                input.read(reinterpret_cast <char *> (&num), sizeof num);
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///int32_t
            case 0x20:{
                int32_t num;
                input.read(reinterpret_cast <char *> (&num), sizeof num);
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///0???
            case 0x21:{
                push_array_mode(res, key, 0, array_mode);
                break;
            }
            ///float32
            case 0x22:{
                float num;
                input.read(reinterpret_cast <char *> (&num), sizeof num);
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///0.0???
            case 0x23:{
                push_array_mode(res, key, 0.0, array_mode);
                break;
            }
            ///RTON number
            case 0x24:{
                uint64_t num = RTON_num2int(read_RTON_num());
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///signed RTON number
            case 0x25:{
                int64_t num = RTON_num2int(read_RTON_num());
                if (num % 2 == 0) num /= -2;
                else num /= 2;
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///uint32_t
            case 0x26:{
                uint32_t num;
                input.read(reinterpret_cast <char *> (&num), sizeof num);
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///0???
            case 0x27:{
                push_array_mode(res, key, 0, array_mode);
                break;
            }
            ///RTON number???
            case 0x28:{
                int64_t num = RTON_num2int(read_RTON_num());
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///0.0???
            case 0x41:{
                push_array_mode(res, key, 0.0, array_mode);
                break;
            }
            ///float64
            case 0x42:{
                double num;
                input.read(reinterpret_cast <char *> (&num), sizeof num);
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///RTON number???
            case 0x44:{
                int64_t num = RTON_num2int(read_RTON_num());
                push_array_mode(res, key, num, array_mode);
                break;
            }
            ///signed RTON number???
            case 0x45:{
                int64_t num = RTON_num2int(read_RTON_num());
                if (num % 2 == 0) num /= -2;
                else num /= 2;
                push_array_mode(res, key, num, array_mode);
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
                push_array_mode(res, key, string(temp), array_mode);
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
                    push_array_mode(res, key, string(s), array_mode);
                }
                else byte_code_error(input.tellg());
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
                    else byte_code_error(input.tellg());
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
                    else byte_code_error(input.tellg());
                    push_array_mode(res, key, string() + + "RTID(" + s2 + '@' + s1 + ')', array_mode);
                }
                else byte_code_error(input.tellg());
                break;
            }
            ///null
            case 0x84:{
                push_array_mode(res, key, nullptr, array_mode);
                break;
            }
            ///sub-object
            case 0x85:{
                push_array_mode(res, key, json_decode(), array_mode);
                break;
            }
            ///array
            case 0x86:{
                ///check for fd
                uint8_t check;
                input.read(reinterpret_cast <char *> (&check), sizeof check);
                if (check == 0xfd){
                    RTON_num2int(read_RTON_num());
                    array_mode = true;
                    res[key] = nlohmann::json::array();
                }
                else byte_code_error(input.tellg());
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
                debug << "prev_stack " << showbase << prev_stack.size() << ":\t" << temp << endl;
                prev_stack.push_back(temp);
                if (key.size() > 0) push_array_mode(res, key, prev_stack[prev_stack.size() - 1], array_mode);
                else key = temp;
                break;
            }
            case 0x91:{
                ///get pos
                uint64_t pos = RTON_num2int(read_RTON_num());

                ///substitute
                if (key.size() > 0) push_array_mode(res, key, prev_stack[pos], array_mode);
                else key = prev_stack[pos];
                break;
            }
            ///end of array
            case 0xFE:{
                array_mode = false;
                key.clear();
                break;
            }
            ///end of object
            case 0xFF:{
                return res;
                break;
            }
            ///else just return error
            default: byte_code_error(input.tellg());
        }
    }
}

void byte_code_error(int pos){
    uint8_t byte_code;
    input.seekg(input.tellg() - 1);
    input.read(reinterpret_cast <char *> (&byte_code), sizeof byte_code);
    cout << "\nERROR READING BYTE CODE " << hex << showbase << (int)byte_code << " AT " << input.tellg() - 1 << "!!!\n";
    exit(1);
}
