#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

// misc
static const char* sys_msg();
static void err_msg(const char* fmt, ...);
static char* loadfile(const char* filename);

// token
typedef enum token_e {
  LCOMMENT = '{',
  RCOMMENT = '}',
  LCODE = '[',
  RCODE = ']',
  VARADR = 256,
  VALUE = 257,
  CHAR = 258,
  CHARPREDICT = '\'',
  ASSIGN = ':',
  RVAL = ';',
  APPLY = '!',
  PLUS = '+',
  MINUS = '-',
  MULTIPLE = '*',
  DIVIDE = '/',
  NEGATE = '_',
  ISEQUAL = '=',
  ISGREATER = '>',
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
  NEWLINE = '\n',
  __TOKEN_BOUND__ = 300
} token_e;
typedef struct token_t {
  token_e type;
  const char* data;
  size_t size;

  const char* head;
  int line;
} token_t;
static void set_token(token_t* token, const token_e type, const char* data, const size_t size,
               const char*head, const int line);
static void token_err(const token_t* token);

// type
typedef enum type_e {
  VARADR_TYPE,
  VALUE_TYPE,
  CODE_TYPE,
  __TYPE_BOUND__
} type_e;
static const char* strtype(const type_e type);
typedef enum boolean_e {
  TRUE = -1,
  FALSE = 0,
  __BOOLEAN_BOUND__
} boolean_e;
typedef struct type_t {
  type_e type;
  union {
    int value;
    struct type_t* varadr;
    struct {
      token_t* first;
      token_t* last;
    } code;
  } data;
} type_t;
static void type_err(const type_t* data, const type_e type);
static type_t* tnew();
static type_t* tnew_value(int value);
static type_t* tnew_varadr(type_t* varadr);
static type_t* tnew_code(token_t* first, token_t* last);
static void tfree(type_t* type);
static type_t* tcopy(const type_t* from, type_t* to);

// global stack
typedef struct stack_t {
  type_t* data;
  struct stack_t* to;
} stack_t;
static stack_t* g_top = NULL;
static int spush(type_t* data);
static type_t* spop();
static int sisempty();
static void sclear();

// global 
#define VARADDR_SIZE 26
static type_t g_varadr[VARADDR_SIZE];
static void varadr_init();

// lexer
static int lexer(char* foo, token_t** tokens, size_t* size);

// parser
typedef int isok_i(token_t*);
typedef token_t* action_i(token_t*, token_t*);
static token_t* parse_linear(token_t* first, token_t* last, isok_i* isok, action_i* action);
static token_t* parse_tree(token_t* first, token_t* last, int open, int close, action_i* action);
static int parse(token_t* first, token_t* last);

// isok
static int pass(token_t* token);
static int is_not_quote(token_t* token);

// action
static token_t* do_nothing(token_t* first, token_t* last);

static token_t* do_code(token_t* first, token_t* last);
static token_t* do_varadr(token_t* first, token_t* last);
static token_t* do_value(token_t* first, token_t* last);
static token_t* do_char(token_t* first, token_t* last);

static token_t* do_assign(token_t* first, token_t* last);
static token_t* do_rval(token_t* first, token_t* last);

static token_t* do_apply(token_t* first, token_t* last);

static token_t* do_binary(token_t* first, token_t* last);

static token_t* do_unary(token_t* first, token_t* last);

static token_t* do_duplicate(token_t* first, token_t* last);
static token_t* do_delete(token_t* first, token_t* last);
static token_t* do_swap(token_t* first, token_t* last);
static token_t* do_rot(token_t* first, token_t* last);

static token_t* do_if(token_t* first, token_t* last);
static token_t* do_while(token_t* first, token_t* last);

static token_t* do_toint(token_t* first, token_t* last);
static token_t* do_quote(token_t* first, token_t* last);
static token_t* do_tochar(token_t* first, token_t* last);

static token_t* do_getc(token_t* first, token_t* last);

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

  varadr_init();
  if (parse(tokens, tokens+size) != 0) {
    err_msg("interpret failed");
    goto err_3;
  }

  if (!sisempty()) {
    err_msg("stack is not empty");
    goto err_3;
  }

  free(tokens);
  free(foo);
  return 0;
err_3:
    sclear();
err_2:
  free(tokens);
err_1:
  free(foo);
err_0:
  return -1;
}

static const char* sys_msg()
{
  return strerror(errno);
}

static void err_msg(const char* fmt, ...)
{
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "\e[31m");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\e[0m\n");
  va_end(ap);
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

static const char* strtype(const type_e type)
{
  static const char* strs[] = {
    "varadr",
    "value",
    "function",
    "__BOUND__"
  };
  return strs[type];
}

static void type_err(const type_t* data, const type_e type)
{
  err_msg("expect %s not %s", strtype(type), strtype(data->type));
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

static void token_err(const token_t* token)
{
  char prefix[BUFSIZ];
  sprintf(prefix, "%d:%d:", token->line, token->data-token->head+1);
  err_msg("%s from here", prefix);

  char foo[BUFSIZ];
  int len = token->data-token->head;
  snprintf(foo, len+token->size+1, "%s", token->head);
  err_msg("%s %s", prefix, foo);

  for (int i = 0; i < len; i++)
    foo[i] = isspace(token->head[i])? token->head[i]: ' ';
  foo[len] = '^';
  foo[len+1] = '\0';
  err_msg("%s %s", prefix, foo);
}

static type_t* tnew()
{
  type_t* bud = malloc(sizeof(type_t));
  if (bud == NULL) {
    err_msg(sys_msg());
    goto err_0;
  }
  return bud;
err_0:
  return NULL;
}

static type_t* tnew_value(int value)
{
  type_t* bud = tnew();
  if (bud == NULL)
    goto err_0;

  bud->type = VALUE_TYPE;
  bud->data.value = value;
  return bud;
err_0:
  return NULL;
}

static type_t* tnew_varadr(type_t* varadr)
{
  type_t* bud = tnew();
  if (bud == NULL)
    goto err_0;

  bud->type = VARADR_TYPE;
  bud->data.varadr = varadr;
  return bud;
err_0:
  return NULL;
}

static type_t* tnew_code(token_t* first, token_t* last)
{
  type_t* bud = tnew();
  if (bud == NULL)
    goto err_0;

  bud->type = CODE_TYPE;
  bud->data.code.first = first;
  bud->data.code.last = last;
  return bud;
err_0:
  return NULL;
}

static void tfree(type_t* type)
{
  free(type);
}

static type_t* tcopy(const type_t* from, type_t* to)
{
  if (to == NULL) {
    to = malloc(sizeof(type_t));
    if (to == NULL) {
      err_msg(sys_msg());
      goto err_0;
    }
  }
  return memcpy(to, from, sizeof(type_t));
err_0:
  return NULL;
}

static int spush(type_t* data)
{
  stack_t* bud = malloc(sizeof(stack_t));
  if (bud == NULL) {
    err_msg(sys_msg());
    goto err_0;
  }

  bud->data = data;
  bud->to = g_top;
  g_top = bud;
  return 0;
err_0:
  return -1;
}

static type_t* spop()
{
  if (sisempty()) {
    err_msg("stack underflow");
    goto err_0;
  }

  type_t* data = g_top->data;
  stack_t* old = g_top;
  g_top = g_top->to;
  free(old);
  return data;
err_0:
  return NULL;
}

static int sisempty()
{
  return g_top == NULL;
}

static void sclear()
{
  while (!sisempty()) {
    type_t* data = spop();
    err_msg("pop %s", strtype(data->type));
    tfree(data);
  }
}

static void varadr_init()
{
  g_varadr[0].type = VALUE_TYPE;
  g_varadr[0].data.value = 0;
  for (int i = 1; i < VARADDR_SIZE; i++)
    g_varadr[i].type = __TYPE_BOUND__;
}

static int lexer(char* foo, token_t** tokens, size_t* size)
{
  token_t* bud = calloc(strlen(foo)+1, sizeof(token_t));
  if (bud == NULL) {
    err_msg(sys_msg());
    goto err_0;
  }

  char* head = foo;
  int line = 1;
  size_t size_ = 0;
  while (*foo)
    if (isspace(*foo)) {
      if (*foo == NEWLINE) {
        head = foo+1;
        line++;
      }
      set_token(bud+size_++, *foo, foo, 1, head, line);
      foo++;
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
    else if (*foo == CHARPREDICT) {
      foo++;
      set_token(bud+size_++, CHAR, foo, 1, head, line);
      if (*foo)
        foo++;
    }
    else {
      set_token(bud+size_++, *foo, foo, 1, head, line);
      foo++;
    }

  set_token(bud+size_, __TOKEN_BOUND__, foo, 0, head, line);
  *tokens = bud;
  *size = size_;
  return 0;
err_0:
  return -1;
}

static token_t* parse_linear(token_t* first, token_t* last, isok_i* isok, action_i* action)
{
  token_t* it = first;
  while (it < last && isok(it))
    it++;
  return action(first, it);
}

static token_t* parse_tree_aux(token_t* first, token_t* last, int down, int up)
{
  token_t* it = first;
  while (it < last && it->type != up)
    if (it->type == down) {
      it = parse_tree_aux(it+1, last, down, up);
      if (it == NULL)
        return NULL;
    }
    else 
      it++;

  if (it->type != up) {
    err_msg("missing matched %c", up);
    token_err(first-1);
    return NULL;
  }
  return it+1;
}

static token_t* parse_tree(token_t* first, token_t* last, int down, int up, action_i* action)
{
  last = parse_tree_aux(first, last, down, up);
  if (last == NULL)
    return NULL;

  return action(first, last-1);
}

static int parse(token_t* first, token_t* last)
{
  while (first < last) {
    if (isspace(first->type)) {
      first++;
      continue;
    }

    token_t* save = first;;
    switch (first->type) {
      case LCOMMENT: {
        first = parse_tree(first+1, last, LCOMMENT, RCOMMENT, do_nothing);
        break;
      }
      case RCOMMENT: {
        err_msg("missing match {");
        first = NULL;
        break;
      }
      case LCODE: {
        first = parse_tree(first+1, last, LCODE, RCODE, do_code);
        break;
      }
      case RCODE: {
        err_msg("missing match [");
        first = NULL;
        break;
      }
      case VARADR: {
        first = parse_linear(first, first+1, pass, do_varadr);
        break;
      }
      case VALUE: {
        first = parse_linear(first, first+1, pass, do_value);
        break;
      }
      case CHAR: {
        first = parse_linear(first, first+1, pass, do_char);
        break;
      }
      case ASSIGN: {
        first = parse_linear(first, first+1, pass, do_assign);
        break;
      }
      case RVAL: {
        first = parse_linear(first, first+1, pass, do_rval);
        break;
      }
      case APPLY: {
        first = parse_linear(first, first+1, pass, do_apply);
        break;
      }
      case PLUS:
      case MINUS:
      case MULTIPLE:
      case DIVIDE:
      case ISEQUAL:
      case ISGREATER:
      case AND:
      case OR: {
        first = parse_linear(first, first+1, pass, do_binary);
        break;
      }
      case NEGATE:
      case NOT: {
        first = parse_linear(first, first+1, pass, do_unary);
        break;
      }
      case DUPLICATE: {
        first = parse_linear(first, first+1, pass, do_duplicate);
        break;
      }
      case DELETE: {
        first = parse_linear(first, first+1, pass, do_delete);
        break;
      }
      case SWAP: {
        first = parse_linear(first, first+1, pass, do_swap);
        break;
      }
      case ROT: {
        first = parse_linear(first, first+1, pass, do_rot);
        break;
      }
      case IF: {
        first = parse_linear(first, first+1, pass, do_if);
        break;
      }
      case WHILE: {
        first = parse_linear(first, first+1, pass, do_while);
        break;
      }
      case TOINT: {
        first = parse_linear(first, first+1, pass, do_toint);
        break;
      }
      case QUOTE: {
        first = parse_linear(first+1, last, is_not_quote, do_quote);
        break;
      }
      case TOCHAR: {
        first = parse_linear(first, first+1, pass, do_tochar);
        break;
      }
      case GETC: {
        first = parse_linear(first, first+1, pass, do_getc);
        break;
      }
      default: {
        err_msg("unknown token");
        first = NULL;
      }
    }

    if (first == NULL) {
      token_err(save);
      goto err_0;
    }
  }
  return 0;
err_0:
  return -1;
}

static int pass(token_t* token)
{
  return 1;
}

static token_t* do_nothing(token_t* first, token_t* last)
{
  return last+1;
}

static token_t* do_code(token_t* first, token_t* last)
{
  type_t* data = tnew_code(first, last);
  if (data == NULL) 
    goto err_0;

  if (spush(data) != 0)
    goto err_1;
  return last+1;
err_1:
  tfree(data);
err_0:
  return NULL;
}

static token_t* do_varadr(token_t* first, token_t* last)
{
  type_t* data = tnew_varadr(g_varadr+first->data[0]-'a');
  if (data == NULL)
    goto err_0;

  if (spush(data) != 0)
    goto err_1;
  return last;
err_1:
  tfree(data);
err_0:
  return NULL;
}

static token_t* do_value(token_t* first, token_t* last)
{
  int value = 0;
  for (int i = 0; i < first->size; i++)
    value = value*10+first->data[i]-'0';

  type_t* data = tnew_value(value);
  if (data == NULL)
    goto err_0;

  if (spush(data) != 0)
    goto err_1;
  return last;
err_1:
  tfree(data);
err_0:
  return NULL;
}

static token_t* do_char(token_t* first, token_t* last)
{
  type_t* data = tnew_value(first->data[0]);
  if (data == NULL)
    goto err_0;

  if (spush(data) != 0)
    goto err_1;
  return last;
err_1:
  tfree(data);
err_0:
  return NULL;
}

static token_t* do_assign(token_t* first, token_t* last)
{
  type_t* lval = spop();
  if (lval == NULL)
    goto err_0;

  if (lval->type != VARADR_TYPE) {
    type_err(lval, VARADR_TYPE);
    goto err_1;
  }

  type_t* rval = spop();
  if (rval == NULL)
    goto err_1;

  tcopy(rval, lval->data.varadr);

  tfree(rval);
  tfree(lval);
  return last;
err_1:
  tfree(lval);
err_0:
  return NULL;
}

static token_t* do_rval(token_t* first, token_t* last)
{
  type_t* lval = spop();
  if (lval == NULL)
    goto err_0;

  if (lval->type != VARADR_TYPE) {
    type_err(lval, VARADR_TYPE);
    goto err_1;
  }

  type_t* rval = tcopy(lval->data.varadr, NULL);
  if (rval == NULL)
    goto err_1;

  if (spush(rval) != 0)
    goto err_2;

  tfree(lval);
  return last;
err_2:
  tfree(rval);
err_1:
  tfree(lval);
err_0:
  return NULL;
}

static token_t* do_apply(token_t* first, token_t* last)
{
  type_t* data = spop();
  if (data == NULL)
    goto err_0;

  if (data->type != CODE_TYPE) {
    type_err(data, CODE_TYPE);
    goto err_1;
  }

  if (parse(data->data.code.first, data->data.code.last) != 0)
    goto err_1;

  tfree(data);
  return last;
err_1:
  tfree(data);
err_0:
  return NULL;
}

static token_t* do_binary(token_t* first, token_t* last)
{
  type_t* rhs = spop();
  if (rhs == NULL)
    goto err_0;
  
  if (rhs->type != VALUE_TYPE) {
    type_err(rhs, VALUE_TYPE);
    goto err_1;
  }

  type_t* lhs = spop();
  if (lhs == NULL)
    goto err_1;

  if (lhs->type != VALUE_TYPE) {
    type_err(lhs, VALUE_TYPE);
    goto err_2;
  }

  int lhsval = lhs->data.value;
  int rhsval = rhs->data.value;
  int lvalval;
  switch (first->type) {
    case PLUS: {
      lvalval = lhsval+rhsval;
      break;
    }
    case MINUS: {
      lvalval = lhsval-rhsval;
      break;
    }
    case MULTIPLE: {
      lvalval = lhsval*rhsval;
      break;
    }
    case DIVIDE: {
      if (rhsval == 0) {
        err_msg("attempt to divide 0");
        goto err_2;
      }

      lvalval = lhsval/rhsval;
      break;
    }
    case ISEQUAL: {
      lvalval = lhsval == rhsval? TRUE: FALSE;
      break;
    }
    case ISGREATER: {
      lvalval = lhsval > rhsval? TRUE: FALSE;
      break;
    }
    case AND: {
      lvalval = lhsval == TRUE && rhsval == TRUE? TRUE: FALSE;
      break;
    }
    case OR: {
      lvalval = lhsval == TRUE || rhsval == TRUE? TRUE: FALSE;
      break;
    }
    default: {
      break;
    }
  }

  type_t* lval = tnew_value(lvalval);
  if (lval == NULL)
    goto err_2;

  if (spush(lval) != 0)
    goto err_3;

  tfree(lhs);
  tfree(rhs);
  return last;
err_3:
  tfree(lval);
err_2:
  tfree(lhs);
err_1:
  tfree(rhs);
err_0:
  return NULL;
}

static token_t* do_unary(token_t* first, token_t* last)
{
  type_t* rval = spop();
  if (rval == NULL)
    goto err_0;

  if (rval->type != VALUE_TYPE) {
    type_err(rval, VALUE_TYPE);
    goto err_1;
  }

  int rvalval = rval->data.value;
  switch (first->type) {
      case NEGATE: {
        rvalval = -rvalval;
        break;
      }
      case NOT: {
        rvalval = rvalval == FALSE? TRUE: FALSE;
        break;
      }
      default: {
        break;
      }
  }

  type_t* lval = tnew_value(rvalval);
  if (lval == NULL)
    goto err_1;

  if (spush(lval) != 0)
    goto err_2;

  tfree(rval);
  return last;
err_2:
  tfree(lval);
err_1:
  tfree(rval);
err_0:
  return NULL;
}

static token_t* do_duplicate(token_t* first, token_t* last)
{
  type_t* from = spop();
  if (from == NULL)
    goto err_0;

  type_t* to = tcopy(from, NULL);
  if (to == NULL)
    goto err_1;

  if (spush(from) != 0 || spush(to) != 0)
    goto err_2;

  return last;
err_2:
  tfree(to);
err_1:
  tfree(from);
err_0:
  return NULL;
}

static token_t* do_delete(token_t* first, token_t* last)
{
  type_t* old = spop();
  if (old == NULL)
    goto err_0;

  tfree(old);
  return last;
err_0:
  return NULL;
}

static token_t* do_swap(token_t* first, token_t* last)
{
  type_t* rhs = spop();
  if (rhs == NULL)
    goto err_0;

  type_t* lhs = spop();
  if (lhs == NULL)
    goto err_1;

  if (spush(rhs) != 0 || spush(lhs) != 0)
    goto err_2;

  return last;
err_2:
  tfree(lhs);
err_1:
  tfree(rhs);
err_0:
  return NULL;
}

static token_t* do_rot(token_t* first, token_t* last)
{
  type_t* rhs = spop();
  if (rhs == NULL)
    goto err_0;

  type_t* mhs = spop();
  if (mhs == NULL)
    goto err_1;

  type_t* lhs = spop();
  if (lhs == NULL)
    goto err_2;

  if (spush(mhs) != 0 || spush(rhs) != 0 || spush(lhs) != 0)
    goto err_3;
  return last;
err_3:
  tfree(lhs);
err_2:
  tfree(mhs);
err_1:
  tfree(rhs);
err_0:
  return NULL;
}

static token_t* do_if(token_t* first, token_t* last)
{
  type_t* rhs = spop();
  if (rhs == NULL)
    goto err_0;

  if (rhs->type != CODE_TYPE) {
    type_err(rhs, CODE_TYPE);
    goto err_1;
  }

  type_t* lhs = spop();
  if (lhs == NULL)
    goto err_1;

  if (lhs->type != VALUE_TYPE) {
    type_err(lhs, VALUE_TYPE);
    goto err_2;
  }

  if (lhs->data.value != FALSE
      && parse(rhs->data.code.first, rhs->data.code.last) != 0)
    goto err_2;

  tfree(lhs);
  tfree(rhs);
  return last;
err_2:
  tfree(lhs);
err_1:
  tfree(rhs);
err_0:
  return NULL;
}

static token_t* do_while(token_t* first, token_t* last)
{
  type_t* rhs = spop();
  if (rhs == NULL)
    goto err_0;

  if (rhs->type != CODE_TYPE) {
    type_err(rhs, CODE_TYPE);
    goto err_1;
  }

  type_t* lhs = spop();
  if (lhs == NULL)
    goto err_1;

  if (lhs->type != CODE_TYPE) {
    type_err(lhs, CODE_TYPE);
    goto err_2;
  }

  type_t* benchmark;
  while (1) {
    if (parse(lhs->data.code.first, lhs->data.code.last) != 0)
      goto err_2;

    benchmark = spop();
    if (benchmark == NULL)
      goto err_2;

    if (benchmark->type != VALUE_TYPE) {
      type_err(benchmark, VALUE_TYPE);
      goto err_3;
    }

    if (benchmark->data.value == FALSE) {
      tfree(benchmark);
      break;
    }

    if (parse(rhs->data.code.first, rhs->data.code.last) != 0)
      goto err_3;

    tfree(benchmark);
  }

  tfree(lhs);
  tfree(rhs);
  return last;
err_3:
  tfree(benchmark);
err_2:
  tfree(lhs);
err_1:
  tfree(rhs);
err_0:
  return NULL;
}

static token_t* do_toint(token_t* first, token_t* last)
{
  type_t* data = spop();
  if (data == NULL)
    goto err_0;

  if (data->type != VALUE_TYPE) {
    type_err(data, VALUE_TYPE);
    goto err_1;
  }

  printf("%d", data->data.value);
  tfree(data);
  return last;
err_1:
  tfree(data);
err_0:
  return NULL;
}

static int is_not_quote(token_t* token)
{
  return token->type != QUOTE;
}

static token_t* do_quote(token_t* first, token_t* last)
{
  if (last->type != QUOTE) {
    err_msg("missing close \"");
    goto err_0;
  }

  while (first < last) {
    putchar(first->data[0]);
    first++;
  }
  return last+1;
err_0:
  return NULL;
}

static token_t* do_tochar(token_t* first, token_t* last)
{
  type_t* data = spop();
  if (data == NULL)
    goto err_0;

  if (data->type != VALUE_TYPE) {
    type_err(data, VALUE_TYPE);
    goto err_1;
  }

  printf("%c", data->data.value);
  tfree(data);
  return last;
err_1:
  tfree(data);
err_0:
  return NULL;
}

static token_t* do_getc(token_t* first, token_t* last)
{
  type_t* data = tnew_value(getchar());
  if (data == NULL)
    goto err_0;

  if (spush(data) != 0)
    goto err_1;
  return last;
err_1:
  tfree(data);
err_0:
  return NULL;
}
