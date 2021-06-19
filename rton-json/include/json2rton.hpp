#pragma once

#include "include/fifo_json.hpp"

std::vector <uint8_t> encode_JSON(const reton::fifo_json &js, std::size_t& pos, std::unordered_map <std::string, uint64_t> &map_0x91, std::unordered_map <std::string, uint64_t> &map_0x93, reton::fifo_json &rton_info);
std::vector <uint8_t> json2rton(const reton::fifo_json &js, reton::fifo_json &rton_info);
