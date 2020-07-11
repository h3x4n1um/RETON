#pragma once

#include "include/json_fifo.hpp"

json_fifo::json decode_RTON(const std::vector <uint8_t> &byte_array, std::size_t &pos, std::vector<std::string> &array_0x91, std::vector<std::string> &array_0x93, json_fifo::json &rton_info);
json_fifo::json rton2json(const std::vector <uint8_t> &byte_array, json_fifo::json &rton_info);
