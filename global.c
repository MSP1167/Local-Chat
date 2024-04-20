#include "global.h"

#ifdef __linux__
#include <unistd.h>
void closesocket(int socket) {
    close(socket);
}
#endif
