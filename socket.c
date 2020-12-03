#include "socket.h"

#include <stdio.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define SUCCESS ANSI_COLOR_GREEN " success" ANSI_COLOR_RESET "\n"
#define ERROR_MESSAGE(string) ANSI_COLOR_RED string ANSI_COLOR_RESET

static WSADATA wsaData;

void printLastWSAError(const char *prefix)
{
    const int errorCode = WSAGetLastError();
    wchar_t *message = NULL;

    // Taken from https://stackoverflow.com/questions/3400922/how-do-i-retrieve-an-error-string-from-wsagetlasterror.
    FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                   NULL,
                   errorCode,
                   MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                   (LPWSTR)&message,
                   0,
                   NULL);

    fflush(stdout);
    fprintf(stderr, ERROR_MESSAGE(" %s failed with error code %d: %S"), prefix, errorCode, message);
    LocalFree(message);
    fflush(stderr);
}

void printWSAErrorCleanupAndExit(const char *prefix)
{
    printLastWSAError(prefix);
    cleanupWinsock();
    exit(EXIT_FAILURE);
}

void initializeWinsock()
{
    printf("\nInitialising Winsock...");

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printWSAErrorCleanupAndExit("WSAStartup()");
    }

    printf(SUCCESS);
}

void cleanupWinsock()
{
    printf("Cleaning up Winsock...");

    if (WSACleanup() != 0)
    {
        printLastWSAError("WSACleanup()");
        exit(EXIT_FAILURE);
    }

    printf(SUCCESS);
}

SOCKET createSocket()
{
    printf("Creating server socket...");

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocket == INVALID_SOCKET)
    {
        printWSAErrorCleanupAndExit("socket()");
    }

    printf(SUCCESS);

    return serverSocket;
}

void shutdownSocket(SOCKET socket, int mode)
{
    if (socket != INVALID_SOCKET)
    {
        printf("Shutting down socket...");

        if (shutdown(socket, mode) == SOCKET_ERROR)
        {
            printLastWSAError("shutdown()");
            closeSocket(socket);
        }
        else
        {
            printf(SUCCESS);
        }
    }
}

void closeSocket(const SOCKET socket)
{
    if (socket != INVALID_SOCKET)
    {
        printf("Closing socket...");

        if (closesocket(socket) != 0)
        {
            printLastWSAError("closesocket()");
        }
        else
        {
            printf(SUCCESS);
        }
    }
}

struct sockaddr_in createSocketAddress(const char *ip, const u_short port)
{
    struct sockaddr_in socketData;
    socketData.sin_family = AF_INET;
    socketData.sin_port = htons(port);

    if (inet_pton(AF_INET, ip, &socketData.sin_addr) != 1)
    {
        printWSAErrorCleanupAndExit("inet_pton()");
    }

    return socketData;
}

void bindServerSocket(const SOCKET serverSocket, const char *ip, const u_short port)
{
    struct sockaddr_in socketData = createSocketAddress(ip, port);

    printf("Binding server socket to TCP port %s:%d...", ip, port);

    if (bind(serverSocket, (const struct sockaddr *)&socketData, sizeof(socketData)) == SOCKET_ERROR)
    {
        printWSAErrorCleanupAndExit("bind()");
    }

    printf(SUCCESS);
}

void listenOnServerSocket(const SOCKET serverSocket)
{
    printf("Listening on server socket...");

    if (listen(serverSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        printWSAErrorCleanupAndExit("bind()");
    }

    printf(SUCCESS);
}

SOCKET acceptClientSocket(const SOCKET serverSocket)
{
    printf("Accepting client connection...");

    // Next call is blocking, make sure output is written before.
    fflush(stdout);
    const SOCKET clientSocket = accept(serverSocket, NULL, NULL);

    if (clientSocket == INVALID_SOCKET)
    {
        printLastWSAError("accept()");
    }
    else
    {
        printf(SUCCESS);
    }

    return clientSocket;
}

void connectToServerSocket(const SOCKET clientSocket, const char *const ip, const u_short port)
{
    struct sockaddr_in socketData = createSocketAddress(ip, port);

    printf("Binding server socket to TCP port %s:%d...", ip, port);

    if (connect(clientSocket, (const struct sockaddr *)&socketData, sizeof(socketData)) == SOCKET_ERROR)
    {
        printWSAErrorCleanupAndExit("connect()");
    }

    printf(SUCCESS);
}

int receiveData(const SOCKET socket, char *const buffer, const int length)
{
    printf("Waiting for data...\n");

    const int result = recv(socket, buffer, length, 0);

    if (result == 0)
    {
        printf("recv() was unblocked because the connection got closed\n");
    }
    else if (result == SOCKET_ERROR)
    {
        printWSAErrorCleanupAndExit("recv()");
    }

    return result;
}

int sendData(const SOCKET socket, const char *const const buffer, const int length)
{
    const int result = send(socket, buffer, length, 0);

    if (result == SOCKET_ERROR)
    {
        printWSAErrorCleanupAndExit("send()");
    }

    return result;
}
