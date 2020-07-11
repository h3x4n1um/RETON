#pragma once

#include <string>

const std::string json_error = "Error! This file is a JSON but format is not supported";

std::string chunk_error(const std::size_t &pos, const uint8_t &chunk_type);
