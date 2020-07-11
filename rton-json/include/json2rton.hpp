#pragma once

#include "include/json_fifo.hpp"

std::vector <uint8_t> encode_JSON(const json_fifo::json &js, std::unordered_map <std::string, uint64_t> &map_0x91, std::unordered_map <std::string, uint64_t> &map_0x93, json_fifo::json &rton_info);
std::vector <uint8_t> json2rton(const json_fifo::json &js, json_fifo::json &rton_info);
