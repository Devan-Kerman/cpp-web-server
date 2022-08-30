//
// Created by devse on 8/29/2022.
//


using namespace std;

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <format>
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

int __cdecl main() {
    std::string content = "<html><body>Hello World!</body></html>";
    std::string formatted = std::vformat("HTTP/1.1 200 OK\nServer: Server\nContent-type: text/html\nContent-length: {}\n\n{}", std::make_format_args(content.size(), content));

    // Initialize Winsock
    WSADATA socketApiData;
    int socketApiError = WSAStartup(MAKEWORD(2, 2), &socketApiData);
    if (socketApiError != 0) {
        printf("WSAStartup failed with error: %d\n", socketApiError);
        return 1;
    }

    addrinfo connectionType;
    ZeroMemory(&connectionType, sizeof(connectionType));
    connectionType.ai_family = AF_UNSPEC;
    connectionType.ai_socktype = SOCK_STREAM;
    connectionType.ai_protocol = IPPROTO_TCP;

    addrinfo *socketId;
    // Resolve the serverSocket address and port
    int resolveErrorCode = getaddrinfo("localhost", "8080", &connectionType, &socketId);
    if (resolveErrorCode != 0) {
        printf("getaddrinfo failed with error: %d\n", resolveErrorCode);
        WSACleanup();
        return 1;
    }

    SOCKET serverSocket = socket(socketId->ai_family, socketId->ai_socktype, socketId->ai_protocol);
    if (serverSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(socketId);
        WSACleanup();
        return 1;
    }

    int socketSetupErrorCode = bind(serverSocket, socketId->ai_addr, (int)socketId->ai_addrlen);
    if (socketSetupErrorCode == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(socketId);
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(socketId);

    int openConnectionErrorCode = listen(serverSocket, SOMAXCONN);
    if (openConnectionErrorCode == SOCKET_ERROR) {
        printf("listen failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    char buf[512];
    int bufLen = 512;
    while(true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        int read;
        do {
            read = recv(clientSocket, buf, bufLen, 0);
            std::string response(buf, 0, read);
            std::cout << response;
        } while(read == bufLen);

        std::printf("formatted: %d", formatted.size());
        send(clientSocket, formatted.c_str(), formatted.size(), 0);
        closesocket(clientSocket);
    }

    WSACleanup();
    return 0;
}