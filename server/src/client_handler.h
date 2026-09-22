#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H
 
#include "server.h"

typedef struct
{
    Server* server;
    int client_fd;
}
Client_Args;
 
void* handle_client(void* arg);
 
#endif /* CLIENT_HANDLER_H */