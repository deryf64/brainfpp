#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void print_mem(unsigned char *mem, size_t mem_size, size_t ptr) {
  printf("[ ");
  for (size_t i = 0; i < mem_size; i++) {
    if (ptr == i) {
      printf("\033[31m%d\033[0m ", mem[i]);
    } else {
      printf("%d ", mem[i]);
    }
  }
  printf("]");
}

int run(char *line, int debug) {
  unsigned char *mem = calloc(1, sizeof(unsigned char));
  if (!mem) return 1;

  size_t ptr = 0;
  size_t mem_size = 1;
  int status = 0;
  size_t code_len = strlen(line);

  for (size_t i = 0; i < code_len; i++) {
    char c = line[i];

    switch (c) {
      case '+': mem[ptr] += 1; break;
      case '-': mem[ptr] -= 1; break;
      case '>': ptr += 1; break;
      case '<': ptr -= 1; break;
      case '[':
        if (mem[ptr] == 0) {
          size_t deph = 1;
          while (deph && ++i < code_len) {
            if (line[i] == '[') deph++;
            else if (line[i] == ']') deph--;
          }
        }
        break;
      case ']':
        if (mem[ptr] != 0) {
          size_t deph = 1;
          while (deph && i > 0) {
            i--;
            if (line[i] == ']') deph++;
            else if (line[i] == '[') deph--;
          };
        }
        break;
      case '.': putchar(mem[ptr]); break;
      case ',':
        mem[ptr] += (unsigned char)getchar();
        break;
      case '}': i += mem[ptr]; break;
      case '{': i -= mem[ptr]; break;
      case '=': {
        size_t new_size = mem_size + mem[ptr];
        unsigned char *new_mem = realloc(mem, new_size);
        if (!new_mem) {
          status = 1;
          goto cleanup;
        }
        memset(new_mem + mem_size, 0, new_size - mem_size);
        mem = new_mem;
        mem_size = new_size;
        break;
      }
    }

    if (ptr >= mem_size) {
      fprintf(stderr, "Error: pointer out of memory range!\n");
      status = 1;
      break;
    }

    if (debug) {
      printf("\t cmd: \033[32m%c\033[0m mem: ", c);
      print_mem(mem, mem_size, ptr);
      printf("\n");
    }
  }

cleanup:
  free(mem);
  return status;
}

int main(int argc, char *argv[]) {
  char *code = argv[1];
  int debug = 0;
  if (argv[2]) {
    debug = 1;
  }

  return run(code, debug);
}