#ifndef GLOBAL_H_
#define GLOBAL_H_

extern char* uuid_str;

#ifdef __linux__
#define SOCKET_ERROR SO_ERROR
#define INVALID_SOCKET -1
typedef int SOCKET;

void closesocket(int socket);

#endif

#endif // GLOBAL_H_
