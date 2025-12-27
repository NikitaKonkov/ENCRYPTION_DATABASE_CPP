#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <winsock2.h>
#include <ws2tcpip.h>
#include <string>
#include <functional>

#pragma comment(lib, "ws2_32.lib")

class SocketServer {
private:
    SOCKET serverSocket;
    SOCKET clientSocket;
    struct sockaddr_in serverAddr;
    int port;
    bool isRunning;

public:
    SocketServer(int port);
    ~SocketServer();
    
    bool initialize();
    bool startListening();
    std::string receiveData();
    void sendResponse(const std::string& response);
    void stop();
    void clearScreen();
    // Callback for handling received data
    std::function<void(const std::string&)> onDataReceived;
};

#endif // SOCKET_SERVER_H
