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

    if (clientSocket != INVALID_SOCKET)
    {
        // In theory, unblocking a socket for receiving should not yield
        // big performance gains (as we are reading in al oop anyway).
        // Actually it seems the blocking TCP implementation is too conservative with the chunks
        // it delivers so some scenarios (WLAN) the receive window fills up.
        // This does not happen with non-blocking reads in a loop in the same WLAN environment
        // (it can also be seen that smaller chunks are received and wireshark shows no TCP window).
        unblock(clientSocket);
        setVerbosity(FALSE);

        // Read data until the buffer is full or 'X' is received.
        clock_t ticks = clock();
        receiveUntil(clientSocket, buffer, BUFFER_SIZE, 'X');
        ticks = clock() - ticks;

        double timeTaken = ((double)ticks) / CLOCKS_PER_SEC;
        double mbitPerSecond = BUFFER_SIZE / 1000 / 1000 / timeTaken * 8;

        printf("Received %d MByte in %.2f s - %.2f MBit/s.\n",
               BUFFER_SIZE / 1000 / 1000,
               timeTaken,
               mbitPerSecond);

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
