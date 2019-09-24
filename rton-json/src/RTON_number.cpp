#include <climits>
#include <cmath>
#include <cstdint>

#include "include/RTON_number.hpp"

constexpr double log256(double q){
    return log2(q) / 8;
}

vector <uint8_t> int2unsigned_RTON_num(uint64_t q){
    vector <uint8_t> res;
    if (q <= 0x7f){
        res.push_back(q);
        return res;
    }
    uint8_t temp = q % 0x100;
    q = q / 0x100 * 2;
    if (temp > 0x7f) ++q;
    else temp += 0x80; //reverse & 0x7f
    res = int2unsigned_RTON_num(q);
    res.insert(res.begin(), temp);
    return res;
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
