#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

typedef enum token_e {
  LCOMMENT = '{',
  RCOMMENT = '}',
  LCODE = '[',
  RCODE = ']',
  VARADR = 256,
  VALUE = 257,
  ASSIGN = ':',
  RVAL = ';',
  APPLY = '!',
  PLUS = '+',
  MINUS = '-',
  MULTIPLE = '*',
  DIVIDE = '/',
  NEGATE = '_',
  ISEQUAL = '=',
  GREATER = '>',
  AND = '&',
  OR = '|',
  NOT = '~',
  DUPLICATE = '$',
  DELETE = '%',
  SWAP = '\\',
  ROT = '@',
  IF = '?',
  WHILE = '#',
  TOINT = '.',
  QUOTE = '"',
  TOCHAR = ',',
  GETC = '^',
  __BOUND__
} token_e;

typedef struct token_t {
  token_e type;
  const char* data;
  size_t size;

  const char* head;
  int line;
} token_t;

static const char* sys_msg()
{
  return strerror(errno);
}

static void err_msg(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  va_end(ap);
}

static void set_token(token_t* token, const token_e type, const char* data, const size_t size,
               const char*head, const int line)
{
  token->type = type;
  token->data = data;
  token->size = size;
  token->head = head;
  token->line = line;
}

static void token_err(token_t* token, const char* msg)
{
  char prefix[BUFSIZ];
  sprintf(prefix, "%d:%d:", token->line, token->data-token->head+1);
  err_msg("%s %s from", prefix, msg);

  char foo[BUFSIZ];
  int len = token->data-token->head;
  snprintf(foo, len+token->size+1, "%s", token->head);
  err_msg("%s %s", prefix, foo);

  for (int i = 0; i < len; i++)
    foo[i] = ' ';
  foo[len] = '^';
  foo[len+1] = '\0';
  err_msg("%s %s", prefix, foo);
}

static char* loadfile(const char* filename)
{
  FILE* file = fopen(filename, "r");
  if (file == NULL) {
    err_msg(sys_msg());
    goto err_0;
  }

  if (fseek(file, 0, SEEK_END) == -1) {
    err_msg(sys_msg());
    goto err_1;
  }
  long size = ftell(file);
  rewind(file);

  char* foo = calloc(size+2, 1);
  if (foo == NULL) {
    err_msg(sys_msg());
    goto err_1;
  }

  if (fread(foo, 1, size, file) != size) {
    err_msg(sys_msg());
    goto err_1;
  }

  fclose(file);
  return foo;
err_1:
  fclose(file);
err_0:
  return NULL;
}

static int lexer(char* foo, token_t** tokens, size_t* size)
{
  token_t* bud = calloc(strlen(foo), sizeof(token_t));
  if (bud == NULL) {
    err_msg(sys_msg());
    goto err_0;
  }

  char* head = foo;
  int line = 1;
  size_t size_ = 0;
  while (*foo)
    if (isspace(*foo)) {
      while (*foo && isspace(*foo)) {
        if (*foo == '\n') {
          head = foo+1;
          line++;
        }
        foo++;
      }
    }
    else if (islower(*foo)) {
      set_token(bud+size_++, VARADR, foo, 1, head, line);
      foo++;
    }
    else if (isdigit(*foo)) {
      char* start = foo;
      while (*foo && isdigit(*foo))
        foo++;
      set_token(bud+size_++, VALUE, start, foo-start, head, line);
    }
    else
      set_token(bud+size_++, *foo, foo++, 1, head, line);

  *tokens = bud;
  *size = size_;
  return 0;
err_1:
  free(bud);
err_0:
  return -1;
}

static int parse(token_t* first, token_t* last)
{
  while (first < last)
    switch (first->type) {
      case LCOMMENT: {
        break;
      }
      case RCOMMENT: {
        break;
      }
      case LCODE: {
        break;
      }
      case RCODE: {
        break;
      }
      case VARADR: {
        break;
      }
      case VALUE: {
        break;
      }
      case ASSIGN: {
        break;
      }
      case RVAL: {
        break;
      }
      case APPLY: {
        break;
      }
      case PLUS: {
        break;
      }
      case MINUS: {
        break;
      }
      case MULTIPLE: {
        break;
      }
      case DIVIDE: {
        break;
      }
      case NEGATE: {
        break;
      }
      case ISEQUAL: {
        break;
      }
      case GREATER: {
        break;
      }
      case AND: {
        break;
      }
      case OR: {
        break;
      }
      case NOT: {
        break;
      }
      case DUPLICATE: {
        break;
      }
      case DELETE: {
        break;
      }
      case SWAP: {
        break;
      }
      case ROT: {
        break;
      }
      case IF: {
        break;
      }
      case WHILE: {
        break;
      }
      case TOINT: {
        break;
      }
      case QUOTE: {
        break;
      }
      case TOCHAR: {
        break;
      }
      case GETC: {
        break;
      }
      default: {
      }
    }
  return 0;
}

int main(int argc, char* argv[])
{
  char* foo = loadfile(argv[1]);
  if (foo == NULL) {
    err_msg("load file failed");
    goto err_0;
  }

  token_t* tokens;
  size_t size;
  if (lexer(foo, &tokens, &size) != 0) {
    err_msg("lexer failed");
    goto err_1;
  }

  for (int i = 0; i < size; i++) {
    char foo[BUFSIZ];
    snprintf(foo, tokens[i].size+1, "%s", tokens[i].data);
    printf("{%d, '%s'}\n", tokens[i].size, foo);
    token_err(tokens+i, "test");
  }

  printf("size=%d\n", size);

  free(tokens);
  free(foo);
  return 0;
err_2:
  free(tokens);
err_1:
  free(foo);
err_0:
  return -1;
}
