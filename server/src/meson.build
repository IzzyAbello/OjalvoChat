server_lib = library (
    'server_lib',
    'server.c',
    'message_buffer.c'
)

ojalvochat = executable(
    'ojalvochat',
    'main.c',
    link_with: server_lib
)