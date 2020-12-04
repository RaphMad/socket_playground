#include <stdio.h>
#include <signal.h>
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

    if (clientSocket != INVALID_SOCKET)
    {
        // No longer need the server socket at this point.
        closeSocket(serverSocket);

        unblock(clientSocket);

        // Read data until the buffer is full or 'X' is received.
        receiveUntil(clientSocket, buffer, BUFFER_SIZE, 'X');

        // Graceful shutdown
        shutdownSocket(clientSocket, SD_BOTH);

        // clientSocket shutdown is done within echoNextMessage() at the earliest point in time possible.
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
