#pragma once

#include <string>
#include <vector>

#include "include/json_fifo.hpp"

const std::string architecture = std::to_string(uint16_t(round(log2(UINTPTR_MAX)))) + "-bit";
const std::string ver = "3.1.0";

std::string to_hex_string(const uint64_t &q);
std::string to_hex_string(const std::vector <uint8_t> &a);
