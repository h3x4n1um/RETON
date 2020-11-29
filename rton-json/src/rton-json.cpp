#include "include/rton-json.hpp"

#include <iomanip>
#include <sstream>

std::string to_hex_string(const uint64_t &q){
    std::stringstream ss;
    ss << "0x" << std::hex << q;
    return ss.str();
}

std::string to_hex_string(const std::vector <uint8_t> &a){
    std::stringstream ss;
    ss << "0x";
    for (uint8_t i : a){
        ss << std::setfill('0') << std::setw(2) << std::hex << (int) i;
    }
    return ss.str();
}
