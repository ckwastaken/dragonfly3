// Created by mizhan on 11/8/26.
#pragma once
#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <stdexcept>

namespace salt {
    std::string mask();
    std::string shake(int multiplier = 1);
};