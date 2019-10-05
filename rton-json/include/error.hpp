#pragma once

int bytecode_error(uint8_t bytecode);
int eof_error(char footer[5]);
int key_error();
int not_supported_json();
int out_of_range_error(uint8_t bytecode);
