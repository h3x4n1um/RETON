#include <climits>
#include <cmath>
#include <cstdint>
#include <vector>

constexpr double log256(double q){
    return log2(q) / 8;
}

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

uint64_t unsigned_RTON_num2int(std::vector <uint8_t> q){
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
