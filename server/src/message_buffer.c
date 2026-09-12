#include <string.h>
#include <ctype.h>

#include "message_buffer.h"

void message_buffer_init(Message_Buffer *buffer)
{
    buffer->length = 0;
}

int message_buffer_append(Message_Buffer *buffer, const char *chunk, size_t length)
{
    if (buffer->length + length > SERVER_BUFFER_SIZE) return -1;
    
    memcpy(buffer->data + buffer->length, chunk, length);
    buffer->length += length;
    
    return 0;
}

bool message_buffer_extract(
        Message_Buffer *buffer,
        char *out,
        size_t out_capacity,
        size_t *out_length
    ) 
{
    if (out_capacity == 0) return false;

    char *newline = memchr(buffer->data, '\n', buffer->length);
    
    if (newline == NULL) return false;

    size_t raw_length = (size_t)(newline - buffer->data);
    size_t copy_length = (raw_length < out_capacity - 1) ? raw_length : out_capacity - 1;

    memcpy(out, buffer->data, copy_length);
    out[copy_length] = '\0';
    if (out_length != NULL) *out_length = raw_length;

    size_t consumed = raw_length + 1;
    size_t remaining = buffer->length - consumed;
    memmove(buffer->data, buffer->data + consumed, remaining);
    buffer->length = remaining;

    return true;
}

bool message_is_valid(const char *text, size_t length) 
{
    for (size_t i = 0; i < length; i++)
    {
        char rp = text[i];
        if (rp == '\0' || rp == '\t' || rp == '\r'|| !isprint(rp))
            return false;
    }
    return true;
}

void message_buffer_sanitize(char *text, size_t *length)
{
    size_t write_pos = 0;
 
    for (size_t read_pos = 0; read_pos < *length; read_pos++)
    {
        char rp = text[read_pos];
        if (rp == '\0' || rp == '\t' || rp == '\r'|| !isprint(rp))
            continue;
        text[write_pos++] = text[read_pos];
    }
 
    text[write_pos] = '\0';
    *length = write_pos;
}
