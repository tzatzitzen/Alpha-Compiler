#ifndef _lex_token_
#define _lex_token_

enum lex_token_type {
  NO_TKN_TYPE,
  INT_TKN_TYPE,
  REAL_TKN_TYPE,
  STR_TKN_TYPE,
  ID_TKN_TYPE
};

struct lex_token {
  unsigned int line_num;
  unsigned int token_num;
  char *content;
  enum lex_token_type type;
  int intVal;
  double doubleVal;
  char *stringVal;
};

typedef struct lex_token lex_token;
#endif