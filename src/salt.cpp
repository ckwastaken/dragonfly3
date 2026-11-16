#include <iostream>
#include <fstream>
#include <string>
#include <array>
#include <stdexcept>

// Created by mizhan on 11/8/26.
namespace salt {
    std::string mask() { return "Pixel 8 Pro"; };

    /* NOTE:                       /\/\/\/\/\/\
                                   | | | | | |
                "Pixel 8 Pro" is a placeholder, frequently replace this value with a random companies other
        flagship, but dont use the same brand twice in a row. this is an important part of the anti fingerprinting
    */

    std::string shake(int multiplier = 1) {

        std::string out = "";

        for (int i = 0; i < multiplier; ++i) {
            // 16 bytes = 128 bits = 32 base-16 characters
            std::array<unsigned char, 16> buffer{};

            std::ifstream urandom("/dev/urandom", std::ios::in | std::ios::binary);
            if (!urandom) {
                std::cout << "Failed to open /dev/urandom.";
                exit(1);
            }

            urandom.read(reinterpret_cast<char*>(buffer.data()), buffer.size());
            if (urandom.gcount() != static_cast<std::streamsize>(buffer.size())) {
                std::cout << "Failed to read 16 bytes from /dev/urandom.";
                exit(1);
            }

            std::string ext_id;
            ext_id.reserve(32);

            for (unsigned char byte : buffer) {
                // High nibble (bits 4-7) mapped to 'a'-'p'
                ext_id.push_back(static_cast<char>('a' + ((byte >> 4) & 0x0F)));
                // Low nibble (bits 0-3) mapped to 'a'-'p'
                ext_id.push_back(static_cast<char>('a' + (byte & 0x0F)));
            }

            out.append(ext_id);
        }
        return out;
    }
};