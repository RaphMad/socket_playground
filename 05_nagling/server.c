#include <stdio.h>
#include <signal.h>
#include "..\\socket.h"

#define BUFFER_SIZE 1000
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

    serverSocket = createTcpSocket();
    bindServerSocket(serverSocket, serverIp, serverPort);
    listenOnServerSocket(serverSocket);

    const SOCKET clientSocket = acceptClientSocket(serverSocket);

    // No longer need the server socket at this point.
    closeSocket(serverSocket);

    if (clientSocket != INVALID_SOCKET)
    {
        unblock(clientSocket);

        receiveUntil(clientSocket, buffer, BUFFER_SIZE, ' ');

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
