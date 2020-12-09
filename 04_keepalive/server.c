#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "..\\lib\\socket\\socket.h"

#define BUFFER_SIZE 100 * 1000 * 1000
static char buffer[BUFFER_SIZE];

static const u_short serverPort = 9000;

static volatile SOCKET serverSocket = INVALID_SOCKET;

void handleSigInt(int signum);
void echoNextMessage(SOCKET socket);

int main()
{
    signal(SIGINT, handleSigInt);

    initializeWinsock();

    serverSocket = createTcpSocket();
    bindServerSocket(serverSocket, serverPort);
    listenOnServerSocket(serverSocket);

    const SOCKET clientSocket = acceptClientSocket(serverSocket);

    // No longer need the server socket at this point.
    closeSocket(serverSocket);

    printf("Value of SO_KEEPALIVE: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_KEEPALIVE));
    printf("Setting SO_KEEPALIVE to %d...\n", TRUE);
    setBooleanSocketOption(clientSocket, SOL_SOCKET, SO_KEEPALIVE, TRUE);
    printf("Value of SO_KEEPALIVE: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_KEEPALIVE));

    if (clientSocket != INVALID_SOCKET)
    {
        clock_t ticks = clock();

        printf("Press any key to close socket\n");
        getchar();

        ticks = clock() - ticks;

        double timeTaken = ((double)ticks) / CLOCKS_PER_SEC;

        printf("TCP connection was closed due to timeout after %.2f s.\n", timeTaken);

        // Graceful shutdown
        shutdownSocket(clientSocket, SD_BOTH);
        closeSocket(clientSocket);
    }

    cleanupWinsock();

    return EXIT_SUCCESS;
}

void handleSigInt(const int signum)
{
    printf(" Received signal (%d)\n", signum);
    closeSocket(serverSocket);
}
