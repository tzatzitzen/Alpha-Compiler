
void printInstructions(unsigned n) {
  int i = 0;
  printf("---------------------------------------------------------------------"
         "----------------\n");
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
      } else { /*2nd assign*/
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
      printf("%d | enterfunc | %d %d\n", i, code[i].result.type,
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
