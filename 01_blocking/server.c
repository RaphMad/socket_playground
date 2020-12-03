#include <stdio.h>
#include <signal.h>
#include "..\\\socket.h"

#define BUFFER_SIZE 1024
char buffer[BUFFER_SIZE];

const char *const ip = "127.0.0.33";
const u_short port = 9000;

volatile SOCKET serverSocket = INVALID_SOCKET;

void handleSigInt(int signum);
void echoNextMessage(SOCKET socket);

int main()
{
    signal(SIGINT, handleSigInt);

    initializeWinsock();

    serverSocket = createSocket();
    bindServerSocket(serverSocket, ip, port);
    listenOnServerSocket(serverSocket);

    const SOCKET clientSocket = acceptClientSocket(serverSocket);

    if (clientSocket != INVALID_SOCKET)
    {
        // No longer need the server socket at this point.
        shutdownSocket(serverSocket, SD_BOTH);
        closeSocket(serverSocket);

        echoNextMessage(clientSocket);

        // Simulate a small delay for a more realistic shutdown sequence, otherwise both endpoints will send their FIN
        // nearl simultaneously because of the static communication protocol.
        // (Usually servers will not know if clients want to be serviced longer)
        Sleep(100);
        closeSocket(clientSocket);
    }

    cleanupWinsock();

    return EXIT_SUCCESS;
}

void echoNextMessage(const SOCKET socket)
{
    const int numberOfBytesReceived = receiveData(socket, buffer, BUFFER_SIZE);
    printf("Received %d bytes: <%.*s>\n", numberOfBytesReceived, numberOfBytesReceived, buffer);

    // Graceful shutdown.
    // This has no notable effect on our static communication protocol,
    // but would cause the connection to be reset if the client tried to send more data.
    shutdownSocket(socket, SD_RECEIVE);

    const char postfix[] = {' ', '-', ' ', 'p', 'o', 'n', 'g'};
    memcpy(buffer + numberOfBytesReceived, postfix, sizeof(postfix));

    const int numberOfBytesSent = sendData(socket, buffer, numberOfBytesReceived + sizeof(postfix));
    printf("Echoed %d bytes back to sender: <%.*s>\n", numberOfBytesSent, numberOfBytesSent, buffer);
}

void handleSigInt(const int signum)
{
    printf(" Received signal (%d)\n", signum);
    closeSocket(serverSocket);
}
