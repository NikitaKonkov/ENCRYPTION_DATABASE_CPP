#include "Dataencryption.h"


std::string encode(std::string &input) {
    std::cout << "Test function in Dataencyrption.cpp: " << input << "\n";
    std::stringstream ss;
    for (int n = 0; n < input.length(); ++n) {
        ss << std::hex << (int)input[n];
    }
    return ss.str();
}