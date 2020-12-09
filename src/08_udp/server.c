#include <stdio.h>
#include <signal.h>
#include "..\\..\\lib\\socket\\socket.h"

// Max IP segment size, will cause fragmentation.
#define BUFFER_SIZE 65507
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
    const int maxMessageSize = getIntegerSocketOption(serverSocket, SOL_SOCKET, SO_MAX_MSG_SIZE);
    bindServerSocket(serverSocket, serverPort);

    unblock(serverSocket);

    receiveUntil(serverSocket, buffer, maxMessageSize, ' ');

    printf("Last 2 chars: %c%c\n", buffer[maxMessageSize - 2], buffer[maxMessageSize - 1]);

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
