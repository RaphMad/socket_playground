#include <stdio.h>
#include "..\\socket.h"

#define BUFFER_SIZE 1000
static char buffer[BUFFER_SIZE];

static const char *const serverIp = "192.168.1.1";
static const u_short serverPort = 9000;

void sendMessageToServer(SOCKET socket, const char *message, size_t size);
void recieveMessageFromServer(SOCKET socket);

int main()
{
    initializeWinsock();

    const SOCKET clientSocket = createSocket();

    connectToServerSocket(clientSocket, serverIp, serverPort);
    unblock(clientSocket);

    //setBooleanSocketOption(clientSocket, IPPROTO_TCP, TCP_NODELAY, TRUE);
    printf("Set TCP_NODELAY to %d\n", getBooleanSocketOption(clientSocket, IPPROTO_TCP, TCP_NODELAY));

    for (int i = 0; i < BUFFER_SIZE; i++)
    {
        sendData(clientSocket, "P", 1);
    }

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
