#pragma once

#include <vector>

using namespace std;

constexpr double log256(double q);
vector <uint8_t> uint64_to_uRTON(uint64_t q);
uint64_t uRTON_to_uint64(vector <uint8_t> q);
