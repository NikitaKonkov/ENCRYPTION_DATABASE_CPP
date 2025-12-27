#include "SocketServer.h"
#include <iostream>

SocketServer::SocketServer(int port) : port(port), isRunning(false), 
    serverSocket(INVALID_SOCKET), clientSocket(INVALID_SOCKET) {
}

SocketServer::~SocketServer() {
    stop();
}

bool SocketServer::initialize() {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "WSAStartup failed: " << result << "\n";
        return false;
    }

    serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET) {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return false;
    }

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        std::cerr << "Bind failed: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed: " << WSAGetLastError() << "\n";
        closesocket(serverSocket);
        WSACleanup();
        return false;
    }

    isRunning = true;
    std::cout << "Server initialized and listening on port " << port << "\n";
    return true;
}

bool SocketServer::startListening() {
    if (!isRunning) {
        std::cerr << "Server not initialized\n";
        return false;
    }

    std::cout << "Waiting for client connection...\n";
    
    clientSocket = accept(serverSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << "Accept failed: " << WSAGetLastError() << "\n";
        return false;
    }

    std::cout << "Client connected!\n";
    return true;
}

std::string SocketServer::receiveData() {
    const int bufferSize = 4096;
    char buffer[bufferSize];
    
    int bytesReceived = recv(clientSocket, buffer, bufferSize - 1, 0);
    
    if (bytesReceived > 0) {
        buffer[bytesReceived] = '\0';
        return std::string(buffer);
    } else if (bytesReceived == 0) {
        std::cout << "Connection closed by client\n";
        return "";
    } else {
        std::cerr << "recv failed: " << WSAGetLastError() << "\n";
        return "";
    }
}

void SocketServer::sendResponse(const std::string& response) {
    if (clientSocket != INVALID_SOCKET) {
        send(clientSocket, response.c_str(), static_cast<int>(response.length()), 0);
    }
}

void SocketServer::stop() {
    isRunning = false;
    
    if (clientSocket != INVALID_SOCKET) {
        closesocket(clientSocket);
        clientSocket = INVALID_SOCKET;
    }
    
    if (serverSocket != INVALID_SOCKET) {
        closesocket(serverSocket);
        serverSocket = INVALID_SOCKET;
    }
    
    WSACleanup();
    std::cout << "Server stopped" << "\n";
}

void SocketServer::clearScreen(){
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif

    std::cout << "Console cleared by server command\n";
}