threads_dep = dependency('threads')

server_lib = library (
    'server_lib',
    'server.c',
    'message_buffer.c',
    dependencies: threads_dep
)

ojalvochat = executable(
    'ojalvochat',
    'main.c',
    link_with: server_lib,
    dependencies: threads_dep
)