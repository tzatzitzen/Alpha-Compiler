#include "vm.h"

#define AVM_ENDING_PC code_size

memcell avm_stack[AVM_STACK_SIZE];

double *const_num_array = NULL;
unsigned const_num_size = 0;

char **const_string_array = NULL;
unsigned const_string_size = 0;

char **lib_func_array = NULL;
unsigned lib_func_size = 0;

userfunc *user_func_array = NULL;
unsigned user_func_size = 0;

library_func_t LibTable[12];
int h_count = 0;

unsigned globals_total = 0;

unsigned char execution_finished = 0;
unsigned pc = 0;

unsigned curr_line = 0;
unsigned code_size = 0;
instr *code = (instr *)0;

int inFunc = 0;

memcell retval;
unsigned top, topsp;

memcell ax, bx, cx;

unsigned total_actuals = 0;

char *typeStrings[] = {
    "number", "string", "bool", "table", "userfunc", "libfunc", "nil", "undef",
};

void avm_memcell_clear(memcell *cell);

memcell *avm_table_getelem(table *tbl, memcell *index);
void avm_table_setelem(memcell *tbl, memcell *index, memcell *content);
void avm_call_libfunc(char *id);

void avm_call_save_env();
void avm_call_functor(table *tbl);
void avm_push_table_arg(table *tbl);
userfunc *avm_get_funcinfo(unsigned address);
void avm_register_libfunc(char *id, library_func_t address);
char *number_to_string(memcell *);
char *string_to_string(memcell *);
char *bool_to_string(memcell *);
char *table_to_string(memcell *);
char *userfunc_to_string(memcell *);
char *libfunc_to_string(memcell *);
char *nil_to_string(memcell *);
char *undef_to_string(memcell *);
library_func_t avm_get_libfunc(char *id);

void avm_warning(char *mess) { printf("Warning: %s\n", mess); }
void avm_error(char *mess) {
  printf("Error at %s \n", mess);
  execution_finished = 1;
}

unsigned char number_to_bool(memcell *m) { return m->data.num_val != 0; }
unsigned char string_to_bool(memcell *m) { return m->data.str_val[0] != 0; }
unsigned char bool_to_bool(memcell *m) { return m->data.bool_val; }
unsigned char table_to_bool(memcell *m) { return 1; }
unsigned char userfunc_to_bool(memcell *m) { return 1; }
unsigned char libfunc_to_bool(memcell *m) { return 1; }
unsigned char nil_to_bool(memcell *m) { return 0; }
unsigned char undef_to_bool(memcell *m) { return 0; }
to_bool_func_t to_boolFuncs[] = {

    number_to_bool,   string_to_bool,  bool_to_bool, table_to_bool,
    userfunc_to_bool, libfunc_to_bool, nil_to_bool,  undef_to_bool,
};

unsigned char avm_tobool(memcell *m) {
  assert(m->type >= 0 && m->type < undef_m);
  return (*to_boolFuncs[m->type])(m);
}

unsigned char check_numbers(memcell *rv1, memcell *rv2, cmp c) {
  switch (c) {
  case eq:
    return rv1->data.num_val == rv2->data.num_val;
  case neq:
    return rv1->data.num_val != rv2->data.num_val;
  case lt:
    return rv1->data.num_val < rv2->data.num_val;
  case le:
    return rv1->data.num_val <= rv2->data.num_val;
  case gt:
    return rv1->data.num_val > rv2->data.num_val;
  case ge:
    return rv1->data.num_val >= rv2->data.num_val;
  default:
    assert(0);
  }
}

unsigned char check_strings(memcell *rv1, memcell *rv2, cmp c) {
  return !strcmp(rv1->data.str_val, rv2->data.str_val);
}

eq_check_dispatch_t checkDispatch[] = {
    check_numbers, check_strings, 0, 0, 0, 0, 0, 0};

unsigned hashFunc(unsigned key) { return key % AVM_HASHTABLE_SIZE; }

unsigned hashString(char *str) {
  unsigned key = 5381;
  int c;
  while ((c = *str++))
    key = ((key << 5) + key) + c;
  return hashFunc(key);
}

bucket **get_bucket(table *tbl, memcell *i, unsigned *hash) {
  bucket **bucket;

  switch (i->type) {
  case number_m:
    bucket = tbl->num_indexed;
    *hash = hashFunc(i->data.num_val);
    break;
  case string_m:
    bucket = tbl->str_indexed;
    *hash = hashString(i->data.str_val);
    break;
  default:
    assert(0);
  }

  return bucket;
}

memcell *avm_table_getelem(table *tbl, memcell *index) {
  assert(tbl);
  assert(index);

  unsigned hash;
  bucket **bucket_list = get_bucket(tbl, index, &hash);
  bucket *current = bucket_list[hash];
  while (current) {
    if ((*checkDispatch)(&current->value, index, eq)) {
      return &current->value;
    }
    current = current->next;
  }
  return NULL;
}

void avm_table_incr_refcounter(table *tbl) { tbl->ref_counter++; }

void avm_table_setelem(memcell *tbl, memcell *index, memcell *content) {
  assert(tbl->data.table_val);
  assert(index);
  assert(content);

  unsigned hash;
  bucket **bucket_list = get_bucket(tbl->data.table_val, index, &hash);
  bucket *current = bucket_list[hash];
  while (current) {
    if ((*checkDispatch)(&current->value, index, eq)) {
      avm_memcell_clear(&current->value);
      current->value = *content;

      return;
    }
    current = current->next;
  }

  bucket *newBucket = malloc(sizeof(struct avm_table_bucket));
  assert(newBucket);
  newBucket->key = *index;
  newBucket->value = *content;
  newBucket->next = bucket_list[hash];

  if (content->type == table_m)
    avm_table_incr_refcounter(content->data.table_val);

  bucket_list[hash] = newBucket;
}

double const_get_number(unsigned pos) { return const_num_array[pos]; }
char *const_get_string(unsigned pos) { return const_string_array[pos]; }
char *libfuncs_get_used(unsigned pos) { return lib_func_array[pos]; }
userfunc *userfuncs_get_used(unsigned pos) { return &user_func_array[pos]; }

userfunc *avm_get_funcinfo(unsigned address) {
  for (unsigned i = 0; i < user_func_size; i++) {
    if (user_func_array[i].address == address) {
      return &user_func_array[i];
    }
  }
  avm_error("Function not found!\n");
  return NULL;
}
memcell *avm_table_getelem(table *tbl, memcell *index);

tostring_func_t to_stringFunc[] = {
    number_to_string,   string_to_string,  bool_to_string, table_to_string,
    userfunc_to_string, libfunc_to_string, nil_to_string,  undef_to_string};

static void avm_init_stack() {
  for (int i = 0; i < AVM_STACK_SIZE; i++) {
    AVM_WIPEOUT(avm_stack[i]);
    avm_stack[i].type = undef_m;
  }
}

char *avm_to_string(memcell *m) {
  assert(m->type >= 0 && m->type <= undef_m);
  // printf("type: %d", m->type);
  return (*to_stringFunc[m->type])(m);
}

void avm_table_buckets_destroy(bucket **b) {
  for (int i = 0; i < AVM_HASHTABLE_SIZE; i++, b++) {
    for (bucket *tmp = *b; tmp;) {
      bucket *rmv = tmp;
      tmp = tmp->next;
      avm_memcell_clear(&rmv->key);
      avm_memcell_clear(&rmv->value);
      free(rmv);
    }
    b[i] = (bucket *)0;
  }
}
void avm_table_destroy(table *tbl) {
  avm_table_buckets_destroy(tbl->str_indexed);
  avm_table_buckets_destroy(tbl->num_indexed);
}

void avm_table_decr_refcounter(table *tbl) {
  assert(tbl->ref_counter > 0);
  if (!--tbl->ref_counter) {
    avm_table_destroy(tbl);
  }
}

void avm_table_buckets_init(bucket **b) {
  for (int i = 0; i < AVM_HASHTABLE_SIZE; i++) {
    b[i] = (bucket *)0;
  }
}

table *avm_table_new() {
  table *tmp = malloc(sizeof(struct avm_table));
  AVM_WIPEOUT(tmp);
  tmp->ref_counter = tmp->total = 0;
  avm_table_buckets_init(tmp->num_indexed);
  avm_table_buckets_init(tmp->str_indexed);
  return tmp;
}

void memclear_string(memcell *str) {
  // printf("GOTIN_MEMCLEAR_STRING\n");
  assert(str->data.str_val);
  free(str->data.str_val);
}

void memclear_table(memcell *tbl) {
  assert(tbl->data.table_val);
  avm_table_decr_refcounter(tbl->data.table_val);
}

memclear_func_t memclear_funcs[] = {
    0, memclear_string, 0, memclear_table, 0, 0, 0, 0};

void avm_memcell_clear(memcell *cell) {
  if (cell->type != undef_m) {
    memclear_func_t func = memclear_funcs[cell->type];
    if (func) {
      (*func)(cell);
      cell->type = undef_m;
    }
  }
}

void print_cell(unsigned pos) {

  memcell *tmp = &avm_stack[pos];
  // printf("address: %p pos: %d val:%f\n", (void *)&avm_stack[pos],
  // pos,tmp->data.num_val);
}

void print_stack(unsigned range) {
  printf("\n\n===============================\n");
  for (unsigned i = AVM_STACK_SIZE - 1; i >= AVM_STACK_SIZE - range; i--) {
    print_cell(i);
  }
  printf("===============================\n\n");
}

memcell *avm_translate(avmarg *arg, memcell *reg) {

  // print_stack(10);
  printf("type: %d\n", arg->type);
  printf("pc: %d\n", pc);

  switch (arg->type) {
  case global_a: {
    printf("Global:\n");
    print_cell(AVM_STACK_SIZE - 1 - arg->val);
    return &avm_stack[AVM_STACK_SIZE - 1 - arg->val];
  }
  case local_a:
    printf("local:\n");
    print_cell(topsp - arg->val);
    print_cell(topsp - arg->val);
    return &avm_stack[topsp - arg->val];
  case formal_a:
    printf("formal:\n");
    return &avm_stack[topsp + AVM_STACK_ENV_SIZE + 1 + arg->val];
  case retval_a:
    printf("ret\n");
    return &retval;
  case number_a:
    printf("number:\n");
    reg->type = number_m;
    reg->data.num_val = const_get_number(arg->val);
    return reg;
  case string_a:
    printf("string:\n");
    reg->type = string_m;
    reg->data.str_val = const_get_string(arg->val);
    return reg;
  case bool_a:
    printf("bool:\n");
    reg->type = bool_m;
    reg->data.bool_val = arg->val;
    return reg;
  case nil_a:
    printf("nil:\n");
    reg->type = nil_m;
    return reg;
  case userfunc_a:
    printf("user:\n");
    reg->type = userfunc_m;
    reg->data.userfunc_val = arg->val;
    return reg;
  case libfunc_a:
    printf("lib:\n");
    reg->type = libfunc_m;
    reg->data.libfunc_val = libfuncs_get_used(arg->val);
    return reg;
  default:
    assert(0);
  }
}

void avm_assign(memcell *lv, memcell *rv) {
  if (lv == rv) {
    return;
  }
  if (lv->type == table_m && rv->type == table_m &&
      lv->data.table_val == rv->data.table_val) {
    return;
  }
  if (rv->type == undef_m) {
    avm_warning("assigning from undef\n");
  }

  avm_memcell_clear(lv);
  memcpy(lv, rv, sizeof(struct avm_memcell));

  if (lv->type == string_m) {
    lv->data.str_val = strdup(rv->data.str_val);
  } else if (lv->type == table_m) {
    avm_table_incr_refcounter(lv->data.table_val);
  }
}

void print_pointers(memcell *lv) {
  printf("lv = %p\n", (void *)lv);
  printf("&avm_stack[top] = %p\n", (void *)&avm_stack[top]);
  printf("&avm_stack[AVM_STACK_SIZE - 1] = %p\n",
         (void *)&avm_stack[AVM_STACK_SIZE - 1]);
  printf("&retval = %p\n", (void *)&retval);
}

unsigned count = 0;
void execute_assign(instr *in) {
  printf("Before Assign:\n");
  //  printf("\n---------GOTIN_ASSING: %d---------\n", ++count);
  memcell *lv = avm_translate(&in->result, (memcell *)0);
  memcell *rv = avm_translate(&in->arg1, &ax);
  print_pointers(lv);
  assert(lv && (&avm_stack[AVM_STACK_SIZE - 1] >= lv && lv > &avm_stack[top]) ||
         lv == &retval);
  avm_assign(lv, rv);
  // printf("After Assign: \n");
  //  print_stack(10);
}

void execute_add(instr *in);
void execute_sub(instr *in);
void execute_mul(instr *in);
void execute_div(instr *in);
void execute_mod(instr *in);
void execute_uminus(instr *in);
void execute_and(instr *in);
void execute_or(instr *in);
void execute_not(instr *in);
void execute_jmp(instr *in);
void execute_jeq(instr *in);
void execute_jne(instr *in);
void execute_jle(instr *in);
void execute_jge(instr *in);
void execute_jlt(instr *in);
void execute_jgt(instr *in);

void execute_call(instr *in) {
  // printf("GOTIN_CALL\n");
  memcell *func = avm_translate(&in->result, &ax);
  assert(func);
  switch (func->type) {
  case userfunc_m:
    avm_call_save_env();
    pc = user_func_array[func->data.userfunc_val].address;
    // printf("func name: %s\n", user_func_array[func->data.userfunc_val].name);
    assert(pc < AVM_ENDING_PC);
    // printf("pc: %d, opcode: %d\n\n", pc, code[pc].opcode);
    assert(code[pc].opcode == funcenter_v);
    break;
  case string_m:
    avm_call_libfunc(func->data.str_val);
    break;
  case libfunc_m:
    // printf("name: %s\n", func->data.libfunc_val);
    avm_call_libfunc(func->data.libfunc_val);
    break;
  case table_m:
    avm_call_functor(func->data.table_val);
    break;
  default: {
    char *str = avm_to_string(func);
    char *mess = " call: cannot bind ";
    strcat(mess, str);
    strcat(mess, " to function");
    avm_error(mess);
    free(str);
    execution_finished = 1;
  }
  }
}

void execute_pusharg(instr *in);
void execute_newtable(instr *in);

void execute_tablegetelem(instr *in) {
  memcell *lv = avm_translate(&in->result, (memcell *)0);
  memcell *t = avm_translate(&in->arg1, (memcell *)0);
  memcell *i = avm_translate(&in->arg2, (memcell *)0);

  assert(lv && (&avm_stack[AVM_STACK_SIZE - 1] >= lv && lv > &avm_stack[top] ||
                lv == &retval));

  avm_memcell_clear(lv);
  lv->type = nil_m;

  if (t->type != table_m) {
    char *error;
    sprintf(error, "illegal use of type %s as table", typeStrings[t->type]);
    avm_error(error);
  } else {
    memcell *content = avm_table_getelem(t->data.table_val, i);
    if (content) {
      avm_assign(lv, content);
    } else {
      char *ts = avm_to_string(t);
      char *is = avm_to_string(i);
      char *warning;
      sprintf(warning, "%s[%s] not found!", ts, is);
      avm_warning(warning);
      free(ts);
      free(is);
    }
  }
}

void execute_tablesetelem(instr *in) {
  memcell *t = avm_translate(&in->result, (memcell *)0);
  memcell *i = avm_translate(&in->arg1, &ax);
  memcell *c = avm_translate(&in->arg2, &bx);

  assert(t && &avm_stack[AVM_STACK_SIZE - 1] >= t && &avm_stack[top]);
  assert(i && c);

  if (t->type != table_m) {
    char *error;
    sprintf(error, "illegal use of type %s as table", typeStrings[t->type]);
    avm_error(error);
  } else {
    avm_table_setelem(t, i, c);
  }
}

void execute_funcenter(instr *in) {
  memcell *func = avm_translate(&in->result, &ax);
  assert(func);
  assert(pc == user_func_array[func->data.userfunc_val].address);
  inFunc++;
  total_actuals = 0;
  userfunc *func_info = avm_get_funcinfo(pc);
  topsp = top;
  top = top - func_info->localSize;
}

unsigned avm_get_env_value(unsigned pos) {
  assert(avm_stack[pos].type == number_m);
  unsigned val = (unsigned)avm_stack[pos].data.num_val;
  assert(avm_stack[pos].data.num_val == ((double)val));
  return val;
}

void execute_funcexit(instr *in) {
  // printf("GOTIN_FUNCEXIT\n\n");
  unsigned old_top = top;
  top = avm_get_env_value(topsp + AVM_SAVED_TOP_OFFSET);
  pc = avm_get_env_value(topsp + AVM_SAVED_PC_OFFSET);
  topsp = avm_get_env_value(topsp + AVM_SAVED_TOPSP_OFFSET);
  while (++old_top <= top) {
    avm_memcell_clear(&avm_stack[old_top]);
  }
  inFunc--;
}

void execute_nop(instr *in);
void execute_and(instr *in) {}
void execute_or(instr *in) {}
void execute_not(instr *in) {}
void execute_uminus(instr *in) {}

execute_func_t execute_funcs[] = {

    execute_assign,   execute_add,          execute_sub,
    execute_mul,      execute_div,          execute_mod,
    execute_uminus,   execute_and,          execute_or,
    execute_not,      execute_jmp,          execute_jeq,
    execute_jne,      execute_jle,          execute_jge,
    execute_jlt,      execute_jgt,          execute_call,
    execute_pusharg,  execute_funcenter,    execute_funcexit,
    execute_newtable, execute_tablegetelem, execute_tablesetelem,
    execute_nop};

void avm_call_functor(table *tbl) {
  cx.type = string_m;
  cx.data.str_val = "()";
  memcell *functor = avm_table_getelem(tbl, &cx);
  if (!functor) {
    avm_error("in calling table: no '()' element found");
  } else if (functor->type == table_m) {
    avm_call_functor(functor->data.table_val);
  } else if (functor->type == userfunc_m) {
    avm_push_table_arg(tbl);
    avm_call_save_env();
    pc = functor->data.userfunc_val;
    assert(pc < AVM_ENDING_PC && code[pc].opcode == funcenter_v);
  } else {
    avm_error("in calling table: illegal '()' element value");
  }
}

void avm_dec_top() {
  if (!top) {
    avm_error("stack overflow");
  } else {
    top--;
  }
}

void avm_push_env_value(unsigned val) {
  // printf("GOTINENVVALUE\n\n");
  // printf("Pushing: %d\n", val);
  avm_stack[top].type = number_m;
  avm_stack[top].data.num_val = val;
  avm_dec_top();
}

void avm_call_save_env() {
  avm_push_env_value(total_actuals);
  assert(code[pc].opcode == call_v);
  avm_push_env_value(pc + 1);
  avm_push_env_value(top + total_actuals + 2);
  avm_push_env_value(topsp);
  // printf("GOTOUT_CALL_SAVE_ENV\n");
}

void avm_call_libfunc(char *id) {
  library_func_t func = avm_get_libfunc(id);
  if (!func) {
    char *error;
    sprintf(error, "unsupported lib func '%s' called", id);
    avm_error(error);
  } else {
    avm_call_save_env();
    topsp = top;
    total_actuals = 0;
    inFunc++;
    (*func)();
    if (!execution_finished) {
      execute_funcexit((instr *)0);
    }
  }
  // printf("GOTOUT_CALL_LIB\n");
}

unsigned avm_total_actuals() {
  return avm_get_env_value(topsp + AVM_NUM_ACTUALS_OFFSET);
}

memcell *avm_get_actual(unsigned pos) {
  assert(pos < avm_total_actuals());
  memcell *tmp = &avm_stack[topsp + AVM_STACK_ENV_SIZE + 1 + pos];
  // printf("Getting value %f from the stack\n", tmp->data.num_val);
  return tmp;
}

void libfunc_print() {
  unsigned int total = avm_total_actuals();

  for (unsigned int i = 0; i < total; ++i) {
    char *str = avm_to_string(avm_get_actual(i));

    if (str) {
      for (int j = 0; str[j] != '\0'; ++j) {
        if (str[j] == '\\' && str[j + 1] == 'n') {
          printf("\n");
          j++;
        } else {
          printf("%c", str[j]);
        }
      }
    } else {
      printf("NULL\n");
    }
  }
}

void avm_push_table_arg(table *tbl) {
  // printf("GOTINPUSHARG\n\n");
  avm_stack[top].type = table_m;
  avm_table_incr_refcounter(avm_stack[top].data.table_val = tbl);
  ++total_actuals;
  avm_dec_top();
}

double add_impl(double x, double y) { return x + y; }
double sub_impl(double x, double y) { return x - y; }
double mul_impl(double x, double y) { return x * y; }
double div_impl(double x, double y) {
  if (y == 0) {
    avm_error("dividing by 0\n");
    return x;
  }
  return x / y;
}
double mod_impl(double x, double y) {
  if (y == 0) {
    avm_error("dividing by 0\n");
    return x;
  }
  return ((unsigned)x) % ((unsigned)y);
}

arithmetic_func_t arithmeticFuncs[] = {add_impl, sub_impl, mul_impl, div_impl};

void execute_arithmetic(instr *in) {
  memcell *lv = avm_translate(&in->result, (memcell *)0);
  memcell *rv1 = avm_translate(&in->arg1, &ax);
  memcell *rv2 = avm_translate(&in->arg2, &bx);

  assert(lv && (&avm_stack[AVM_STACK_SIZE - 1] >= lv && lv > &avm_stack[top] ||
                lv == &retval));
  assert(rv1 && rv2);

  if (rv1->type != number_m || rv2->type != number_m) {
    avm_error("not a number in arithmetic");
    execution_finished = 1;
  } else {
    arithmetic_func_t op = arithmeticFuncs[in->opcode - add_v];
    avm_memcell_clear(lv);
    lv->type = number_m;
    lv->data.num_val = (*op)(rv1->data.num_val, rv2->data.num_val);
  }
}

void execute_jeq(instr *in) {
  assert(in->result.type == label_a);

  memcell *rv1 = avm_translate(&in->arg1, &ax);
  memcell *rv2 = avm_translate(&in->arg2, &bx);

  unsigned char result = 0;

  if (rv1->type == undef_m || rv2->type == undef_m) {
    avm_error("'undef' involved in equality!\n");
  } else if (rv1->type == bool_m || rv2->type == bool_m) {
    result = (avm_tobool(rv1) == avm_tobool(rv2));
  } else if (rv1->type == nil_m || rv2->type == nil_m) {
    result = rv1->type == nil_m && rv2->type == nil_m;
  } else if (rv1->type != rv2->type) {
    char *error;
    sprintf(error, "%s == %s is illegal\n", typeStrings[rv1->type],
            typeStrings[rv2->type]);
    avm_error(error);
  } else {
    result = (*checkDispatch[rv1->type])(rv1, rv2, eq);
  }
  if (!execution_finished && result) {
    printf("==JEQ: %d\n\n", in->result.val);
    pc = in->result.val;
  }
}

void execute_jne(instr *in) {
  assert(in->result.type == label_a);

  memcell *rv1 = avm_translate(&in->arg1, &ax);
  memcell *rv2 = avm_translate(&in->arg2, &bx);

  unsigned char result = 0;

  if (rv1->type == undef_m || rv2->type == undef_m)
    avm_error("'undef' involved in equality");
  else if (rv1->type == nil_m || rv2->type == nil_m)
    result = (rv1->type == nil_m || rv2->type == nil_m);
  else if (rv1->type == bool_m || rv2->type == bool_m)
    result = (avm_tobool(rv1) == avm_tobool(rv2));
  else if (rv1->type != rv2->type)
    avm_error("Illegal check");
  else {
    if (rv1->type == number_m)
      result = rv1->data.num_val == rv2->data.num_val;
    else if (rv1->type == string_m)
      result = !strcmp(rv1->data.str_val, rv2->data.str_val);
    else
      result = (avm_tobool(rv1) == avm_tobool(rv2));
  }
}

void execute_jle(instr *instr) {
  assert(instr->result.type == label_a);
  memcell *rv1 = avm_translate(&instr->arg1, &ax);
  memcell *rv2 = avm_translate(&instr->arg2, &bx);

  if (rv1->type == undef_m || rv2->type == undef_m)
    avm_error("undef shouldn't be in less eq");
  if (rv1->type != number_m || rv2->type != number_m)
    avm_error("illegal comparison");
  if (rv1->data.num_val <= rv2->data.num_val)
    pc = instr->result.val;
  else
    pc++;
}

void execute_jge(instr *instr) {
  assert(instr->result.type == label_a);
  memcell *rv1 = avm_translate(&instr->arg1, &ax);
  memcell *rv2 = avm_translate(&instr->arg2, &bx);

  if (rv1->type == undef_m || rv2->type == undef_m)
    avm_error("undef shouldn't be in greater eq");
  if (rv1->type != number_m || rv2->type != number_m)
    avm_error("illegal comparison");
  if (rv1->data.num_val >= rv2->data.num_val)
    pc = instr->result.val;
  else
    pc++;
}

void execute_jlt(instr *instr) {
  assert(instr->result.type == label_a);
  memcell *rv1 = avm_translate(&instr->arg2, &ax);
  memcell *rv2 = avm_translate(&instr->arg2, &bx);
  if (rv1->type == undef_m || rv2->type == undef_m)
    avm_error("undef shouldn't be in lesser than");
  if (rv1->type != number_m || rv2->type != number_m)
    avm_error("illegal comparison");
  if (rv1->data.num_val < rv2->data.num_val)
    pc = instr->result.val;
  else
    pc++;
}

void execute_jgt(instr *instr) {
  assert(instr->result.type == label_a);
  // printf("GOTIN_JGT\n");
  memcell *rv1 = avm_translate(&instr->arg1, &ax);
  memcell *rv2 = avm_translate(&instr->arg2, &bx);
  if (rv1->type == undef_m || rv2->type == undef_m)
    avm_error("undef shouldn't be in greater than");
  if (rv1->type != number_m || rv2->type != number_m)
    avm_error("illegal comparison");
  if (rv1->data.num_val > rv2->data.num_val)
    pc = instr->result.val;
  else
    pc++;
}

void execute_jmp(instr *in) {
  // printf("GOTIN_JMP: %d\n\n", in->result.val);
  assert(in->result.type == label_a);

  if (!execution_finished)
    pc = in->result.val;
}

void libfunc_typeof() {
  unsigned n = avm_total_actuals();
  if (n != 1) {
    char *error;
    sprintf(error, "one argument (not %d) expected in 'typeof'\n", n);
    avm_error(error);
  } else {
    avm_memcell_clear(&retval);
    retval.type = string_m;
    retval.data.str_val = strdup(typeStrings[avm_get_actual(0)->type]);
  }
}

void libfunc_totalarguments() {
  unsigned p_topsp = avm_get_env_value(topsp + AVM_SAVED_TOPSP_OFFSET);
  avm_memcell_clear(&retval);

  if (!p_topsp) {
    avm_error("'totalarguments' called outside of function");
    retval.type = nil_m;
  } else {
    retval.type = number_m;
    retval.data.num_val = avm_get_env_value(p_topsp + AVM_NUM_ACTUALS_OFFSET);
  }
}

void libfunc_argument(char *id) {
  unsigned p_topsp = avm_get_env_value(topsp + AVM_SAVED_TOPSP_OFFSET);
  if (!p_topsp) {
    avm_memcell_clear(&retval);
    retval.type = nil_m;
  }

  unsigned n = avm_total_actuals();
  if (n != 1) {
    avm_error("'argument' expected 1 argument");
  }
  unsigned totalargs = avm_get_env_value(p_topsp + AVM_NUM_ACTUALS_OFFSET);
  memcell *arg = avm_get_actual(0);
  if (arg->type != number_m) {
    avm_error("'argument' expected number");
  }
  if (arg->data.num_val < 0 || arg->data.num_val >= totalargs) {
    avm_error("'argument' no argument");
  }
  avm_assign(&retval, &avm_stack[p_topsp + AVM_NUM_ACTUALS_OFFSET + 1 +
                                 (int)arg->data.num_val]);
}

// eidwmen
void libfunc_input() {}

void libfunc_objectmemberkeys() {
  unsigned i = avm_total_actuals();

  if (i != 1) {
    avm_error("one argument expected");
    return;
  }
  if (avm_get_actual(0)->type != table_m) {
    avm_error("'objectmemberkeys' gets args of type table");
    return;
  }
  if (avm_get_actual(0)->data.table_val != NULL) {
    avm_memcell_clear(&retval);
    retval.type = table_m;
    retval.data.table_val = avm_get_actual(0)->data.table_val;
  }
}

void libfunc_objecttotalmembers() {
  memcell *mem = avm_get_actual(0);
  int i = mem->data.table_val->total;
  retval.data.num_val = i;
}

// kati leipei ??
void libfunc_objectcopy() {
  unsigned i = avm_total_actuals();
  if (i != 1)
    avm_error("'object copy' expected 1 argument");
  memcell *arg = avm_get_actual(0);
  if (arg->type != table_m)
    avm_error("'object copy' expected table argument");
  avm_memcell_clear(&retval);
  retval.type = table_m;

  avm_table_incr_refcounter(retval.data.table_val);
}

void libfunc_strtonum() {
  char *i = avm_get_actual(0)->data.str_val;
  if (avm_get_actual(0)->type != string_m || (i != 0 && atof(i) == 0)) {
    retval.type = nil_m;
  } else {
    retval.data.num_val = atof(i);
    retval.type = number_m;
  }
}

void libfunc_sqrt() {
  double i = avm_get_actual(0)->data.num_val;
  if (i < 0)
    retval.type = nil_m;
  else
    retval.data.num_val = sqrt(i);
}

void libfunc_cos() {
  double i = avm_get_actual(0)->data.num_val;
  double rad = 3.141592654 * i / 180;
  retval.data.num_val = cos(rad);
}

void libfunc_sin() {
  double i = avm_get_actual(0)->data.num_val;
  double rad = 3.141592654 * i / 180;
  retval.data.num_val = sin(rad);
}

char *number_to_string(memcell *i) {
  assert(i->type >= 0 && i->type <= undef_m);
  // printf("GOTINNUMTOSTRIN\n\n");
  char *tmp = malloc(sizeof(char *));
  sprintf(tmp, "%.4f", i->data.num_val);
  return tmp;
}

char *string_to_string(memcell *i) {
  assert(i->type == string_m);
  return i->data.str_val;
}

char *bool_to_string(memcell *i) {
  assert(i->type == bool_m);
  if (i->data.bool_val == '0') {
    return "false";
  } else {
    return "true";
  }
}

// kai allo
char *table_to_string(memcell *m) {
  char *y;
  char *z;
  y = malloc(8 * sizeof(char *));
  z = malloc(sizeof(char *));
  for (int i = 0; i < AVM_HASHTABLE_SIZE; i++) {
    bucket *tmp = m->data.table_val->str_indexed[i];
    while (tmp != NULL) {
      strcat(y, "{\"");
      strcat(y, tmp->key.data.str_val);
      strcat(y, "\":");
      strcat(y, avm_to_string(&(tmp->value)));
      strcat(y, "} ");
      tmp = tmp->next;
    }

    tmp = m->data.table_val->num_indexed[i];
    while (tmp != NULL) {
      strcat(y, "{");
      strcat(y, z);
      strcat(y, ":");
      strcat(y, avm_to_string(&(tmp->value)));
      strcat(y, "} ");
      tmp = tmp->next;
    }
  }

  strcat(y, "]");
  return y;
}

char *userfunc_to_string(memcell *m) {
  char *x;
  x = malloc(sizeof(char *));
  sprintf(x, "User Function : %d",
          user_func_array[m->data.userfunc_val].address);
  return x;
}

char *libfunc_to_string(memcell *m) {
  char *str = "LibFunc";
  return str;
}

char *nil_to_string(memcell *m) {
  char *str = "Nil";
  return str;
}

char *undef_to_string(memcell *m) {
  char *str = "Undef";
  return str;
}

library_func_t avm_get_libfunc(char *id) {
  if (!strcmp("print", id))
    return libfunc_print;
  else if (!strcmp("input", id))
    return libfunc_input;
  else if (!strcmp("objectmemberkeys", id))
    return libfunc_objectmemberkeys;
  else if (!strcmp("objecttotalmembers", id))
    return libfunc_objecttotalmembers;
  else if (!strcmp("objectcopy", id))
    return libfunc_objectcopy;
  else if (!strcmp("totalarguments", id))
    return libfunc_totalarguments;
  /*
else if (!strcmp("argument", id))
  return libfunc_argument;
  */
  else if (!strcmp("typeof", id))
    return libfunc_typeof;
  else if (!strcmp("strtonum", id))
    return libfunc_strtonum;
  else if (!strcmp("sqrt", id))
    return libfunc_sqrt;
  else if (!strcmp("cos", id))
    return libfunc_cos;
  else if (!strcmp("sin", id))
    return libfunc_sin;
  else
    return 0;
  return 0;
}

void execute_pusharg(instr *in) {
  // printf("GOTINEXE_PUSHARG\n\n");
  //  printf("Arg value: %d\n", in->result.val);
  memcell *arg = avm_translate(&in->result, &ax);
  assert(arg);

  // printf("Arg value: %f\n", arg->data.num_val);
  // printf("Arg type: %d\n", arg->type);

  avm_assign(&avm_stack[top], arg);
  // printf("GOTOUT\n");
  ++total_actuals;
  avm_dec_top();
}

void execute_newtable(instr *in) {
  memcell *lv = avm_translate(&in->result, (memcell *)0);
  assert(lv && (&avm_stack[AVM_STACK_SIZE - 1] >= lv && lv > &avm_stack[top] ||
                lv == &retval));
  avm_memcell_clear(lv);

  lv->type = table_m;
  lv->data.table_val = avm_table_new();
  avm_table_incr_refcounter(lv->data.table_val);
}

void execute_nop(instr *in) { execution_finished = 1; }

void avm_initialize(void) {
  avm_init_stack();

  avm_register_libfunc("print", libfunc_print);
  avm_register_libfunc("typeof", libfunc_typeof);
  avm_register_libfunc("totalarguments", libfunc_totalarguments);
  // avm_register_libfunc("argument", libfunc_argument);
  avm_register_libfunc("input", libfunc_input);
  avm_register_libfunc("objectmemberkeys", libfunc_objectmemberkeys);
  avm_register_libfunc("objecttotalmembers", libfunc_objecttotalmembers);
  avm_register_libfunc("strtonum", libfunc_strtonum);
  avm_register_libfunc("cos", libfunc_cos);
  avm_register_libfunc("sin", libfunc_sin);
  avm_register_libfunc("sqrt", libfunc_sqrt);
  avm_register_libfunc("objectcopy", libfunc_objectcopy);
}

void avm_register_libfunc(char *id, library_func_t address) {
  // LibTable[avm_get_libfunc(id)] = address;
  h_count++;
}

void execute_cycle() {
  // printf("top: %d\n", top);
  // printf("pc: %d\n", pc);
  if (execution_finished) {
    // printf("GOTIN1\n");
    return;
  } else if (pc == AVM_ENDING_PC) {
    // printf("GOTIN2\n");
    execution_finished = 1;
    return;
  } else {
    // printf("GOTIN3\n");
    assert(pc < AVM_ENDING_PC);
    instr *in = code + pc;
    assert(in->opcode >= 0 && in->opcode <= AVM_MAX_INSTRUCTIONS);
    if (in->line) {
      curr_line = in->line;
    }
    unsigned old_pc = pc;
    (*execute_funcs[in->opcode])(in);
    if (pc == old_pc) {
      ++pc;
    }
  }
  // printf("GOTOuT\n");
}

int main() {
  avm_initialize();
  decodeBinaryFile();
  top = AVM_STACK_SIZE - 1 - globals_total;
  printf("\n========= Code Output =========\n\n");
  while (!execution_finished) {
    // printf("top address: %p\n", (void *)(&avm_stack[top]));
    execute_cycle();
  }
  printf("\n");
  return 0;
}
