#include <iostream>
#include <thread>
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
const uint16_t MaxDataLength = MAX_PATH;

int main(int argc, const char *argv[]) {
    WSAData wsaData;
    const auto versionRequested = makeWord(2, 2);
    if (WSAStartup(versionRequested, &wsaData) != 0) {
        std::cerr << "Can not initialse windows socket." << std::endl;
        return EXIT_FAILURE;
    }
    Socket clientSocket;
    if ((clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == INVALID_SOCKET) {
        std::cerr << "Can not create client socket." << std::endl;
        WSACleanup();
        return EXIT_FAILURE;
    }
    SocketAddressIn serverAddress;
    serverAddress.sin_addr.s_addr = htonl(ServerIP);
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(ServerPort);
    if (connect(clientSocket, (SocketAddress *) &serverAddress, sizeof(SocketAddress)) == SOCKET_ERROR) {
        std::cerr << "Can not connet to the server." << std::endl;
        closesocket(clientSocket);
        WSACleanup();
        return EXIT_FAILURE;
    }
    std::cout << "Successfully connected to the server." << std::endl;
    char data[MaxDataLength];
    while (true) {
        std::cin.getline(data, sizeof(char) * MaxDataLength);
        if (strcmp(data, "exit") == 0) {
            break;
        }
        if (send(clientSocket, data, strlen(data) + 1, 0) == SOCKET_ERROR) {
            std::cout << "Can not send data to the server now." << std::endl;
            closesocket(clientSocket);
            WSACleanup();
            return EXIT_FAILURE;
        }
    }
    closesocket(clientSocket);
    WSACleanup();
    return EXIT_SUCCESS;
}
