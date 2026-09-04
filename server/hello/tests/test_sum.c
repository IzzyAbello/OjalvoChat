#include <criterion/criterion.h>
#include "sum.h"

Test(sum, positive_numbers)
{
    cr_assert(sum(5, 4) == 9);
    cr_assert(sum(4, 5) == 9);
    cr_assert(sum(5, 5) == 10);
}

Test (sum, zero)
{
    cr_assert(sum(0, 0) == 0);
}

Test(sum, negative_numbers) 
{
    cr_assert(sum(-10, -3) == -13);
}

Test(sum, mixed_numbers)
{
    cr_assert(sum(-2, 4) == 2);
}