#ifndef _intermediate_
#define _intermediate_

#define EXPAND_SIZE 1024
#define CURR_SIZE (total * sizeof(quad))
#define NEW_SIZE (EXPAND_SIZE * sizeof(quad) + CURR_SIZE)

#include "symtable.h"

typedef enum Iopcode {
  assign,
  add,
  sub,
  mul,
  divop,
  mod,
  tablecreate,
  tablegetelem,
  tablesetelem,
  uminus,
  jump,
  if_eq,
  if_noteq,
  if_greater,
  if_greatereq,
  if_less,
  if_lesseq,
  notop,
  orop,
  andop,
  param,
  call,
  getretval,
  funcstart,
  ret,
  funcend,
} iopcode;

typedef enum Expression_T {
  var_e,
  tableitem_e,
  programfunc_e,
  libraryfunc_e,
  arithexpr_e,
  boolexpr_e,
  assignexpr_e,
  newtable_e,
  constnum_e,
  constbool_e,
  conststring_e,
  nil_e
} expr_t;

typedef struct Expression {
  expr_t type;
  Entry_T sym;
  struct Expression *index;
  struct Expression *value;
  double numConst;
  char *strConst;
  unsigned int boolConst;
  struct Expression *next;
} expr;

typedef struct Quad {
  iopcode op;
  expr *result;
  expr *arg1;
  expr *arg2;
  unsigned int label;
  unsigned int line;
  unsigned int taddress;
} quad;

typedef struct functionCall {
  expr *elist;
  unsigned int method;
  char *name;
} fcall_t;

typedef struct ForLoop {
  unsigned int test;
  unsigned int enter;
} forloop;

typedef struct Break_Cont_List {
  quad *q;
  struct Break_Cont_List *next;
} list;

typedef struct LoopStruct {
  struct stack *breaklist;
  bool break_exists;
  struct stack *contlist;
  bool cont_exists;
  struct LoopStruct *next;
  struct LoopStruct *prev;
} loop;

typedef struct Stmt_T {
  int breaklist, contlist;
  forloop *loop;
} stmt_t;

void expand();
void emit(iopcode op, expr *result, expr *arg1, expr *arg2, unsigned int label,
          unsigned int line);
char *newtempname();
void resettemp();
Entry_T newtemp();

scopespace_t currScopespace();
unsigned int currScopespaceOffset();
void inCurrScopespaceOffset();
void enterScopespace();
void exitScopespace();

void resetFormalArgOffset();
void resetFunctionLocalOffset();
void restoreCurrScopeOffset(unsigned int n);
unsigned int nextQuadLabel();
void patchLabel(unsigned int quadNo, unsigned int label);
expr *newexpr(expr_t t);
expr *newexpr_constString(char *s);
expr *newexpr_constNum(double n);
expr *newexpr_constBool(unsigned int b);
expr *newexpr_nil();
expr *emit_iftableitem(expr *e);
bool check_arith(expr *e);
expr *lvalue_expr(Entry_T sym);
bool isValidOper(expr *L, expr *R);
const char *returnOp(iopcode op);
void printQuads();
expr *make_call(expr *lvalue, expr *elist);
expr *member_item(expr *lv, char *name);
void push(struct stack *x, unsigned int y);
int pop(struct stack *x);
int pop_and_top(struct stack *x);
unsigned int isTempExpr(expr *e);
stmt_t *make_stmt();
unsigned int newlist(unsigned int i);
unsigned int mergelist(unsigned int l1, unsigned int l2);
void patchlist(int list, int label);
void initList(struct stack **current, bool *exists);

extern quad *quads;
extern unsigned total;
extern unsigned int currQuad;
#endif