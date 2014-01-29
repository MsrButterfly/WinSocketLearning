#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <cstring>
#include <windows.h>

#pragma comment(lib, "Ws2_32.lib")

using Socket = SOCKET;
using SocketAddress = SOCKADDR;
using SocketAddressIn = SOCKADDR_IN;
using Word = WORD;
inline Word makeWord(const int &a, const int &b) {
    return MAKEWORD(a, b);
}
const uint64_t ServerIP = 0xC0A80106; // or const char *ServerIP = 192.168.1.6, with inet_addr(ServerIP)
const uint32_t ServerPort = 10000;
const uint32_t BackLog = 10;
const uint16_t MaxDataLength = MAX_PATH;

int main(int argc, const char *argv[]) {
    WSAData wsaData;
    const auto versionRequested = makeWord(2, 2);
    if (WSAStartup(versionRequested, &wsaData) != 0) {
        std::cerr << "Can not initialse windows socket." << std::endl;
        return EXIT_FAILURE;
    }
    Socket serverSocket;
    if ((serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        std::cerr << "Can not create server socket." << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }
    SocketAddressIn serverAddress;
    serverAddress.sin_addr.s_addr = htonl(ServerIP);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(ServerPort);
    if (bind(serverSocket, (SocketAddress *) &serverAddress, sizeof(SocketAddress)) == SOCKET_ERROR) {
        std::cerr << "Can not bind socket with specified address." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }
    if (listen(serverSocket, BackLog) == SOCKET_ERROR) {
        std::cerr << "Can not listen on specified socket now." << std::endl;
        closesocket(serverSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }
    std::cout << "Server successfully launched." << std::endl;
    Socket clientSocket;
    SocketAddressIn clientAddress;
    int length = sizeof(clientAddress);
    std::vector<std::unique_ptr<std::thread>> threads;
    while (true) {
        if ((clientSocket = accept(serverSocket, (SocketAddress *) &clientAddress, &length)) == INVALID_SOCKET) {
            std::cerr << "Can not accept data from specified socket now." << std::endl;
            return EXIT_FAILURE;
        }
        const uint16_t net = clientAddress.sin_addr.s_net;
        const uint16_t host = clientAddress.sin_addr.s_host;
        const uint16_t logicalHost = clientAddress.sin_addr.s_lh;
        const uint16_t imp = clientAddress.sin_addr.s_impno;
        std::cout << "Connected to " << net << '.' << host << '.' << logicalHost << '.' << imp << '.' << std::endl;
        threads.emplace_back(new std::thread([clientSocket, net, host, logicalHost, imp, &serverSocket, &threads]() {
            char data[MaxDataLength];
            while (true) {
                memset(data, NULL, sizeof(char) * MaxDataLength);
                int flag = recv(clientSocket, data, sizeof(char) * MaxDataLength, NULL);
                if (flag == 0 || flag == SOCKET_ERROR) {
                    std::cout << "Disconnected from client " << net << '.' << host << '.' << logicalHost << '.' << imp << '.' << std::endl;
                    closesocket(clientSocket);
                    return;
                }
                if (strcmp(data, "shutdown") == 0) {
                    closesocket(serverSocket);
                    for (const auto &thread : threads) {
                        thread->detach();
                    }
                    WSACleanup();
                    exit(EXIT_SUCCESS);
                }
                std::cout << "[" << net << '.' << host << '.' << logicalHost << '.' << imp << "] " << data << std::endl;
            }
        }));
    }
    return EXIT_SUCCESS;
}
