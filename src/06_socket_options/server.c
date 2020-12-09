#include <stdio.h>
#include <signal.h>
#include "..\\..\\lib\\socket\\socket.h"

#define BUFFER_SIZE 1000 * 1000 * 100
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

    if (clientSocket != INVALID_SOCKET)
    {
        unblock(clientSocket);

        receiveUntil(clientSocket, buffer, BUFFER_SIZE, ' ');

        printf("SO_KEEPALIVE: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_KEEPALIVE));
        printf("TCP_NODELAY: %d\n", getBooleanSocketOption(clientSocket, IPPROTO_TCP, TCP_NODELAY));
        printf("SO_LINGER.l_onoff: %d\n", getLingerSocketOption(clientSocket).l_onoff);
        printf("SO_LINGER.l_linger: %d\n", getLingerSocketOption(clientSocket).l_linger);
        printf("SO_RCVBUF: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_RCVBUF));
        printf("SO_RCVTIMEO: %d\n", getDWordSocketOption(clientSocket, SOL_SOCKET, SO_RCVTIMEO));
        printf("SO_SNDBUF: %d\n", getBooleanSocketOption(clientSocket, SOL_SOCKET, SO_SNDBUF));
        printf("SO_SNDTIMEO: %d\n", getDWordSocketOption(clientSocket, SOL_SOCKET, SO_SNDTIMEO));

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
