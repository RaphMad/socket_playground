#include <stdio.h>
#include <time.h>
#include "..\\socket.h"

#define BUFFER_SIZE 100 * 1000 * 1000
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

    printf("Value of SO_KEEPALIVE: %d\n", getSocketOption(clientSocket, SO_KEEPALIVE));
    printf("Setting SO_KEEPALIVE to %d...\n", TRUE);
    setSocketOption(clientSocket, SO_KEEPALIVE, TRUE);
    printf("Value of SO_KEEPALIVE: %d\n", getSocketOption(clientSocket, SO_KEEPALIVE));

    clock_t ticks = clock();

    printf("Press any key to close socket\n");
    getchar();

    ticks = clock() - ticks;

    double timeTaken = ((double)ticks) / CLOCKS_PER_SEC;

    printf("TCP connection was closed due to timeout after %.2f s.\n", timeTaken);

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
