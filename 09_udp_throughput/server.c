#include <stdio.h>
#include <signal.h>
#include "..\\socket.h"

// Based on ethernet MTU.
#define BUFFER_SIZE 1472
static char buffer[BUFFER_SIZE];

static const u_short serverPort = 9000;

static volatile SOCKET serverSocket = INVALID_SOCKET;

void handleSigInt(int signum);
void echoNextMessage(SOCKET socket);

int main()
{
    signal(SIGINT, handleSigInt);

    initializeWinsock();

    serverSocket = createUdpSocket();
    bindServerSocket(serverSocket, serverPort);

    unblock(serverSocket);
    setVerbosity(FALSE);

    while (TRUE)
    {
        receiveUntil(serverSocket, buffer, BUFFER_SIZE, ' ');
    }

    // Graceful shutdown
    shutdownSocket(serverSocket, SD_BOTH);
    closeSocket(serverSocket);

    cleanupWinsock();

    return EXIT_SUCCESS;
}

void handleSigInt(const int signum)
{
    printf(" Received signal (%d)\n", signum);
    closeSocket(serverSocket);
}
