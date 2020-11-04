#include "fixedpoint.h"
#include "stdint.h"

// convert an int to a fixed_point_t
fixed_point_t
fixed_point_convert_int(int n)
{
   return n << FIXED_POINT_NUMBER_OF_DECIMAL_DIGITS;
}

// convert a fixed point number to an integer
int
fixed_point_to_integer(fixed_point_t x)
{
   return x >> FIXED_POINT_NUMBER_OF_DECIMAL_DIGITS;
}

// add two fixed point numbers
fixed_point_t
fixed_point_add(fixed_point_t x, fixed_point_t y)
{
   return x + y;
}

// Subtract two fixed point numbers
fixed_point_t
fixed_point_sub(fixed_point_t x, fixed_point_t y)
{
   return x - y;
}

// add an integer to a fixed point number
fixed_point_t
fixed_point_add_int(fixed_point_t x, int n)
{
   return x + (n << FIXED_POINT_NUMBER_OF_DECIMAL_DIGITS);
}

// subtract an integer from a fixed point number
fixed_point_t
fixed_point_sub_int(fixed_point_t x, int n)
{
    return x - (n << FIXED_POINT_NUMBER_OF_DECIMAL_DIGITS);
}

// multiply two fixed point numbers
fixed_point_t
fixed_point_mul(fixed_point_t x, fixed_point_t y)
{
    return (((int64_t)x) * y) / ( 1 << FIXED_POINT_NUMBER_OF_DECIMAL_DIGITS);
}

// multiply a fixed point number and an integer
fixed_point_t
fixed_point_mul_int(fixed_point_t x, int n)
{
    return x * n;
}

// divide two fixed point number
fixed_point_t
fixed_point_div(fixed_point_t x, fixed_point_t y)
{
    return ( ((int64_t)x) << FIXED_POINT_NUMBER_OF_DECIMAL_DIGITS ) / y;
}

// divide a fixed point number to an integer
fixed_point_t
fixed_point_div_int(fixed_point_t x, int n)
{
    return x / n;
}
