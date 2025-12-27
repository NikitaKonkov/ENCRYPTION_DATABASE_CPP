#include <iostream>
#include <string>
#include <sstream> // Add this include to fix the incomplete type error for std::ostringstream
#include <unordered_set>
#include "core/Database.h"
#include "core/SocketServer.h"
#include "core/Datahash.h"

// Parse command from client (format: COMMAND|key|value)
void processCommand(const std::string& command, Database& db, SocketServer& server) {
    // Trim whitespace and newlines from command
    std::string trimmedCmd = command;
    size_t end = trimmedCmd.find_last_not_of(" \t\r\n");
    if (end != std::string::npos) {
        trimmedCmd = trimmedCmd.substr(0, end + 1);
    }
    
    // Extract the command part (before the delimiter)
    size_t delimPos = trimmedCmd.find(':');
    std::string cmd = (delimPos != std::string::npos) ? trimmedCmd.substr(0, delimPos) : trimmedCmd;
    
    std::unordered_set<std::string> helpCommands = {"HELP", "help", "Help"};
    std::unordered_set<std::string> insertCommands = {"INSERT", "insert", "Insert", "put"};
    std::unordered_set<std::string> encryptedInsertCommands = {"SINSERT","sinsert","Sinsert","putsec"};
    std::unordered_set<std::string> getCommands = {"GET", "get", "Get", "fetch"};
    std::unordered_set<std::string> dgetCommands = {"DGET", "dget", "Dget"};
    std::unordered_set<std::string> updateCommands = {"UPDATE", "update", "Update"};
    std::unordered_set<std::string> deleteCommands = {"DELETE", "delete", "del"};
    std::unordered_set<std::string> listCommands = {"LIST", "list","ls"};
    std::unordered_set<std::string> clearCommands = {"CLEAR", "clear","Clear","clr"};
    std::unordered_set<std::string> hashCommands = {"HASH", "hash", "Hash"};
    std::unordered_set<std::string> displayClear = {"cls","CLS","clear","CLEAR"};

    if (helpCommands.find(cmd) != helpCommands.end()) {
        std::string helpMessage = "Commands:\n";
        helpMessage += "  INSERT|key|value - Insert a new record\n";
        helpMessage += "  SINSERT|key|value - Insert a new record with encryption\n";
        helpMessage += "  DGET|key         - Retrieve a decrypted value by key\n";
        helpMessage += "  GET|key          - Retrieve a value by key\n";
        helpMessage += "  UPDATE|key|value - Update an existing record\n";
        helpMessage += "  DELETE|key       - Delete a record\n";
        helpMessage += "  LIST             - List all records\n";
        helpMessage += "  CLEAR            - Clear the entire database\n";
        helpMessage += "  HELP             - Show this help message\n";
        helpMessage += "  HASH             - Show hash of database contents\n";
        helpMessage += "  CLS/CLEAR        - Clear the console display\n";
        server.sendResponse(helpMessage);
    } else if (insertCommands.find(cmd) != insertCommands.end()) {
        size_t firstDelim = trimmedCmd.find(':');
        size_t secondDelim = trimmedCmd.find('=', firstDelim + 1);
        if (firstDelim != std::string::npos && secondDelim != std::string::npos) {
            std::string key = trimmedCmd.substr(firstDelim + 1, secondDelim - firstDelim - 1);
            std::string value = trimmedCmd.substr(secondDelim + 1);
            db.insert(key, value);
            db.save();
            server.sendResponse("OK: Inserted successfully\n");
        } else {
            server.sendResponse("ERROR: Invalid INSERT command format\n");
        }
    } else if (getCommands.find(cmd) != getCommands.end()) {
        size_t firstDelim = trimmedCmd.find(':');
        if (firstDelim != std::string::npos) {
            std::string key = trimmedCmd.substr(firstDelim + 1);
            std::string value = db.get(key);
            if (!value.empty()) {
                server.sendResponse("OK: " + value + "\n");
            } else {
                server.sendResponse("ERROR: Key not found\n");
            }
        } else {
            server.sendResponse("ERROR: Invalid GET command format. Use: GET|key\n");
        }
    } else if (updateCommands.find(cmd) != updateCommands.end()) {
        size_t firstDelim = trimmedCmd.find(':');
        size_t secondDelim = trimmedCmd.find('=', firstDelim + 1);
        if (firstDelim != std::string::npos && secondDelim != std::string::npos) {
            std::string key = trimmedCmd.substr(firstDelim + 1, secondDelim - firstDelim - 1);
            std::string value = trimmedCmd.substr(secondDelim + 1);
            if (db.update(key, value)) {
                db.save();
                server.sendResponse("OK: Updated successfully\n");
            } else {
                server.sendResponse("ERROR: Key not found\n");
            }
        } else {
            server.sendResponse("ERROR: Invalid UPDATE command format\n");
        }
    } else if (deleteCommands.find(cmd) != deleteCommands.end()) {
        size_t firstDelim = trimmedCmd.find(':');
        if (firstDelim != std::string::npos) {
            std::string key = trimmedCmd.substr(firstDelim + 1);
            if (db.remove(key)) {
                db.save();
                server.sendResponse("OK: Deleted successfully\n");
            } else {
                server.sendResponse("ERROR: Key not found\n");
            }
        } else {
            server.sendResponse("ERROR: Invalid DELETE command format. Use: DELETE|key\n");
        }
    } else if (listCommands.find(cmd) != listCommands.end()) {
        server.sendResponse(db.getAllData());
    } else if (clearCommands.find(cmd) != clearCommands.end()) {
        db.clear();
        db.save();
        server.sendResponse("OK: Database cleared\n");
    } else if (hashCommands.find(cmd) != hashCommands.end()) {
        server.sendResponse(db.hash());
    } else if (encryptedInsertCommands.find(cmd) != encryptedInsertCommands.end()) {
        size_t firstDelim = trimmedCmd.find(':');
        size_t secondDelim = trimmedCmd.find('=', firstDelim + 1);
        if (firstDelim != std::string::npos && secondDelim != std::string::npos) {
            std::string key = trimmedCmd.substr(firstDelim + 1, secondDelim - firstDelim - 1);
            std::string value = trimmedCmd.substr(secondDelim + 1);
            db.encrypted_insert(key, value);
            db.save();
            server.sendResponse("OK: Encrypted insert successful\n");
        } else {
            server.sendResponse("ERROR: Invalid SINSERT command format\n");
        }
    } else if (dgetCommands.find(cmd) != dgetCommands.end()) {
        size_t firstDelim = trimmedCmd.find(':');
        if (firstDelim != std::string::npos) {
            std::string key = trimmedCmd.substr(firstDelim + 1);
            std::string value = db.dget(key);
            if (!value.empty()) {
                server.sendResponse("OK: " + value + "\n");
            } else {
                server.sendResponse("ERROR: Key not found\n");
            }
        } else {
            server.sendResponse("ERROR: Invalid DGET command format. Use: DGET|key\n");
        }
    } else if (displayClear.find(cmd) != displayClear.end()) {
        // Clear console display command
        db.clearDisplay();

        server.sendResponse("\033[2J\033[H");
        server.sendResponse("OK: Console cleared\n");
    } else {
        server.sendResponse("ERROR: Unknown command. Use HELP to see available commands.\n");
    }
}

int main() {
    const int PORT = 8080;
    const std::string DB_FILE = "database.txt";
    
    std::cout << "========================================\n";
    std::cout << "    Socket Database Server\n";
    std::cout << "========================================\n";
    std::cout << "Commands:\n";
    std::cout << "  INSERT|key|value - Insert new record\n";
    std::cout << "  SINSERT|key|value - Insert new record with encryption\n";
    std::cout << "  GET|key          - Get record by key\n";
    std::cout << "  DGET|key         - Get decrypted record by key\n";
    std::cout << "  UPDATE|key|value - Update existing record\n";
    std::cout << "  DELETE|key       - Delete record\n";
    std::cout << "  LIST             - List all records\n";
    std::cout << "  CLEAR            - Clear database\n";
    std::cout << "  HELP             - Show help message\n";
    std::cout << "  HASH             - Show hash of database contents\n";
    std::cout << "  CLS/CLEAR        - Clear console display\n";
    std::cout << "========================================\n";
    
    Database db(DB_FILE);
    SocketServer server(PORT);
    
    if (!server.initialize()) {
        std::cerr << "Failed to initialize server\n";
        return 1;
    }
    
    while (true) {
        if (server.startListening()) {
            // Handle multiple requests from same client
            while (true) {
                std::string data = server.receiveData();
                if (data.empty()) {
                    break; // Client disconnected
                }
                
                std::cout << "Received: " << data << "\n";
                processCommand(data, db, server);
            }
        }
        
        std::cout << "Waiting for new connection...\n";
    }
    
    return 0;
}


