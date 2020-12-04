#include <stdio.h>
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

    unblock(clientSocket);

    // Build message consisting of 'CCCC...X'
    // Termination character is important because the server will rely on it to know that no more data will be received.
    const size_t messageLength = BUFFER_SIZE;
    memset(buffer, 'C', messageLength - 1);
    buffer[messageLength - 1] = 'X';

    sendMessageToServer(clientSocket, buffer, messageLength);

    // Graceful shutodwn - we signal to the server that we no longer want to send data.
    // This allows to send an "early FIN" to the server.
    // The server can still respond with some last data followed by his FIN.
    shutdownSocket(clientSocket, SD_SEND);

    // At this point the socket can be closed.
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
