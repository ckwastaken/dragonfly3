// Created by mizhan on 11/12/26.
#pragma once
#include <cstdlib>
#include <string>

namespace configs {
    inline std::string built_by = std::getenv("USER");
    inline std::string galleryat = "https://sprigatito.fanclub.rocks/";

    inline int major = 3;
    inline int minor = 1;
    inline int patch = 12;

    inline std::string for_board = "nuwa"; // Xiaomi 13 Pro
    inline std::string make = "Xiaomi";
    inline std::string model = "13 Pro";
}