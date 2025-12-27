#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>

class Database {
private:
    std::map<std::string, std::string> data;
    std::string filename;

public:
    Database(const std::string& filename);
    void save();
    void printAll();
    void insert(const std::string& key, const std::string& value);
    void encrypted_insert(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    std::string dget(const std::string& key);
    bool update(const std::string& key, const std::string& value);
    bool remove(const std::string& key);
    void clear();
    std::string getAllData(); // New method to retrieve all key-value pairs
    std::string hash(); // New method to hash the database contents
    void clearDisplay();
};

#endif // DATABASE_H
