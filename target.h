#ifndef _target_
#define _target_

#include "intermediate.h"
#include "symtable.h"
#include <limits.h>

typedef enum avmOpcode {
  assign_v,
  add_v,
  sub_v,
  mul_v,
  div_v,
  mod_v,
  uminus_v,
  and_v,
  or_v,
  not_v,
  jmp_v,
  jeq_v,
  jne_v,
  jle_v,
  jge_v,
  jlt_v,
  jgt_v,
  call_v,
  pusharg_v,
  funcenter_v,
  funcexit_v,
  newtable_v,
  tablegetelem_v,
  tablesetelem_v,
  nop_v
} avmop;

typedef enum avmArg_t {
  label_a = 0,
  global_a = 1,
  formal_a = 2,
  local_a = 3,
  number_a = 4,
  string_a = 5,
  bool_a = 6,
  nil_a = 7,
  userfunc_a = 8,
  libfunc_a = 9,
  retval_a = 10
} avmarg_t;

typedef struct avmArg {
  avmarg_t type;
  unsigned int val;
} avmarg;

typedef struct instruction {
  avmop opcode;
  avmarg result;
  avmarg arg1;
  avmarg arg2;
  unsigned int line;
} instr;

typedef struct userFunc {
  const char *name;
  unsigned int address;
  unsigned int localSize;
} userfunc;

typedef struct incomplete_jump {
  unsigned no_of_instr;
  unsigned icode_address;
  struct incomplete_jump *next;
} incjump;

typedef struct stack_function {
  int top;
  Entry_T stack_arr[SIZE];
} func_stack;

void serialize(unsigned int globals);
void generate_all(void);
#endif
