#include "include/error.hpp"

#include "include/rton-json.hpp"

std::string chunk_error(const std::size_t &pos, const uint8_t &chunk_type){
    return "Error reading chunk type " + to_hex_string(chunk_type) + " at " + to_hex_string(pos-1) + "!!!";
}
