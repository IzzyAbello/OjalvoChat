criterion_dep = dependency('criterion', required: false)

if criterion_dep.found()
    test_server_parse_args = executable (
        'test_server_parse_args',
        'test_server_parse_args.c',
        dependencies: criterion_dep,
        link_with: server_lib
    )
    test('server_parse_args', test_server_parse_args)

    test_message_buffer = executable (
        'test_message_buffer',
        'test_message_buffer.c',
        dependencies: criterion_dep,
        link_with: server_lib
    )
    test('message_buffer', test_message_buffer)
else
    warning('Criterion no encontrado: los test no compilarán.\nPor favor instale: \'libcriterion-dev\'.')
endif