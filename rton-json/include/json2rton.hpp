#pragma once

#include <string>
#include <vector>

using namespace std;

int rton_encode();
int write_RTON(json js);
int write_RTON_block(json js);
int write_uRTON_t(vector <uint8_t> a);

size_t get_utf8_size(string q);
