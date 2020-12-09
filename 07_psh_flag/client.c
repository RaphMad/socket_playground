#include <stdio.h>
#include "..\\lib\\socket\\socket.h"

// Just a bit more than MSS
#define BUFFER_SIZE 1461
static char buffer[BUFFER_SIZE];

static const char *const serverIp = "192.168.1.1";
static const u_short serverPort = 9000;

void sendMessageToServer(SOCKET socket, const char *message, size_t size);
void recieveMessageFromServer(SOCKET socket);

int main()
{
    initializeWinsock();

    const SOCKET clientSocket = createTcpSocket();

    connectToServerSocket(clientSocket, serverIp, serverPort);
    unblock(clientSocket);

    memset(buffer, 'X', BUFFER_SIZE);
    sendAllData(clientSocket, buffer, BUFFER_SIZE);

    // Graceful shutdown
    shutdownSocket(clientSocket, SD_BOTH);
    closeSocket(clientSocket);

    cleanupWinsock();

    return EXIT_SUCCESS;
}

void sendMessageToServer(const SOCKET socket, const char *const message, const size_t size)
{
    sendAllData(socket, message, size);
}

void recieveMessageFromServer(const SOCKET socket)
{
    receiveUntil(socket, buffer, BUFFER_SIZE, 'Y');
}
