criterion = dependency('criterion')

test_server_parse_args = executable (
    'test_server_parse_args',
    'test_server_parse_args.c',
    dependencies: criterion,
    link_with: server_lib
)

test('server_parse_args', test_server_parse_args)