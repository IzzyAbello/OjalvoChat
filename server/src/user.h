#ifndef USER_H
#define USER_H
 
#include <stdbool.h>
 
#define USER_USERNAME_MAX_LENGTH 8
 
typedef enum
{
    USER_STATUS_ACTIVE,
    USER_STATUS_AWAY,
    USER_STATUS_BUSY
}
User_Status;
 
typedef struct
{
    char username[USER_USERNAME_MAX_LENGTH + 1];
    int socket_fd;
    User_Status status;
}
User;

bool user_init(User* user, const char* username, int socket_fd);
 
const char* user_status_to_string(User_Status status);
 
bool user_status_from_string(const char* text, User_Status* out_status);
 
#endif /* USER_H */