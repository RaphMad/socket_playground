#include "socket.h"

#include <stdio.h>
#include <limits.h>
#include <ws2tcpip.h>

#pragma comment(lib, "ws2_32.lib")

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_YELLOW "\x1b[33m"
#define ANSI_COLOR_RESET "\x1b[0m"

#define SUCCESS ANSI_COLOR_GREEN " success" ANSI_COLOR_RESET "\n"
#define INFO_MESSAGE(string) ANSI_COLOR_YELLOW string ANSI_COLOR_RESET
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

SOCKET createTcpSocket()
{
    printf("Creating TCP socket...");

    SOCKET createdSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (createdSocket == INVALID_SOCKET)
    {
        printWSAErrorCleanupAndExit("socket()");
    }

    printf(SUCCESS);

    return createdSocket;
}

SOCKET createUdpSocket()
{
    printf("Creating UDP socket...");

    SOCKET createdSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    if (createdSocket == INVALID_SOCKET)
    {
        printWSAErrorCleanupAndExit("socket()");
    }

    printf(SUCCESS);

    return createdSocket;
}

void shutdownSocket(SOCKET socket, int mode)
{
    if (socket != INVALID_SOCKET)
    {
        printf("Shutting down socket...");

        if (shutdown(socket, mode) == SOCKET_ERROR)
        {
            printLastWSAError("shutdown()");
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

    printf("Binding server socket to %s:%d...", ip, port);

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
        printWSAErrorCleanupAndExit("listen()");
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

    printf("Connecting to server socket at %s:%d...", ip, port);

    if (connect(clientSocket, (const struct sockaddr *)&socketData, sizeof(socketData)) == SOCKET_ERROR)
    {
        printWSAErrorCleanupAndExit("connect()");
    }

    printf(SUCCESS);
}

int sendData(const SOCKET socket, const char *const const buffer, const int length)
{
    const int result = send(socket, buffer, length, 0);

    if (result == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
            printf(INFO_MESSAGE("Cannot send right now\n"));
        }
        else
        {
            printWSAErrorCleanupAndExit("send()");
        }
    }
    else
    {
        printf(INFO_MESSAGE("Sent %d bytes\n"), result);
    }

    return result;
}

int receiveData(const SOCKET socket, char *const buffer, const int length)
{
    int result = recv(socket, buffer, length, 0);

    if (result == 0)
    {
        printf("recv() was unblocked because the connection got closed\n");
    }
    else if (result == SOCKET_ERROR)
    {
        if (WSAGetLastError() == WSAEWOULDBLOCK)
        {
            // Not ideal, because we will most likely be called in a loop - either busy waiting
            // or sleeps that may queue up incoming data.
            //Sleep(1);
        }
        else
        {
            printWSAErrorCleanupAndExit("recv()");
        }
    }
    else
    {
        printf(INFO_MESSAGE("Received %d bytes\n"), result);
    }

    return result;
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))

void sendAllData(const SOCKET socket, const char *const buffer, const size_t length)
{
    size_t sentBytes = 0;

    while (sentBytes != length)
    {
        sentBytes += sendData(socket, buffer + sentBytes, (int)(MIN(length, INT_MAX) - sentBytes));
    }
}

size_t receiveUntil(const SOCKET socket, char *const buffer, const size_t length, const char delimiter)
{
    printf("Waiting for data terminated by <%c> up to an amount of %zu bytes...\n", delimiter, length);

    size_t receivedBytes = 0;

    while (receivedBytes < length)
    {
        const int max_chunk = (int)MIN(length - receivedBytes, INT_MAX);
        const size_t result = receiveData(socket, buffer + receivedBytes, max_chunk);

        // Returning with -1 is normal for non-blocking cases when no data was available.
        if (result != -1)
        {
            receivedBytes += result;

            if (result == 0 || buffer[receivedBytes - 1] == delimiter)
            {
                break;
            }
        }
    }

    return receivedBytes;
}

void unblock(const SOCKET socket)
{

    if (socket != INVALID_SOCKET)
    {
        printf("Unblocking socket...");

        unsigned long ul = 1;

        if (ioctlsocket(socket, FIONBIO, &ul) == SOCKET_ERROR)
        {
            printLastWSAError("shutdown()");
        }
        else
        {
            printf(SUCCESS);
        }
    }
}

BOOL getBooleanSocketOption(const SOCKET socket, const int level, const int option)
{
    BOOL value = FALSE;
    int sizeOfBool = sizeof(BOOL);

    if (getsockopt(socket, level, option, (char *)&value, &sizeOfBool) == SOCKET_ERROR)
    {
        printLastWSAError("getsockopt()");
    }

    return value;
}

void setBooleanSocketOption(const SOCKET socket, const int level, const int option, const BOOL value)
{
    if (setsockopt(socket, level, option, (char *)&value, sizeof(BOOL)) == SOCKET_ERROR)
    {
        printLastWSAError("getsockopt()");
    }
}

int getIntegerSocketOption(SOCKET socket, int level, int option)
{
    int value = 0;
    int sizeOfInt = sizeof(int);

    if (getsockopt(socket, level, option, (char *)&value, &sizeOfInt) == SOCKET_ERROR)
    {
        printLastWSAError("getsockopt()");
    }

    return value;
}

void setIntegerSocketOption(SOCKET socket, int level, int option, int value)
{
    if (setsockopt(socket, level, option, (char *)&value, sizeof(int)) == SOCKET_ERROR)
    {
        printLastWSAError("getsockopt()");
    }
}

DWORD getDWordSocketOption(SOCKET socket, int level, int option)
{
    int value = 0;
    int sizeOfDword = sizeof(DWORD);

    if (getsockopt(socket, level, option, (char *)&value, &sizeOfDword) == SOCKET_ERROR)
    {
        printLastWSAError("getsockopt()");
    }

    return value;
}

void setDWordSocketOption(SOCKET socket, int level, int option, DWORD value)
{
    if (setsockopt(socket, level, option, (char *)&value, sizeof(DWORD)) == SOCKET_ERROR)
    {
        printLastWSAError("getsockopt()");
    }
}

struct linger getLingerSocketOption(SOCKET socket)
{
    struct linger value;
    int sizeOfLinger = sizeof(struct linger);

    if (getsockopt(socket, SOL_SOCKET, SO_LINGER, (char *)&value, &sizeOfLinger) == SOCKET_ERROR)
    {
        printLastWSAError("getsockopt()");
    }

    return value;
}

void setLingerSocketOption(SOCKET socket, const struct linger lingerOption)
{
    if (setsockopt(socket, SOL_SOCKET, SO_LINGER, (char *)&lingerOption, sizeof(struct linger)) == SOCKET_ERROR)
    {
        printLastWSAError("getsockopt()");
    }
}
