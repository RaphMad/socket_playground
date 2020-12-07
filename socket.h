#include <winsock2.h>

void initializeWinsock();
void cleanupWinsock();

SOCKET createSocket();
void shutdownSocket(SOCKET socket, int mode);
void closeSocket(SOCKET socket);

void bindServerSocket(SOCKET serverSocket, const char *ip, u_short port);
void listenOnServerSocket(SOCKET serverSocket);
SOCKET acceptClientSocket(SOCKET serverSocket);

void connectToServerSocket(SOCKET clientSocket, const char *ip, u_short port);

int sendData(SOCKET socket, const char *buffer, int length);
int receiveData(SOCKET socket, char *buffer, int length);

void sendAllData(SOCKET socket, const char *buffer, size_t length);
size_t receiveUntil(SOCKET socket, char *buffer, size_t length, char delimiter);

void unblock(SOCKET socket);

BOOL getBooleanSocketOption(SOCKET socket, int level, int option);
void setBooleanSocketOption(SOCKET socket, int level, int option, BOOL value);
