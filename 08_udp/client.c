#include <stdio.h>
#include "..\\lib\\socket\\socket.h"

// Max IP segment size, will cause fragmentation.
#define BUFFER_SIZE 65507
static char buffer[BUFFER_SIZE];

static const char *const serverIp = "192.168.1.1";
static const u_short serverPort = 9000;

void sendMessageToServer(SOCKET socket, const char *message, size_t size);
void recieveMessageFromServer(SOCKET socket);

int main()
{
    initializeWinsock();

    const SOCKET clientSocket = createUdpSocket();
    const int maxMessageSize = getIntegerSocketOption(clientSocket, SOL_SOCKET, SO_MAX_MSG_SIZE);

    connectToServerSocket(clientSocket, serverIp, serverPort);
    unblock(clientSocket);

    memset(buffer, 'U', maxMessageSize - 1);
    buffer[maxMessageSize - 1] = 'X';

    sendAllData(clientSocket, buffer, maxMessageSize);

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
