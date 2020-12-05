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

    // Send seems send is always take the full data (even 1.9GB), so unblocking a send socket does not do a lot.
    unblock(clientSocket);

    // Build message consisting of 'CCCC...X'
    // Termination character is important because the server will rely on it to know that no more data will be received.
    const size_t messageLength = BUFFER_SIZE;
    memset(buffer, 'C', messageLength - 1);
    buffer[messageLength - 1] = 'X';

    clock_t ticks = clock();
    sendMessageToServer(clientSocket, buffer, messageLength);
    ticks = clock() - ticks;

    double timeTaken = ((double)ticks) / CLOCKS_PER_SEC;
    double mbitPerSecond = BUFFER_SIZE / 1000 / 1000 / timeTaken * 8;

    printf("Sent %d MByte in %.2f s - %.2f MBit/s (time taken to put it into the receive buffer).\n",
           BUFFER_SIZE / 1000 / 1000,
           timeTaken,
           mbitPerSecond);

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
