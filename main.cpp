#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>

extern "C" int evaluate(FILE* text);

int main() {
  printf("%d\n", evaluate(stdin));
  return 0;
}