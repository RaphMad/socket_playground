#include <stdio.h>
#include <signal.h>
#include "..\\socket.h"

// 1.9GB, about the largest size of memory that can be allocated before running into errors.
// Note that you would not do this in production to send/receive large amounts of data,
// but rather stream individual chunks of whatever is being transferred.
#define BUFFER_SIZE 1900000000
static char buffer[BUFFER_SIZE];

static const char *const ip = "192.168.1.1";
static const u_short port = 9000;

static volatile SOCKET serverSocket = INVALID_SOCKET;

void handleSigInt(int signum);
void echoNextMessage(SOCKET socket);

int main()
{
    signal(SIGINT, handleSigInt);

    initializeWinsock();

    serverSocket = createTcpSocket();
    bindServerSocket(serverSocket, ip, port);
    listenOnServerSocket(serverSocket);

    const SOCKET clientSocket = acceptClientSocket(serverSocket);

    // No longer need the server socket at this point.
    closeSocket(serverSocket);

    if (clientSocket != INVALID_SOCKET)
    {
        echoNextMessage(clientSocket);

        // clientSocket shutdown is done within echoNextMessage() at the earliest point in time possible
        closeSocket(clientSocket);
    }

    cleanupWinsock();

    return EXIT_SUCCESS;
}

void echoNextMessage(const SOCKET socket)
{
    // Read data until the buffer is full or 'X' is received.
    const u_long numberOfBytesReceived = receiveUntil(socket, buffer, BUFFER_SIZE, 'X');

    // Graceful shutdown.
    // This has no notable effect on our static communication protocol,
    // but would cause the connection to be reset if the client tried to send more data.
    shutdownSocket(socket, SD_RECEIVE);

    const char postfix[] = {'S', 'Y'};
    memcpy(buffer + numberOfBytesReceived, postfix, sizeof(postfix));

    sendAllData(socket, buffer, numberOfBytesReceived + sizeof(postfix));
}

void handleSigInt(const int signum)
{
    printf(" Received signal (%d)\n", signum);
    closeSocket(serverSocket);
}
