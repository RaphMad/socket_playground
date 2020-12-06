#include <stdio.h>
#include <signal.h>
#include <time.h>
#include "..\\socket.h"

#define BUFFER_SIZE 100 * 1000 * 1000
static char buffer[BUFFER_SIZE];

static const char *const serverIp = "192.168.1.1";
static const u_short serverPort = 9000;

static volatile SOCKET serverSocket = INVALID_SOCKET;

void handleSigInt(int signum);
void echoNextMessage(SOCKET socket);

int main()
{
    signal(SIGINT, handleSigInt);

    initializeWinsock();

    serverSocket = createSocket();
    bindServerSocket(serverSocket, serverIp, serverPort);
    listenOnServerSocket(serverSocket);

    const SOCKET clientSocket = acceptClientSocket(serverSocket);

    // No longer need the server socket at this point.
    closeSocket(serverSocket);

    printf("Value of SO_KEEPALIVE: %d\n", getSocketOption(clientSocket, SO_KEEPALIVE));
    printf("Setting SO_KEEPALIVE to true...\n");
    setSocketOption(clientSocket, SO_KEEPALIVE, TRUE);
    printf("Value of SO_KEEPALIVE: %d\n", getSocketOption(clientSocket, SO_KEEPALIVE));

    if (clientSocket != INVALID_SOCKET)
    {
        clock_t ticks = clock();

        // Perform a blocking receive thats never fulfilled
        receiveData(clientSocket, buffer, BUFFER_SIZE);

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
