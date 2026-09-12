#ifndef MESSAGE_BUFFER_H
#define MESSAGE_BUFFER_H

#include <stdbool.h>
#include <stddef.h>

#define SERVER_BUFFER_SIZE (1024 * 1024)

typedef struct 
{
    char data[SERVER_BUFFER_SIZE];
    size_t length;
}
Message_Buffer;

void message_buffer_init(Message_Buffer *buffer);

int message_buffer_append(Message_Buffer *buffer, const char *chunk, size_t length);

bool message_buffer_extract(Message_Buffer *buffer, char *out, size_t out_capacity, size_t *out_length);

bool message_is_valid(const char *text, size_t length);

void message_buffer_sanitize(char *text, size_t *length);

#endif