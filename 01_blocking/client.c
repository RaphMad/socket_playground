#include <stdio.h>
#include "..\\\socket.h"

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];

const char *const ip = "127.0.0.33";
const u_short port = 9000;

void sendMessageToServer(SOCKET socket, const char *const message, const int size);
void recieveMessageFromServer(SOCKET socket);

int main()
{

    initializeWinsock();

    const SOCKET clientSocket = createSocket();

    connectToServerSocket(clientSocket, ip, port);

    const char message[] = {'p', 'i', 'n', 'g'};
    sendMessageToServer(clientSocket, message, sizeof(message));

    // Graceful shutodwn - we signal to the server that we no longer want to send data.
    // This allows to send an "early FIN" to the server.
    // The server can still respond with some last data followed by his FIN.
    shutdownSocket(clientSocket, SD_SEND);

    // Read all outstanding data from the server.
    recieveMessageFromServer(clientSocket);

    // At this point the socket can be closed.
    closeSocket(clientSocket);

    cleanupWinsock();

    return EXIT_SUCCESS;
}

void sendMessageToServer(const SOCKET socket, const char *const message, const int size)
{
    const int numberOfBytesSent = sendData(socket, message, size);
    printf("Sent %d bytes to server: <%.*s>\n", numberOfBytesSent, numberOfBytesSent, message);
}

void recieveMessageFromServer(const SOCKET socket)
{
    const int numberOfBytesReceived = receiveData(socket, buffer, BUFFER_SIZE);
    printf("Received %d bytes: <%.*s>\n", numberOfBytesReceived, numberOfBytesReceived, buffer);
}
