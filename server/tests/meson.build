criterion_dep = dependency('criterion', required: false)

if criterion_dep.found()
    test_server_parse_args = executable (
        'test_server_parse_args',
        'test_server_parse_args.c',
        dependencies: [criterion_dep, glib_dep],
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

    test_user = executable (
        'test_user',
        'test_user.c',
        dependencies: criterion_dep,
        link_with: server_lib
    )
    test('user', test_user)

    test_message = executable (
        'test_message',
        'test_message.c',
        dependencies: [criterion_dep, cjson_dep],
        link_with: server_lib
    )
    test('message', test_message)

    test_users_table = executable (
        'test_users_table',
        'test_users_table.c',
        dependencies: [criterion_dep, glib_dep],
        link_with: server_lib
    )
    test('users_table', test_users_table)

    test_users_table_concurrency = executable (
        'test_users_table_concurrency',
        'test_users_table_concurrency.c',
        dependencies: [criterion_dep, glib_dep, threads_dep],
        link_with: server_lib
    )
    test('users_table_concurrency', test_users_table_concurrency)
else
    warning('Criterion no encontrado: los test no compilarán.\nPor favor instale: \'libcriterion-dev\'.')
endif