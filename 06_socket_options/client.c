#include <stdio.h>
#include "..\\socket.h"

#define BUFFER_SIZE 1000 * 1000 * 100
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

    memset(buffer, 'X', BUFFER_SIZE);
    sendAllData(clientSocket, buffer, BUFFER_SIZE);

    printf("SO_KEEPALIVE: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_KEEPALIVE));
    printf("TCP_NODELAY: %d\n", getBooleanSocketOption(clientSocket, IPPROTO_TCP, TCP_NODELAY));
    printf("SO_LINGER.l_onoff: %d\n", getLingerSocketOption(clientSocket).l_onoff);
    printf("SO_LINGER.l_linger: %d\n", getLingerSocketOption(clientSocket).l_linger);
    printf("SO_RCVBUF: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_RCVBUF));
    printf("SO_RCVTIMEO: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_RCVTIMEO));
    printf("SO_SNDBUF: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_SNDBUF));
    printf("SO_SNDTIMEO: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_SNDTIMEO));

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
