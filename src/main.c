#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

int run(char *line, size_t default_size, int debug) {
  unsigned char *mem = calloc(default_size, sizeof(unsigned char));
  if (!mem) return 1;

  size_t ptr = 0;
  size_t mem_size = default_size;
  int status = 0;
  size_t code_len = strlen(line);

  for (size_t i = 0; i < code_len; i++) {
    char c = line[i];

    switch (c) {
      case ' ': continue;
      case '\t': continue;
      case '#':
        i++;
        while (i < code_len && line[i] != '#') i++;
        if (i < code_len && line[i] == '#') i++;
        continue;
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
      case '@': print_mem(mem, mem_size, ptr); putchar('\n'); break;
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
  int opt;
  int D_flag = 0; // -D Debug
  int m_flag = 0; // -m Memory
  char *m_value = NULL;
  int t_flag = 0; // -t Text
  char *t_value = NULL;
  int f_flag = 0; // -f File
  char *f_value = NULL;

  while ((opt = getopt(argc, argv, "Dm:t:f:")) != -1) {
    switch (opt) {
      case 'D':
        D_flag = 1;
        break;
      case 'm':
        m_flag = 1;
        m_value = optarg;
        break;
      case 't':
        t_flag = 1;
        t_value = optarg;
        break;
      case 'f':
        f_flag = 1;
        f_value = optarg;
        break;
      case '?':
        fprintf(stderr, "Unknown option: -%c\n", optopt);
        return 1;
      default:
        return 1;
    }
  }

  size_t mem_size = atoi(m_value);

  if (t_flag && f_flag) {
    fprintf(stderr, "-t -f conflict");
    return 1;
  }

  char *code;
  if (t_flag) {
    code = t_value;
  } else {
    FILE *file = fopen(f_value, "r");
    if (!file) {
      perror("File open error");
      return 1;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    code = malloc(size + 1);
    if (!code) {
      fclose(file);
      fprintf(stderr, "Memory allocation error\n");
      return 1;
    }

    fread(code, 1, size, file);
    code[size] = '\0';
    fclose(file);
  }

  return run(code, mem_size, D_flag);
}
