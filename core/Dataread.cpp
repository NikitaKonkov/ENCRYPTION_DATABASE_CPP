#include "Dataread.h"

void readFile(const std::string& fileName) {
    std::ifstream file(fileName);
    if (file.is_open()) {
        std::string line;
        while (std::getline(file, line)) {
            std::cout << line << std::endl;
        }
        file.close();
    } else {
        std::cerr << "Unable to open file: " << fileName << std::endl;
    }
}
