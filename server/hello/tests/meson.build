criterion = dependency('criterion')

test_sum = executable (
    'test_sum', 
    'test_sum.c', 
    dependencies: criterion,
    link_with: arithmetics 
)

test('sum', test_sum)