#include "Dataencryption.h"


std::string encode(std::string input) {
    std::cout << "Test function in Dataencyrption.cpp: " << input << "\n";
    std::stringstream ss;
    for (int n = 0; n < input.length(); ++n) {
        ss << std::hex << (int)input[n];
    }
    return ss.str();
}


std::string decode(std::string input) {
    std::string output;
    for (size_t i = 0; i < input.length(); i += 2) {
        std::string byteString = input.substr(i, 2);
        char byte = (char) strtol(byteString.c_str(), nullptr, 16);
        output.push_back(byte);
    }
    return output;
}