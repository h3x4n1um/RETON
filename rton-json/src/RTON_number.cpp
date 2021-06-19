#include "include/RTON_number.hpp"

std::vector <uint8_t> uint64_t2uRTON_t(uint64_t q){
    std::vector <uint8_t> res;

    while(q){
        uint8_t temp = q%0x100;
        q = q / 0x100 * 2;
        if (temp > 0x7f) ++q;
        else if (q > 0) temp += 0x80; //reverse & 0x7f
        res.push_back(temp);
    }
    if (res.empty()) res.push_back(0);

    return res;
}

uint64_t uRTON_t2uint64_t(std::vector <uint8_t> q){
    if (q.size() == 1 && q.back() > 0x7f) return UINT64_MAX; //return max when RTON number has 1 byte and > 0x7f

    uint64_t res = 0;
    while(q.size()){
        uint64_t last_byte = q.back();
        q.pop_back();

        if (res%2 == 0) last_byte &= 0x7f;
        res /= 2;
        res = res*0x100+last_byte;
    }

    return res;
}
