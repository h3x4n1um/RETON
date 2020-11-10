#pragma once

#include <iomanip>
#include <sstream>
#include <string>
#include <vector>

#include "include/json_fifo.hpp"

#if INTPTR_MAX == INT64_MAX
// 64-bit
#  define ARCHITECTURE "64-bit"
#elif INTPTR_MAX == INT32_MAX
// 32-bit
#  define ARCHITECTURE "32-bit"
#else
#  define ARCHITECTURE "Unknown"
#endif

const std::string architecture = ARCHITECTURE;
const std::string ver = "3.0.0";

std::string to_hex_string(const uint64_t &q);
std::string to_hex_string(const std::vector <uint8_t> &a);
