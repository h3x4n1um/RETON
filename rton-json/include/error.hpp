#pragma once

int bytecode_error(uint8_t bytecode);
int key_error();
int out_of_range_error(uint8_t bytecode);
int eof_error(char footer[5]);
int not_supported_json();
