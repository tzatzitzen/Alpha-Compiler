#include "target.h"

double *constNumbers = NULL;
unsigned totalConstNumbers = 0;
unsigned currConstNumbers = 0;
const char **constStrings = NULL;
unsigned totalConstStrings = 0;
unsigned currConstStrings = 0;
const char **libraryFunctions = NULL;
unsigned totalLibraryFunctions = 0;
unsigned currLibraryFunctions = 0;
userfunc *userFunctions = NULL;
unsigned totalUserFunctions = 0;
unsigned currUserFunctions = 0;

instr *instructions_head = NULL;
unsigned instructions_total = 0;
unsigned instructions_curr = 0;

incjump *incj_head = NULL;
unsigned incj_total = 0;
unsigned incj_curr = 0;

unsigned current_quad;

void array_expand(unsigned *total, unsigned curr, expr_t type) {
  assert(*total == curr);
  switch (type) {
  case constnum_e:
    constNumbers = (double *)realloc(constNumbers,
                                     (EXPAND_SIZE + *total) * sizeof(double));
    if (!constNumbers) {
      printf("Error realloc double\n");
      exit(1);
    }
    break;
  case conststring_e:
    constStrings = (const char **)realloc(
        constStrings, (EXPAND_SIZE + *total) * sizeof(const char *));
    if (!constStrings) {
      printf("Error realloc string\n");
      exit(1);
    }
    break;
  case libraryfunc_e:
    libraryFunctions = (const char **)realloc(
        libraryFunctions, (EXPAND_SIZE + *total) * sizeof(const char *));
    if (!libraryFunctions) {
      printf("Error realloc libraryfunctions\n");
      exit(1);
    }
    break;
  case programfunc_e:
    userFunctions = (userfunc *)realloc(userFunctions, (EXPAND_SIZE + *total) *
                                                           sizeof(userfunc));
    if (!userFunctions) {
      printf("Error realloc userfunctions\n");
      exit(1);
    }
    break;
  default:
    printf("Cant realloc this type\n");
    assert(0);
    break;
  }
  *total += EXPAND_SIZE;
}

unsigned insert_constNumber(double num) {

  if (totalConstNumbers == currConstNumbers) {
    array_expand(&totalConstNumbers, currConstNumbers, constnum_e);
  }
  unsigned pos = currConstNumbers;
  constNumbers[currConstNumbers++] = num;
  return pos;
}

unsigned insert_constString(const char *str) {

  if (totalConstStrings == currConstStrings) {
    array_expand(&totalConstStrings, currConstStrings, conststring_e);
  }
  unsigned pos = currConstStrings;
  constStrings[currConstStrings++] = str;
  return pos;
}

unsigned insert_libraryFunction(const char *func) {
  for (int i = 0; i < currLibraryFunctions; i++) {
    if (strcmp(func, libraryFunctions[i]) == 0) {
      return i;
    }
  }
  if (totalLibraryFunctions == currLibraryFunctions) {
    array_expand(&totalLibraryFunctions, currLibraryFunctions, libraryfunc_e);
  }
  unsigned pos = currLibraryFunctions;
  libraryFunctions[currLibraryFunctions++] = func;
  return pos;
}

unsigned insert_userFunction(Entry_T sym) {
  for (int i = 0; i < currUserFunctions; i++) {
    if (strcmp(sym->name, userFunctions[i].name) == 0 &&
        sym->isfunction->label == userFunctions[i].address) {
      return i;
    }
  }
  if (totalUserFunctions == currUserFunctions) {
    array_expand(&totalUserFunctions, currUserFunctions, programfunc_e);
  }
  unsigned pos = currUserFunctions;
  userfunc *u = userFunctions + currUserFunctions++;
  u->name = sym->name;
  u->address = sym->isfunction->label;
  u->localSize = sym->isfunction->locals;
  return pos;
}

void make_operand(expr *exp, avmarg *arg) {
  if (exp == NULL) {
    arg->type = nil_a;
    arg->val = UINT_MAX;
    return;
  }
  switch (exp->type) {
  case var_e:
  case tableitem_e:
  case arithexpr_e:
  case boolexpr_e:
  case assignexpr_e:
  case newtable_e: {
    assert(exp->sym);
    arg->val = exp->sym->offset;
    switch (exp->sym->space) {
    case programvar:
      arg->type = global_a;
      break;
    case functionlocal:
      arg->type = local_a;
      break;
    case formalarg:
      arg->type = formal_a;
      break;
    default:
      assert(0);
    }
    break;
  }
  case constbool_e: {
    arg->val = exp->boolConst;
    arg->type = bool_a;
    break;
  }
  case conststring_e: {
    arg->val = insert_constString(exp->strConst);
    arg->type = string_a;
    break;
  }
  case constnum_e: {
    arg->val = insert_constNumber(exp->numConst);
    arg->type = number_a;
    break;
  }
  case nil_e: {
    arg->type = nil_a;
    break;
  }
  case programfunc_e: {
    arg->val = insert_userFunction(exp->sym);
    arg->type = userfunc_a;
    break;
  }
  case libraryfunc_e: {
    arg->val = insert_libraryFunction(exp->sym->name);
    arg->type = libfunc_a;
    break;
  }
  default:
    printf("type: %d\n", exp->type);
    assert(0);
  }
}

void make_numberOperand(avmarg *arg, double num) {
  arg->val = insert_constNumber(num);
  arg->type = number_a;
}

void make_boolOperand(avmarg *arg, unsigned val) {
  arg->val = val;
  arg->type = bool_a;
}

void make_returnValOperand(avmarg *arg) { arg->type = retval_a; }

void emit_instruction(instr in) {
  if (instructions_total == instructions_curr) {
    instructions_head = (instr *)realloc(
        instructions_head, (EXPAND_SIZE + instructions_total) * sizeof(instr));
  }
  if (!instructions_head) {
    printf("Error realloc instruction\n");
  }
  instructions_total += EXPAND_SIZE;

  instr *tmp = instructions_head + instructions_curr++;
  tmp->opcode = in.opcode;
  tmp->result = in.result;
  tmp->arg1 = in.arg1;
  tmp->arg2 = in.arg2;
  tmp->line = in.line;
}

void add_inc_jump(unsigned instrNum, unsigned iaddress) {
  if (incj_total == incj_curr) {
    incj_head = (incjump *)realloc(incj_head, (EXPAND_SIZE + incj_total) *
                                                  sizeof(incjump));
    if (!incj_head) {
      printf("Error realloc jumps\n");
      exit(1);
    }
  }
  incj_total += EXPAND_SIZE;

  incjump *j = incj_head + incj_curr++;
  j->no_of_instr = instrNum;
  j->icode_address = iaddress;
}

unsigned getNextInstructionLabel() { return instructions_curr; }

void patchIncJumps() {
  for (int i = 0; i < incj_curr; i++) {
    if (incj_head[i].icode_address == nextQuadLabel()) {
      instructions_head[incj_head[i].no_of_instr].result.val =
          getNextInstructionLabel();
    } else {
      instructions_head[incj_head[i].no_of_instr].result.val =
          quads[incj_head[i].icode_address].taddress;
    }
  }
}

void generate_instr(avmop opcode, quad *quad) {
  instr in;
  in.opcode = opcode;
  make_operand(quad->arg1, &in.arg1);
  make_operand(quad->arg2, &in.arg2);
  make_operand(quad->result, &in.result);
  quad->taddress = getNextInstructionLabel();
  emit_instruction(in);
}

void generate_instr_relational(avmop opcode, quad *quad) {
  instr in;
  in.opcode = opcode;
  make_operand(quad->arg1, &in.arg1);
  make_operand(quad->arg2, &in.arg2);
  in.result.type = label_a;
  if (quad->label < current_quad) {
    in.result.val = quads[quad->label].taddress;
  } else {
    add_inc_jump(getNextInstructionLabel(), quad->label);
  }
  quad->taddress = getNextInstructionLabel();
  emit_instruction(in);
}
void reset_operand(avmarg *a) {
  a->type = nil_a;
  a->val = UINT_MAX;
}

func_stack *func_s = NULL;
void init_stack() {
  func_s = malloc(sizeof(struct stack_function));
  func_s->top = -1;
}

void push_function(Entry_T symbol) {
  if (func_s->top == SIZE) {
    // printf("Error, stackoveflow in function\n");
    return;
  }
  func_s->top++;
  func_s->stack_arr[func_s->top] = symbol;
}

Entry_T pop_function() {
  if (func_s->top == -1) {
    printf("Cannot pop element from empty stack\n");
    return NULL;
  }
  Entry_T symbol = func_s->stack_arr[func_s->top];
  func_s->top--;
  return symbol;
}

Entry_T top_function() {
  if (func_s->top == -1) {
    printf("Error, stack empty\n");
  }
  return func_s->stack_arr[func_s->top];
}

void append_ret(returnl **list, unsigned label) {
  returnl *new_node = malloc(sizeof(struct returnlist));
  new_node->label = label;
  new_node->next = NULL;
  if (*list == NULL) {
    *list = new_node;
  } else {
    returnl *current = *list;
    while (current->next) {
      current = current->next;
    }
    current->next = new_node;
  }
}

void backpatch(returnl *list, unsigned label) {
  returnl *current = list;
  while (current) {
    instructions_head[current->label].result.val = label;
    current = current->next;
  }
}

void generate_instr_ASSIGN(quad *quad) { generate_instr(assign_v, quad); }
void generate_instr_ADD(quad *quad) { generate_instr(add_v, quad); }
void generate_instr_SUB(quad *quad) { generate_instr(sub_v, quad); }
void generate_instr_MUL(quad *quad) { generate_instr(mul_v, quad); }
void generate_instr_DIV(quad *quad) { generate_instr(div_v, quad); }
void generate_instr_MOD(quad *quad) { generate_instr(mod_v, quad); }
void generate_instr_NEWTABLE(quad *quad) { generate_instr(newtable_v, quad); }
void generate_instr_TABLEGETELEM(quad *quad) {
  generate_instr(tablegetelem_v, quad);
}
void generate_instr_TABLESETELEM(quad *quad) {
  generate_instr(tablesetelem_v, quad);
}
void generate_instr_NOP(quad *quad) {
  instr in;
  in.opcode = nop_v;
  emit_instruction(in);
}
void generate_instr_JMP(quad *quad) { generate_instr_relational(jmp_v, quad); }
void generate_instr_JEQ(quad *quad) { generate_instr_relational(jeq_v, quad); }
void generate_instr_JNE(quad *quad) { generate_instr_relational(jne_v, quad); }
void generate_instr_JGT(quad *quad) { generate_instr_relational(jgt_v, quad); }
void generate_instr_JGE(quad *quad) { generate_instr_relational(jge_v, quad); }
void generate_instr_JLT(quad *quad) { generate_instr_relational(jlt_v, quad); }
void generate_instr_JLE(quad *quad) { generate_instr_relational(jle_v, quad); }
void generate_instr_NOT(quad *quad) {
  quad->taddress = getNextInstructionLabel();
  instr in;

  in.opcode = jeq_v;
  make_operand(quad->arg1, &in.arg1);
  make_boolOperand(&in.arg2, 0);
  in.result.type = label_a;
  in.result.val = getNextInstructionLabel() + 3;
  emit_instruction(in);

  in.opcode = assign_v;
  make_boolOperand(&in.arg1, 0);
  reset_operand(&in.arg2);
  make_operand(quad->result, &in.result);
  emit_instruction(in);

  in.opcode = jmp_v;
  reset_operand(&in.arg1);
  reset_operand(&in.arg2);
  in.result.type = label_a;
  in.result.val = getNextInstructionLabel() + 2;
  emit_instruction(in);

  in.opcode = assign_v;
  make_boolOperand(&in.arg1, 1);
  reset_operand(&in.arg2);
  make_operand(quad->result, &in.result);
  emit_instruction(in);
}
void generate_instr_OR(quad *quad) {
  quad->taddress = getNextInstructionLabel();
  instr in;

  in.opcode = jeq_v;
  make_operand(quad->arg1, &in.arg1);
  make_boolOperand(&in.arg2, 1);
  in.result.type = label_a;
  in.result.val = getNextInstructionLabel() + 4;
  emit_instruction(in);

  make_operand(quad->arg2, &in.arg2);
  in.result.val = getNextInstructionLabel() + 3;
  emit_instruction(in);

  in.opcode = assign_v;
  make_boolOperand(&in.arg1, 0);
  reset_operand(&in.arg2);
  make_operand(quad->result, &in.result);
  emit_instruction(in);

  in.opcode = jmp_v;
  reset_operand(&in.arg1);
  reset_operand(&in.arg2);
  in.result.type = label_a;
  in.result.val = getNextInstructionLabel() + 2;
  emit_instruction(in);

  in.opcode = assign_v;
  make_boolOperand(&in.arg1, 1);
  reset_operand(&in.arg2);
  make_operand(quad->result, &in.result);
  emit_instruction(in);
}
void generate_instr_AND(quad *quad) {
  quad->taddress = getNextInstructionLabel();
  instr in;

  in.opcode = jeq_v;
  make_operand(quad->arg1, &in.arg1);
  make_boolOperand(&in.arg2, 1);
  in.result.type = label_a;
  in.result.val = getNextInstructionLabel() + 4;
  emit_instruction(in);

  make_operand(quad->arg2, &in.arg2);
  in.result.val = getNextInstructionLabel() + 3;
  emit_instruction(in);

  in.opcode = assign_v;
  make_boolOperand(&in.arg1, 0);
  reset_operand(&in.arg2);
  make_operand(quad->result, &in.result);
  emit_instruction(in);

  in.opcode = jmp_v;
  reset_operand(&in.arg1);
  reset_operand(&in.arg2);
  in.result.type = label_a;
  in.result.val = getNextInstructionLabel() + 2;
  emit_instruction(in);

  in.opcode = assign_v;
  make_boolOperand(&in.arg1, 1);
  reset_operand(&in.arg2);
  make_operand(quad->result, &in.result);
  emit_instruction(in);
}

void generate_instr_PARAM(quad *quad) {
  quad->taddress = getNextInstructionLabel();
  instr in;
  in.opcode = pusharg_v;
  make_operand(quad->result, &in.result);
  emit_instruction(in);
}
void generate_instr_CALL(quad *quad) {
  quad->taddress = getNextInstructionLabel();
  instr in;
  in.opcode = call_v;
  make_operand(quad->result, &in.result);
  emit_instruction(in);
}
void generate_instr_GETRETVAL(quad *quad) {
  quad->taddress = getNextInstructionLabel();
  instr in;
  in.opcode = assign_v;
  make_operand(quad->result, &in.result);
  make_returnValOperand(&in.arg1);
  emit_instruction(in);
}
void generate_instr_FUNCSTART(quad *quad) {
  Entry_T func = quad->result->sym;
  func->isfunction->taddress = getNextInstructionLabel();
  quad->taddress = getNextInstructionLabel();

  insert_userFunction(func);
  push_function(func);

  instr in;
  in.opcode = funcenter_v;
  make_operand(quad->result, &in.result);
  emit_instruction(in);
}
void generate_instr_RETURN(quad *quad) {
  quad->taddress = getNextInstructionLabel();
  instr in;
  in.opcode = assign_v;
  make_returnValOperand(&in.result);
  make_operand(quad->arg1, &in.arg1);
  emit_instruction(in);

  Entry_T func = top_function();
  append_ret(&func->isfunction->return_list, getNextInstructionLabel());

  in.opcode = jmp_v;
  reset_operand(&in.arg1);
  reset_operand(&in.arg2);
  in.result.type = label_a;
  emit_instruction(in);
}
void generate_instr_FUNCEND(quad *quad) {
  Entry_T func = pop_function();
  backpatch(func->isfunction->return_list, getNextInstructionLabel());

  quad->taddress = getNextInstructionLabel();
  instr in;
  in.opcode = funcexit_v;
  make_operand(quad->result, &in.result);
  emit_instruction(in);
}

typedef void (*generator_func)(quad *);

generator_func generators[] = {
    generate_instr_ASSIGN,       generate_instr_ADD,
    generate_instr_SUB,          generate_instr_MUL,
    generate_instr_DIV,          generate_instr_MOD,
    generate_instr_NEWTABLE,     generate_instr_TABLEGETELEM,
    generate_instr_TABLESETELEM, generate_instr_NOP,
    generate_instr_JMP,          generate_instr_JEQ,
    generate_instr_JNE,          generate_instr_JGT,
    generate_instr_JGE,          generate_instr_JLT,
    generate_instr_JLE,          generate_instr_NOT,
    generate_instr_OR,           generate_instr_AND,
    generate_instr_PARAM,        generate_instr_CALL,
    generate_instr_GETRETVAL,    generate_instr_FUNCSTART,
    generate_instr_RETURN,       generate_instr_FUNCEND,
};

void print_arrays() {
  for (int i = 0; i < currConstNumbers; i++) {
    printf("pos: %d, num: %f\n", i, constNumbers[i]);
  }
  for (int i = 0; i < currConstStrings; i++) {
    printf("pos: %d, num: %s\n", i, constStrings[i]);
  }
  for (int i = 0; i < currLibraryFunctions; i++) {
    printf("pos: %d, num: %s\n", i, libraryFunctions[i]);
  }
  for (int i = 0; i < currUserFunctions; i++) {
    printf("pos: %d, num: %s, address: %d\n", i, userFunctions[i].name,
           userFunctions[i].address);
  }
}

void generate_all(void) {
  init_stack();
  for (int i = 0; i < nextQuadLabel(); i++) {
    current_quad = i;
    (*generators[quads[i].op])(quads + i);
  }
  patchIncJumps();
  print_arrays();
}

void serialize(unsigned int globals) {
  FILE *f;
  f = fopen("target_to_bin.abc", "wb");
  assert(f);

  unsigned int magicnumber = 340200501;
  fwrite(&magicnumber, sizeof(unsigned), 1, f);

  fwrite(&currConstStrings, sizeof(unsigned), 1, f);
  printf("string size: %d\n", currConstStrings);
  for (unsigned i = 0; i < currConstStrings; i++) {
    unsigned size = strlen(constStrings[i]) + 1;

    fwrite(&size, sizeof(unsigned), 1, f);
    fwrite(constStrings[i], sizeof(char), size, f);
  }

  fwrite(&currConstNumbers, sizeof(unsigned), 1, f);
  printf("num size: %d\n", currConstNumbers);
  for (unsigned i = 0; i < currConstNumbers; i++) {
    printf("writing: %f\n", constNumbers[i]);
    fwrite(&constNumbers[i], sizeof(double), 1, f);
  }

  fwrite(&currUserFunctions, sizeof(unsigned), 1, f);
  printf("user size: %d\n", currUserFunctions);
  for (unsigned i = 0; i < currUserFunctions; i++) {
    userfunc curr = userFunctions[i];
    fwrite(&(curr.address), sizeof(unsigned), 1, f);
    fwrite(&(curr.localSize), sizeof(unsigned), 1, f);
    const char *funcName = curr.name;
    unsigned size = strlen(funcName) + 1;
    fwrite(&size, sizeof(unsigned), 1, f);
    fwrite(funcName, sizeof(char), size, f);
  }

  fwrite(&currLibraryFunctions, sizeof(unsigned), 1, f);
  for (unsigned i = 0; i < currLibraryFunctions; i++) {
    unsigned size = strlen(libraryFunctions[i]) + 1;

    fwrite(&size, sizeof(unsigned), 1, f);
    printf("size: %d\n", size);
    fwrite(libraryFunctions[i], sizeof(char), size, f);
  }

  fwrite(&instructions_curr, sizeof(unsigned), 1, f);
  for (unsigned i = 0; i < instructions_curr; i++) {
    fwrite(&(instructions_head[i].opcode), sizeof(avmop), 1, f);
    fwrite(&(instructions_head[i].result.type), sizeof(avmarg_t), 1, f);
    fwrite(&(instructions_head[i].result.val), sizeof(unsigned int), 1, f);
    fwrite(&(instructions_head[i].arg1.type), sizeof(avmarg_t), 1, f);
    fwrite(&(instructions_head[i].arg1.val), sizeof(unsigned int), 1, f);
    fwrite(&(instructions_head[i].arg2.type), sizeof(avmarg_t), 1, f);
    fwrite(&(instructions_head[i].arg2.val), sizeof(unsigned int), 1, f);
    fwrite(&(instructions_head[i].line), sizeof(unsigned int), 1, f);
  }

  fwrite(&globals, sizeof(unsigned), 1, f);
  fclose(f);
}
