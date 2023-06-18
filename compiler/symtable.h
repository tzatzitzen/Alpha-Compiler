#ifndef _symtable_
#define _symtable_

#define SIZE 100
#define MAX_SCOPE 1000
#define STACK_MAX_SIZE 1000

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct SymbolTableEntry;
struct SymbolTable;
struct isFunction;

// typedef struct Variable Variable;
// typedef struct Function Function;
typedef struct SymbolTableEntry *Entry_T;
typedef struct SymbolTable *SymbolTable_T;

/*truct Variable {
    const char *nameVar;
    unsigned int scope;
    unsigned int line;

}; */

/*struct Function {
    const char *nameFunc;
    unsigned int scope;
    unsigned int line;
}; */

typedef enum SymbolType {
  ST_GLOBAL,
  ST_LOCAL,
  ST_FORMAL,
  ST_USERFUNC,
  ST_LIBFUNC
} Type;

typedef enum Scopespace { programvar, functionlocal, formalarg } scopespace_t;

struct stack {
  int data[STACK_MAX_SIZE];
  int top;
};

struct SymbolTableEntry {
  bool isActive;
  const char *name;
  scopespace_t space;
  unsigned int scope;
  unsigned int line;
  unsigned int offset;
  // union {
  //   Variable *varVal;
  //   Function *funcVal;
  // }  value;
  Type type;
  struct SymbolTableEntry *next_in_scope;
  struct SymbolTableEntry *next_in_collision;
  struct isFunction *isfunction;
  struct stack *stackf;
  struct stack *stackj;
};

struct SymbolTable {
  bool scope_is_func[MAX_SCOPE];
  Entry_T scope_list[MAX_SCOPE];
  unsigned int count;
  Entry_T *items; // buckets
  // Entry_T* scope_list;    //0 ,1 ,2
  unsigned int scope_size;
};

typedef struct returnlist {
  unsigned label;
  struct returnlist *next;
} returnl;

struct isFunction {
  unsigned int label;
  unsigned int locals;
  unsigned int taddress;
  returnl *return_list;
};

extern SymbolTable_T symtable;
extern int curr_scope;
extern int yylineno;

void checkScopeSize(int size);
const char *getType(Type type);
SymbolTable_T SymbolTable_new(void);
void SymbolTable_insert(const char *name, int scope, int line, Type type,
                        scopespace_t scopespace, unsigned int offset);
Entry_T ScopeLookupFunction(const char *name, int scope);
Entry_T ScopeLookupVariable(const char *name, int scope);
Entry_T ScopeLookupLibraryFunction(const char *name);
Entry_T ScopeLookup(const char *name, int scope);
Entry_T GlobalLookup(const char *name);
void libfunctions_init();
void hide_scope(int scope);
bool is_scope_function(int scope);
void printElement(Entry_T current);
void printTable();
unsigned getGlobalsNum();

#endif
