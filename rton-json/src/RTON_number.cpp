#include "include/RTON_number.hpp"

std::vector <uint8_t> uint64_t2uRTON_t(uint64_t q){
    std::vector <uint8_t> res;
    while(q > 0){
        uint8_t temp = q % 0x100;
        q = q / 0x100 * 2;
        if (temp > 0x7f) ++q;
        else if (q > 0) temp += 0x80; //reverse & 0x7f
        res.push_back(temp);
    }
    if (res.empty()) res.push_back(0);
    return res;
}

uint64_t uRTON_t2uint64_t(std::vector <uint8_t> q){
    if (q.size() == 1 && q[0] > 0x7f) return UINT64_MAX; //return max when RTON number has 1 byte and > 0x7f
    uint64_t near_last_byte, last_byte = 0;
    for (; q.size() > 0; q.pop_back()){
        near_last_byte = q[q.size() - 1];
        if (last_byte % 2 == 0) near_last_byte &= 0x7f;
        last_byte /= 2;
        last_byte = near_last_byte + last_byte * 0x100;
    }
    return last_byte;
}
