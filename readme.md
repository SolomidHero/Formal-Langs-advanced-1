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

<i>expression</i> - string of arithmetic expression

<i>symbol_t</i> - struct describes global variables and functions

<i>out_buffer</i> - pointer to the block of executable code

<b>Input</b>:
Definitions of variables in .vars segment and
arithmetic expression in .expression segment

<b>Output</b>:
Value of expression

## Details

Build and launch:

```
// make sure you have linked 
// arm-linux-gnueabi-g++ and arm-linux-gnueabi-gcc
// arm sysroot library must be located at ~/arm-sysroot
make
qemu-arm -L [path_to_arm_sysroot] ./program

// also you can execute by make
make launch
```

Tests:
```
// compile and link for test
make test
make launch_test
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
