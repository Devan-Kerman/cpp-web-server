//
// Created by devse on 8/29/2022.
//
#define WIN32_LEAN_AND_MEAN


#include <Windows.h>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <format>
#include <iostream>
#include <sstream>
#include <fstream>
#include <filesystem>

#include "http.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

std::string readFile(const std::string &fileName) {
    std::ifstream file(fileName);
    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    return content;
}

void sendResponse(SOCKET &socket, const std::string &contentType, const std::string &content) {
    std::stringstream outS;

    http::write(http::response {
        .version = "HTTP/1.1",
        .code = "200",
        .message = "OK",
        .headers = {{"Content-Type", {contentType}}},
        .body = content,
    }, outS);

    std::string out = outS.str();
    send(socket, out.c_str(), out.size(), 0);
}

int __cdecl main(int argc, char** argv) {
    std::cout << "Working directory: " << std::filesystem::current_path();

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
        printf("socket failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(socketId);
        WSACleanup();
        return 1;
    }

    int socketSetupErrorCode = bind(serverSocket, socketId->ai_addr, (int) socketId->ai_addrlen);
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
    while (true) {
        SOCKET clientSocket = accept(serverSocket, NULL, NULL);
        if (clientSocket == INVALID_SOCKET) {
            printf("accept failed with error: %d\n", WSAGetLastError());
            closesocket(serverSocket);
            WSACleanup();
            return 1;
        }

        std::string response;
        int read;
        do {
            read = recv(clientSocket, buf, bufLen, 0);
            std::string responseTemp(buf, 0, read);
            response += responseTemp;
        } while (read == bufLen);

        int ln = response.find('\n'), get = response.find("GET"), end = response.find("HTTP");
        if (ln != std::string::npos && get != std::string::npos && end != std::string::npos && get < ln && end < ln && end > get) {
            std::string query = response.substr(get+4, (end-1) - (get+4));
            if(query == "/info.txt") {
                std::string content = std::to_string(std::rand() % 100000);
                sendResponse(clientSocket, "text/txt", content);
            } else if(query == "/") {
                std::string fileName = "../resources/index.html";
                std::string content = readFile(fileName);
                sendResponse(clientSocket, "text/html", content);
            } else {
                std::fstream fstream("../resources/"+query);
                if(fstream) {
                    std::stringstream buffer;
                    buffer << fstream.rdbuf();
                    std::string content = buffer.str();
                    std::string contentType;

                    if(query.ends_with(".png")) {
                        contentType = "image/png";
                    } else {
                        contentType = "text/html";
                    }
                    sendResponse(clientSocket, contentType, content);
                } else {
                    std::string fileName = "../resources/404.html";
                    std::string content = readFile(fileName);
                    sendResponse(clientSocket, "text/html", content);
                }

            }
        } else {
            std::string fileName = "../resources/404.html";
            std::string content = readFile(fileName);
            sendResponse(clientSocket, "text/html", content);
        }
        closesocket(clientSocket);
    }

    WSACleanup();
    return 0;
}
