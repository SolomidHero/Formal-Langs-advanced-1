# Expression calculator (based on arm assembly)

## Description

This program evaluates arithmetic expression with custom variables and functions.
There is an implementation of function with definition:

```
typedef struct {
	const char* name;
	void* pointer;
} symbol_t;

extern void
jit_compile_expression_to_arm(const char * expression, const symbol_t * externs, void * out_buffer);
```

expression - string of arithmetic expression
symbol_t - struct describes global variables and functions
out_buffer - pointer to the block of executable code

Input:
Definitions of variables in .vars segment
Arithmetic expression in .expression segment

Output:
Value of expression

## Details

Build and launch:

```
// make sure you have linked 
// arm-linux-gnueabi-g++ and arm-linux-gnueabi-gcc
// arm sysroot library must be located at ~/arm-sysroot
make
./program
```

## Example

Input:
```
.vars
a=1 b=2 c=3
.expression
(1+a)*c + div(2+4,2)
```
Output:
```
9
```
