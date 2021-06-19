#pragma once

#include "include/fifo_json.hpp"

reton::fifo_json decode_RTON(const std::vector <uint8_t> &byte_array, std::size_t &pos, std::vector<std::string> &array_0x91, std::vector<std::string> &array_0x93, reton::fifo_json &rton_info);
reton::fifo_json rton2json(const std::vector <uint8_t> &byte_array, reton::fifo_json &rton_info);
