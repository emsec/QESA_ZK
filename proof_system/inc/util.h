#pragma once

#include "types.h"

#define UNUSED(x) (void)(x);

#include <string>
#include <vector>

std::string ltrim(std::string str, const std::string& chars = "\t\n\v\f\r ");

std::string rtrim(std::string str, const std::string& chars = "\t\n\v\f\r ");

std::string trim(std::string str, const std::string& chars = "\t\n\v\f\r ");

std::vector<std::string> split(const std::string& s, char delimiter);

u32 round_to_next_power_of_two(u32 x);
