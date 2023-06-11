#ifndef _vm_
#define _vm_

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AVM_STACK_ENV_SIZE 4
#define AVM_HASHTABLE_SIZE 211
#define AVM_STACK_SIZE 4096
#define AVM_WIPEOUT(m) memset(&(m), 0, sizeof(m))
#define AVM_MAX_INSTRUCTIONS (unsigned)nop_v

#define AVM_NUM_ACTUALS_OFFSET +4
#define AVM_SAVED_PC_OFFSET +3
#define AVM_SAVED_TOP_OFFSET +2
#define AVM_SAVED_TOPSP_OFFSET +1

#define AVM_ENDING_PC code_size

#define execute_add execute_arithmetic
#define execute_sub execute_arithmetic
#define execute_mul execute_arithmetic
#define execute_div execute_arithmetic
#define execute_mod execute_arithmetic

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

typedef enum avm_memcell_t {
  number_m = 0,
  string_m = 1,
  bool_m = 2,
  table_m = 3,
  userfunc_m = 4,
  libfunc_m = 5,
  nil_m = 6,
  undef_m = 7
} memcell_t;

typedef struct avm_table table;

typedef struct avm_memcell {
  memcell_t type;
  union {
    double num_val;
    char *str_val;
    unsigned char bool_val;
    table *table_val;
    unsigned userfunc_val;
    char *libfunc_val;
  } data;
} memcell;

typedef struct avm_table_bucket {
  memcell key;
  memcell value;
  struct avm_table_bucket *next;
} bucket;

struct avm_table {
  unsigned ref_counter;
  bucket *str_indexed[AVM_HASHTABLE_SIZE];
  bucket *num_indexed[AVM_HASHTABLE_SIZE];
  unsigned total;
};

typedef enum compare { eq, neq, lt, le, gt, ge } cmp;

typedef void (*memclear_func_t)(memcell *);
typedef void (*execute_func_t)(instr *);
typedef char *(*tostring_func_t)(memcell *);
typedef void (*library_func_t)(void);
typedef double (*arithmetic_func_t)(double x, double y);
typedef unsigned char (*to_bool_func_t)(memcell *);
typedef unsigned char (*eq_check_dispatch_t)(memcell *, memcell *, cmp c);

void decodeBinaryFile();

#endif
