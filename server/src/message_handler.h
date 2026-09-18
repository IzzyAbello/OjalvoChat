#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

#include <stdbool.h>

#include "server.h"
#include "users_table.h"
#include "message.h"

bool message_handler_process(
    Server* server,
    const Message* msg_in,
    int client_fd
);
 
#endif /* MESSAGE_HANDLER_H */