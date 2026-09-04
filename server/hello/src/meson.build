project('ojalvochat', 'c')

criterion = dependency('criterion')

test_sum = executable (
    'test_sum',
    'sum.c', 
    'test_sum.c', 
    dependencies: criterion
)

test('sum', test_sum)