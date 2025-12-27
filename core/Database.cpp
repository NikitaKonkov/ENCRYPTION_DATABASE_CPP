#include "Database.h"
#include "Datahash.h"
#include <iostream>
#include <fstream>
#include <string>
#include <unordered_map>
#include <sstream>
#include "Dataencryption.h"

Database::Database(const std::string& filename) : filename(filename) {
    // Load data from file if it exists
    std::ifstream file(filename);
    if (file.is_open()) {
        std::string key, value;
        while (file >> key >> value) {
            data[key] = value;
        }
        file.close();
    }
}

void Database::save() {
    std::ofstream file(filename, std::ios::trunc);
    for (const auto& pair : data) {
        file << pair.first << " " << pair.second << "\n";
    }
    file.close();
}

void Database::printAll() {
    for (const auto& pair : data) {
        std::cout << pair.first << " : " << pair.second << "\n";
    }
}

void Database::insert(const std::string& key, const std::string& value) {
    data[key] = value;
}

void Database::encrypted_insert(const std::string& key, const std::string& value){
    data[key] = value;
}

void Database::clearDisplay() {
    // Platform-specific clear screen
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
}

std::string Database::get(const std::string& key) {
    auto it = data.find(key);
    if (it != data.end()) {
        return it->second;
    }
    return "";
}

bool Database::update(const std::string& key, const std::string& value) {
    auto it = data.find(key);
    if (it != data.end()) {
        it->second = value;
        return true;
    }
    return false;
}

bool Database::remove(const std::string& key) {
    return data.erase(key) > 0;
}

void Database::clear() {
    data.clear();
}

std::string Database::hash() {
    std::ostringstream oss;
    for (const auto& pair : data) {
        oss << pair.first << pair.second;
    }
    std::string allData = oss.str();
    unsigned long long hashValue = fastHash(const_cast<char*>(allData.c_str()));
    std::cout << "Database hash: " << hashValue << std::endl;
    return "Database hash: " + std::to_string(hashValue) + "\n";
}

std::string Database::getAllData() {
    std::ostringstream oss;
    for (const auto& pair : data) {
        oss << pair.first << " : " << pair.second << "\n";
    }
    return oss.str();
}
