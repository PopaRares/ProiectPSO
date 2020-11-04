#ifndef _FIXED_POINT_INCLUDED_
#define _FIXED_POINT_INCULDED_

/* This file provides a basic implementation for fixed point arithmetics
 * Fixed point numbers are in signed p.q format where p+q=31
 */

#define FIXED_POINT_NUMBER_OF_DECIMAL_DIGITS 14

typedef int fixed_point_t;

// convert an int to a fixed_point_t
fixed_point_t
fixed_point_convert_int(int n);

// convert a fixed point number to an integer 
int
fixed_point_to_integer(fixed_point_t x);

// add two fixed point numbers
fixed_point_t
fixed_point_add(fixed_point_t x, fixed_point_t y);

// Subtract two fixed point numbers
fixed_point_t
fixed_point_sub(fixed_point_t x, fixed_point_t y);

// add an integer to a fixed point number
fixed_point_t
fixed_point_add_int(fixed_point_t x, int n);

// subtract an integer from a fixed point number
fixed_point_t
fixed_point_sub_int(fixed_point_t x, int n);

// multiply two fixed point numbers
fixed_point_t
fixed_point_mul(fixed_point_t x, fixed_point_t y);

// multiply a fixed point number and an integer
fixed_point_t
fixed_point_mul_int(fixed_point_t x, int n);

// divide two fixed point number
fixed_point_t
fixed_point_div(fixed_point_t x, fixed_point_t y);

// divide a fixed point number to an integer
fixed_point_t
fixed_point_div_int(fixed_point_t x, int n);

#endif
