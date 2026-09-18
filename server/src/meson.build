threads_dep = dependency('threads')
glib_dep = dependency('glib-2.0')
cjson_dep = dependency('libcjson')

server_lib = library (
    'server_lib',
    'server.c',
    'message_buffer.c',
    'client_handler.c',
    'user.c',
    'users_table.c',
    'message.c',
    'message_handler.c',
    dependencies: [threads_dep, glib_dep, cjson_dep]
)

ojalvochat = executable(
    'ojalvochat',
    'main.c',
    link_with: server_lib,
    dependencies: [threads_dep, glib_dep, cjson_dep]
)