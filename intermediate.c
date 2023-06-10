#include "intermediate.h"

quad *quads = (quad *)0;
unsigned total = 0;
unsigned int currQuad = 0;

void expand() {
  assert(total == currQuad);
  quad *p = (quad *)malloc(NEW_SIZE);
  if (quads) {
    memcpy(p, quads, CURR_SIZE);
    free(quads);
  }
  quads = p;
  total += EXPAND_SIZE;
}

void emit(iopcode op, expr *result, expr *arg1, expr *arg2, unsigned int label,
          unsigned int line) {
  if (currQuad == total) {
    expand();
  }
  quad *p = quads + currQuad++;
  p->op = op;
  p->arg1 = arg1;
  p->arg2 = arg2;
  p->result = result;
  p->label = label;
  p->line = line;
}

expr *emit_iftableitem(expr *e) {
  if (e->type != tableitem_e) {
    return e;
  } else {
    expr *result = newexpr(var_e);
    result->sym = newtemp();
    emit(tablegetelem, result, e, e->index, nextQuadLabel(), yylineno);
    return result;
  }
}

unsigned int tempcounter = 0;
char *newtempname() {
  char *tmp = (char *)malloc(20 * sizeof(char));
  sprintf(tmp, "%s%d", "_t", tempcounter);
  return tmp;
}
void resettemp() { tempcounter = 0; }

Entry_T newtemp() {
  char *name = newtempname();
  tempcounter++;
  Entry_T sym = ScopeLookup(name, curr_scope);
  if (sym == NULL) {
    SymbolTable_insert(name, curr_scope, yylineno,
                       curr_scope == 0 ? ST_GLOBAL : ST_LOCAL, currScopespace(),
                       currScopespaceOffset());
    sym = ScopeLookup(name, curr_scope);
  }
  return sym;
}

unsigned int isTempName(const char *s) { return !strncmp(s, "_t", 2); }

unsigned int isTempExpr(expr *e) { return e->sym && isTempName(e->sym->name); }

expr *make_call(expr *lvalue, expr *elist) {
  expr *func = emit_iftableitem(lvalue);
  while (elist != NULL) {
    emit(param, elist, NULL, NULL, nextQuadLabel(), yylineno);
    elist = elist->next;
  }
  emit(call, func, NULL, NULL, nextQuadLabel(), yylineno);
  expr *result = newexpr(var_e);
  result->sym = newtemp();
  emit(getretval, result, NULL, NULL, nextQuadLabel(), yylineno);
  return result;
}

stmt_t *make_stmt() {
  stmt_t *s = malloc(sizeof(struct Stmt_T));
  assert(s);
  s->loop = NULL;
  s->breaklist = s->contlist = 0;
  return s;
}

unsigned int newlist(unsigned int i) {
  quads[i].label = 0;
  return i;
}

unsigned int mergelist(unsigned int l1, unsigned int l2) {
  if (!l1) {
    return l2;
  } else if (!l2) {
    return l1;
  } else {
    int i = l1;
    while (quads[i].label && quads[i].label != i)
      i = quads[i].label;
    quads[i].label = l2;
    return l1;
  }
}

void patchlist(int list, int label) {
  while (list) {
    int next = quads[list].label;
    quads[list].label = label;
    list = next;
  }
}

unsigned int nextQuadLabel() { return currQuad; }

void patchLabel(unsigned int quadNo, unsigned int label) {
  assert(quadNo < currQuad);
  quads[quadNo].label = label;
}

unsigned int programVarOffset = 0;
unsigned int functionLocalOffset = 0;
unsigned int formalArgOffset = 0;
unsigned int scopespaceCounter = 1;

scopespace_t currScopespace() {
  if (scopespaceCounter == 1) {
    return programvar;
  } else if (scopespaceCounter % 2 == 0) {
    return formalarg;
  } else {
    return functionlocal;
  }
}

unsigned int currScopespaceOffset() {
  switch (currScopespace()) {
  case programvar:
    return programVarOffset;
  case functionlocal:
    return functionLocalOffset;
  case formalarg:
    return formalArgOffset;
  default:
    assert(0);
  }
}

void inCurrScopespaceOffset() {
  switch (currScopespace()) {
  case programvar:
    ++programVarOffset;
    break;
  case functionlocal:
    ++functionLocalOffset;
    break;
  case formalarg:
    ++formalArgOffset;
    break;
  default:
    assert(0);
  }
}

void enterScopespace() { ++scopespaceCounter; }

void exitScopespace() {
  assert(scopespaceCounter > 1);
  --scopespaceCounter;
}

void resetFormalArgOffset() { formalArgOffset = 0; }

void resetFunctionLocalOffset() { functionLocalOffset = 0; }

void restoreCurrScopeOffset(unsigned int n) {
  switch (currScopespace()) {
  case programvar:
    programVarOffset = n;
    break;
  case functionlocal:
    functionLocalOffset = n;
    break;
  case formalarg:
    formalArgOffset = n;
    break;
  default:
    assert(0);
  }
}

expr *newexpr(expr_t t) {
  expr *e = (expr *)malloc(sizeof(expr));
  memset(e, 0, sizeof(expr));
  e->type = t;
  return e;
}

expr *newexpr_constString(char *s) {
  expr *e = newexpr(conststring_e);
  e->strConst = strdup(s);
  return e;
}

expr *newexpr_constNum(double n) {
  expr *e = newexpr(constnum_e);
  e->numConst = n;
  return e;
}

expr *newexpr_constBool(unsigned int b) {
  expr *e = newexpr(constbool_e);
  e->boolConst = b;
  return e;
}

expr *newexpr_nil() {
  expr *e = newexpr(nil_e);
  e->numConst = 0;
  return e;
}

expr *lvalue_expr(Entry_T sym) {
  assert(sym);
  expr *e = (expr *)malloc(sizeof(expr));
  memset(e, 0, sizeof(expr));

  e->next = (expr *)0;
  e->sym = sym;

  switch (sym->type) {
  case ST_LOCAL:
    e->type = var_e;
    break;
  case ST_GLOBAL:
    e->type = var_e;
    break;
  case ST_FORMAL:
    e->type = var_e;
    break;
  case ST_LIBFUNC:
    e->type = libraryfunc_e;
    break;
  case ST_USERFUNC:
    e->type = programfunc_e;
    break;
  default:
    assert(0);
  }

  return e;
}

bool check_arith(expr *e) {

  if (e->type == constbool_e || e->type == conststring_e || e->type == nil_e ||
      e->type == newtable_e || e->type == programfunc_e ||
      e->type == libraryfunc_e || e->type == boolexpr_e) {
    printf("Illegal expr used\n");
    return false;
  }
  return true;
}

// bool isValidOper(expr *L, expr *R){
//     if(check_arith(L->type) || check_arith(R->type)){
//       return true;
//     }
//       return false;
// }

const char *returnOp(iopcode op) {
  switch (op) {
  case assign:
    return "assign";
  case add:
    return "add";
  case sub:
    return "sub";
  case mul:
    return "mul";
  case divop:
    return "div";
  case mod:
    return "mod";
  case uminus:
    return "uminus";
  case andop:
    return "and";
  case orop:
    return "or";
  case notop:
    return "not";
  case if_eq:
    return "if_eq";
  case if_noteq:
    return "if_noteq";
  case if_lesseq:
    return "if_lesseq";
  case if_greatereq:
    return "if_greatereq";
  case if_less:
    return "if_less";
  case if_greater:
    return "if_greater";
  case call:
    return "call";
  case param:
    return "param";
  case ret:
    return "ret";
  case getretval:
    return "getretval";
  case funcstart:
    return "funcstart";
  case funcend:
    return "funcend";
  case tablecreate:
    return "tablecreate";
  case tablegetelem:
    return "tablegetelem";
  case tablesetelem:
    return "tablesetelem";
  case jump:
    return "jump";
  default:
    return "unknown";
  }
}

void removeTrailingZeros(char *str) {
  int length = strlen(str);

  if (length == 0)
    return;

  int i = length - 1;

  // Scan the string from the end
  while (i >= 0 && str[i] == '0') {
    // Replace zeros with null characters
    str[i] = '\0';
    i--;
  }

  // If the last character is a decimal point, remove it as well
  if (str[i] == '.')
    str[i] = '\0';
}

expr *member_item(expr *lv, char *name) {
  lv = emit_iftableitem(lv);
  expr *ti = newexpr(tableitem_e);
  ti->sym = lv->sym;
  ti->index = newexpr_constString(name);
  return ti;
}

void initList(struct stack **current, bool *exists) {
  *current = malloc(sizeof(struct stack));
  // starting from 0, cause -1 doesnt work with break/continue labels
  (*current)->top = 0;
  *exists = true;
}

void push(struct stack *x, unsigned int y) {
  if (x->top == STACK_MAX_SIZE) {
    printf("Error, stackoverflow, cannot push element, stack is full\n");
    return;
  }
  printf("top:%d \n\n", x->top);
  x->top++;
  x->data[x->top] = y;
  printf("pushed: %d\n\n", y);
}

int pop(struct stack *x) {
  if (x->top == 0) {
    printf("Cannot pop element from empty stack\n");
    return -1;
  }
  int y = x->data[x->top];
  x->top--;
  printf("poped: %d\n\n", y);
  return y;
}

char *getArgToString(expr *arg) {
  char *buffer = malloc(20 * sizeof(char));
  if (buffer != NULL && arg != NULL) {
    if (arg->type == constnum_e) {
      sprintf(buffer, "%f", arg->numConst);
      removeTrailingZeros(buffer);
    } else if (arg->type == constbool_e) {
      buffer = arg->boolConst == 0 ? "false" : "true";
    } else if (arg->type == conststring_e) {
      strcpy(buffer, arg->strConst);
    } else {
      strcpy(buffer, arg->sym->name);
    }
  }
  return buffer;
}

void printJump(FILE *file, int i) {
  fprintf(file, "%d:\t%-20s\t\t\t\t\t\t\t%d\n", i + 1, returnOp(quads[i].op),
          quads[i].label + 1);
  printf("%d:\t%-20s\t\t\t\t\t\t\t%d\n", i + 1, returnOp(quads[i].op),
         quads[i].label + 1);
}

void printAssign(FILE *file, int i) {
  if (quads[i].arg1 == NULL) {
    fprintf(file, "%d:\t%-20s\t%s\n", i + 1, returnOp(quads[i].op),
            quads[i].result->sym->name);
    printf("%d:\t%-20s\t%s\n", i + 1, returnOp(quads[i].op),
           quads[i].result->sym->name);
  } else {
    fprintf(file, "%d:\t%-20s\t%s\t\t%s\n", i + 1, returnOp(quads[i].op),
            quads[i].result->sym->name, getArgToString(quads[i].arg1));
    printf("%d:\t%-20s\t%s\t\t%s\n", i + 1, returnOp(quads[i].op),
           quads[i].result->sym->name, getArgToString(quads[i].arg1));
  }
}

void printArithm(FILE *file, int i) {
  fprintf(file, "%d:\t%-20s\t%s\t\t%s\t\t%s\n", i + 1, returnOp(quads[i].op),
          quads[i].result->sym->name, getArgToString(quads[i].arg1),
          getArgToString(quads[i].arg2));
  printf("%d:\t%-20s\t%s\t\t%s\t\t%s\n", i + 1, returnOp(quads[i].op),
         quads[i].result->sym->name, getArgToString(quads[i].arg1),
         getArgToString(quads[i].arg2));
}

void printBool(FILE *file, int i) {
  fprintf(file, "%d:\t%-20s\t\t\t%s\t\t%s\t\t%d\n", i + 1,
          returnOp(quads[i].op), getArgToString(quads[i].arg1),
          getArgToString(quads[i].arg2), quads[i].label + 1);
  printf("%d:\t%-20s\t\t\t%s\t\t%s\t\t%d\n", i + 1, returnOp(quads[i].op),
         getArgToString(quads[i].arg1), getArgToString(quads[i].arg2),
         quads[i].label + 1);
}

void printMember(FILE *file, int i) {
  fprintf(file, "%d:\t%-20s\t%s\t\t%s\t\t%s\n", i + 1, returnOp(quads[i].op),
          quads[i].result->sym->name, getArgToString(quads[i].arg1),
          getArgToString(quads[i].arg2));
  printf("%d:\t%-20s\t%s\t\t%s\t\t%s\n", i + 1, returnOp(quads[i].op),
         quads[i].result->sym->name, getArgToString(quads[i].arg1),
         getArgToString(quads[i].arg2));
}

void printUminus(FILE *file, int i) {
  fprintf(file, "%d:\t%-20s\t%s\t\t%s\n", i + 1, returnOp(quads[i].op),
          quads[i].result->sym->name, getArgToString(quads[i].arg1));
  printf("%d:\t%-20s\t%s\t\t%s\n", i + 1, returnOp(quads[i].op),
         quads[i].result->sym->name, getArgToString(quads[i].arg1));
}

void printResult(FILE *file, int i) {
  fprintf(file, "%d:\t%-20s\t%s\n", i + 1, returnOp(quads[i].op),
          getArgToString(quads[i].result));
  printf("%d:\t%-20s\t%s\n", i + 1, returnOp(quads[i].op),
         getArgToString(quads[i].result));
}

void printQuads() {
  FILE *file = fopen("quads.txt", "w");
  if (file == NULL) {
    printf("Error opening file!\n");
    exit(1);
  }

  fprintf(file, "\nquad#\topcode\t\t\tresult\t\targ1\t\targ2\t\tlabel\n");
  printf("\nquad#\topcode\t\t\tresult\t\targ1\t\targ2\t\tlabel\n");

  fprintf(file, "-------------------------------------------------------------"
                "------------------------\n");
  printf("-------------------------------------------------------------"
         "------------------------\n");

  for (int i = 0; i < nextQuadLabel(); i++) {
    // Add your conditionals here and call the respective function..
    if (quads[i].op == add || quads[i].op == sub || quads[i].op == mul ||
        quads[i].op == divop || quads[i].op == mod) {
      printArithm(file, i);
    } else if (quads[i].op == if_eq || quads[i].op == if_noteq ||
               quads[i].op == if_lesseq || quads[i].op == if_greatereq ||
               quads[i].op == if_less || quads[i].op == if_greater) {
      printBool(file, i);
    } else if (quads[i].op == assign) {
      printAssign(file, i);
    } else if (quads[i].op == jump) {
      printJump(file, i);
    } else if (quads[i].op == uminus) {
      printUminus(file, i);
    } else if (quads[i].op == tablesetelem || quads[i].op == tablegetelem) {
      printMember(file, i);
    } else if (quads[i].op == tablecreate || quads[i].op == funcstart ||
               quads[i].op == funcend || quads[i].op == param ||
               quads[i].op == call || quads[i].op == getretval ||
               quads[i].op == ret) {
      printResult(file, i);
    }
  }
  fclose(file);
}

void libfunctions_init() {
  SymbolTable_insert("print", 0, 0, ST_LIBFUNC, currScopespace(), -1);
  SymbolTable_insert("input", 0, 0, ST_LIBFUNC, currScopespace(), -1);
  SymbolTable_insert("objectmemberkeys", 0, 0, ST_LIBFUNC, currScopespace(),
                     -1);
  SymbolTable_insert("objecttotalmembers", 0, 0, ST_LIBFUNC, currScopespace(),
                     -1);
  SymbolTable_insert("objectcopy", 0, 0, ST_LIBFUNC, currScopespace(), -1);
  SymbolTable_insert("totalarguments", 0, 0, ST_LIBFUNC, currScopespace(), -1);
  SymbolTable_insert("argument", 0, 0, ST_LIBFUNC, currScopespace(), -1);
  SymbolTable_insert("typeof", 0, 0, ST_LIBFUNC, currScopespace(), -1);
  SymbolTable_insert("strtonum", 0, 0, ST_LIBFUNC, currScopespace(), -1);
  SymbolTable_insert("sqrt", 0, 0, ST_LIBFUNC, currScopespace(), -1);
  SymbolTable_insert("cos", 0, 0, ST_LIBFUNC, currScopespace(), -1);
  SymbolTable_insert("sin", 0, 0, ST_LIBFUNC, currScopespace(), -1);
}