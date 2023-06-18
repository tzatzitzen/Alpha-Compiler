#include "vm.h"

extern double *const_num_array;
extern unsigned const_num_size;

extern char **const_string_array;
extern unsigned const_string_size;

extern char **lib_func_array;
extern unsigned lib_func_size;

extern userfunc *user_func_array;
extern unsigned user_func_size;

extern instr *code;
extern unsigned code_size;

extern unsigned globals_total;

void print_decoded_arrays() {
  printf("-------------------------------------------\n");
  for (int i = 0; i < const_num_size; i++) {
    printf("pos: %d, num: %f\n", i, const_num_array[i]);
  }
  for (int i = 0; i < const_string_size; i++) {
    printf("pos: %d, string: %s\n", i, const_string_array[i]);
  }
  for (int i = 0; i < user_func_size; i++) {
    printf("pos: %d, user: %s, address: %d locals: %d\n", i,
           user_func_array[i].name, user_func_array[i].address,
           user_func_array[i].localSize);
  }
  for (int i = 0; i < lib_func_size; i++) {
    printf("pos: %d, lib: %s\n", i, lib_func_array[i]);
  }
}

void printInstructions(unsigned n) {
  int i = 0;
  printf("-------------------------------------------\n");
  for (i = 0; i < n; i++) {
    switch (code[i].opcode) {
    case assign_v:
      if (code[i].arg1.type == number_a) {
        printf("%d | assign | %d %d | %d %d:%d\n", i, code[i].result.type,
               code[i].result.val, code[i].arg1.type, code[i].arg1.val,
               (int)const_num_array[code[i].arg1.val]);
      } else if (code[i].arg1.type == string_a) {
        printf("%d | assign | %d %d | %d %d:\"%s\"\n", i, code[i].result.type,
               code[i].result.val, code[i].arg1.type, code[i].arg1.val,
               const_string_array[code[i].arg1.val]);
      } else if (code[i].arg1.type == userfunc_a) {
        printf("%d | assign | %d %d | %d %d:%s\n", i, code[i].result.type,
               code[i].result.val, code[i].arg1.type, code[i].arg1.val,
               user_func_array[code[i].arg1.val].name);
      } else if (code[i].arg1.type == libfunc_a) {
        printf("%d | assign | %d %d | %d %d:%s\n", i, code[i].result.type,
               code[i].result.val, code[i].arg1.type, code[i].arg1.val,
               lib_func_array[code[i].arg1.val]);
      } else if (code[i].arg1.type == retval_a) {
        printf("%d | assign | %d %d | %d\n", i, code[i].result.type,
               code[i].result.val, code[i].arg1.type);
      } else {
        if (code[i].result.type == retval_a) {
          printf("%d | assign | %d | %d %d\n", i, code[i].result.type,
                 code[i].arg1.type, code[i].arg1.val);
        } else {
          printf("%d | assign | %d %d | %d %d\n", i, code[i].result.type,
                 code[i].result.val, code[i].arg1.type, code[i].arg1.val);
        }
      }
      break;
    case add_v:
      printf("%d | add | %d %d | %d %d | %d %d\n", i, code[i].result.type,
             code[i].result.val, code[i].arg1.type, code[i].arg1.val,
             code[i].arg2.type, code[i].arg2.val);

      break;
    case sub_v:
      printf("%d | sub | %d %d | %d %d | %d %d\n", i, code[i].result.type,
             code[i].result.val, code[i].arg1.type, code[i].arg1.val,
             code[i].arg2.type, code[i].arg2.val);
      break;
    case mul_v:
      printf("%d | mul | %d %d | %d %d | %d %d\n", i, code[i].result.type,
             code[i].result.val, code[i].arg1.type, code[i].arg1.val,
             code[i].arg2.type, code[i].arg2.val);
      break;
    case div_v:
      printf("%d | div | %d %d | %d %d | %d %d\n", i, code[i].result.type,
             code[i].result.val, code[i].arg1.type, code[i].arg1.val,
             code[i].arg2.type, code[i].arg2.val);
      break;
    case mod_v:
      printf("%d | mod | %d %d | %d %d | %d %d\n", i, code[i].result.type,
             code[i].result.val, code[i].arg1.type, code[i].arg1.val,
             code[i].arg2.type, code[i].arg2.val);
      break;
    case jeq_v:
      printf("%d | if_eq | %d %d | %d %d | %d %d\n", i, code[i].result.type,
             code[i].result.val, code[i].arg1.type, code[i].arg1.val,
             code[i].arg2.type, code[i].arg2.val);
      break;
    case jne_v:
      printf("%d | if_noteq | %d %d | %d %d | %d %d\n", i, code[i].result.type,
             code[i].result.val, code[i].arg1.type, code[i].arg1.val,
             code[i].arg2.type, code[i].arg2.val);
      break;
    case jle_v:
      printf("%d | if_lesseq | %d %d | %d %d | %d %d\n", i, code[i].result.type,
             code[i].result.val, code[i].arg1.type, code[i].arg1.val,
             code[i].arg2.type, code[i].arg2.val);
      break;
    case jge_v:
      printf("%d | if_greatereq | %d %d | %d %d | %d %d\n", i,
             code[i].result.type, code[i].result.val, code[i].arg1.type,
             code[i].arg1.val, code[i].arg2.type, code[i].arg2.val);
      break;
    case jlt_v:
      printf("%d | if_less | %d %d | %d %d | %d %d\n", i, code[i].result.type,
             code[i].result.val, code[i].arg1.type, code[i].arg1.val,
             code[i].arg2.type, code[i].arg2.val);
      break;
    case jgt_v:
      printf("%d | if_greater | %d %d | %d %d | %d %d\n", i,
             code[i].result.type, code[i].result.val, code[i].arg1.type,
             code[i].arg1.val, code[i].arg2.type, code[i].arg2.val);
      break;
    case call_v:
      printf("%d | callfunc | %d %d\n", i, code[i].result.type,
             code[i].result.val);
      break;
    case pusharg_v:
      printf("%d | pusharg | %d %d\n", i, code[i].result.type,
             code[i].result.val);
      break;
    case funcenter_v:
      printf("%d | enterfunc | %d %d \n", i, code[i].result.type,
             code[i].result.val);
      break;
    case funcexit_v:
      printf("%d | exitfunc | %d %d\n", i, code[i].result.type,
             code[i].result.val);
      break;
    case newtable_v:
      printf("%d | tablecreate | %d %d\n", i, code[i].result.type,
             code[i].result.val);
      break;
    case tablegetelem_v:
      printf("%d | tablegetelement | %d %d | %d %d | %d %d\n", i,
             code[i].result.type, code[i].result.val, code[i].arg1.type,
             code[i].arg1.val, code[i].arg2.type, code[i].arg2.val);
      break;
    case tablesetelem_v:
      printf("%d | tablesetelement | %d %d | %d %d | %d %d\n", i,
             code[i].result.type, code[i].result.val, code[i].arg1.type,
             code[i].arg1.val, code[i].arg2.type, code[i].arg2.val);
      break;
    case nop_v:
      printf("%d | nop \n", i);
      break;
    case jmp_v:
      printf("%d | jump | %d %d\n", i, code[i].result.type, code[i].result.val);
      break;
    default:
      break;
    }
  }
}

void decodeBinaryFile() {
  FILE *f;
  f = fopen("target_to_bin.abc", "rb");
  assert(f);

  unsigned int magicnumber;
  fread(&magicnumber, sizeof(unsigned), 1, f);
  assert(magicnumber == 340200501);

  fread(&const_string_size, sizeof(unsigned), 1, f);
  const_string_array = malloc(const_string_size * sizeof(char *));
  for (unsigned i = 0; i < const_string_size; i++) {
    unsigned size;
    fread(&size, sizeof(unsigned), 1, f);
    const_string_array[i] = malloc(size * sizeof(char));
    fread((char *)const_string_array[i], sizeof(char), size, f);
  }

  fread(&const_num_size, sizeof(unsigned), 1, f);
  const_num_array = malloc(const_num_size * sizeof(double));
  for (unsigned i = 0; i < const_num_size; i++) {
    fread(&const_num_array[i], sizeof(double), 1, f);
  }
  fread(&user_func_size, sizeof(unsigned), 1, f);
  user_func_array = malloc(lib_func_size * sizeof(userfunc));
  for (unsigned i = 0; i < user_func_size; i++) {
    fread(&(user_func_array[i].address), sizeof(unsigned), 1, f);
    fread(&(user_func_array[i].localSize), sizeof(unsigned), 1, f);
    unsigned size;
    fread(&size, sizeof(unsigned), 1, f);
    user_func_array[i].name = malloc(size * sizeof(char));
    fread((char *)user_func_array[i].name, sizeof(char), size, f);
  }

  fread(&lib_func_size, sizeof(unsigned), 1, f);
  lib_func_array = malloc(lib_func_size * sizeof(char *));
  for (unsigned i = 0; i < lib_func_size; i++) {
    unsigned size;
    fread(&size, sizeof(unsigned), 1, f);
    lib_func_array[i] = malloc(size * sizeof(char));
    fread((char *)lib_func_array[i], sizeof(char), size, f);
  }

  fread(&code_size, sizeof(unsigned), 1, f);
  code = malloc(code_size * sizeof(instr));
  for (unsigned i = 0; i < code_size; i++) {
    fread(&(code[i].opcode), sizeof(avmop), 1, f);
    fread(&(code[i].result.type), sizeof(avmarg_t), 1, f);
    fread(&(code[i].result.val), sizeof(unsigned int), 1, f);
    fread(&(code[i].arg1.type), sizeof(avmarg_t), 1, f);
    fread(&(code[i].arg1.val), sizeof(unsigned int), 1, f);
    fread(&(code[i].arg2.type), sizeof(avmarg_t), 1, f);
    fread(&(code[i].arg2.val), sizeof(unsigned int), 1, f);
    fread(&(code[i].line), sizeof(unsigned int), 1, f);
  }
  fread(&globals_total, sizeof(unsigned), 1, f);

  print_decoded_arrays();

  printInstructions(code_size);

  // printf("globals: %d\n", globals_total);
  fclose(f);
}
