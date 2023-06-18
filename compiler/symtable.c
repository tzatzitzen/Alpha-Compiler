#include "symtable.h"

SymbolTable_T symtable;

const char *getType(Type type) {
  switch (type) {
  case ST_GLOBAL:
    return "ST_GLOBAL";
    break;
  case ST_LOCAL:
    return "ST_LOCAL";
    break;
  case ST_FORMAL:
    return "ST_FORMAL";
    break;
  case ST_USERFUNC:
    return "ST_USERFUNC";
    break;
  case ST_LIBFUNC:
    return "ST_LIBFUNC";
    break;
  }
  return "not found";
}

static unsigned int hash_function(const char *key) {
  unsigned int i = 0;
  int j;
  for (j = 0; key[j]; j++) {
    i += key[j];
  }
  return i % SIZE;
}

void checkScopeSize(int size) {
  if (size > symtable->scope_size) {
    symtable->scope_size = size;
  }
}

SymbolTable_T SymbolTable_new(void) {
  SymbolTable_T new_st = (SymbolTable_T)malloc(sizeof(struct SymbolTable));
  if (new_st == NULL) {
    exit(1);
  }
  memset(new_st->scope_is_func, 0, MAX_SCOPE);
  memset(new_st->scope_list, 0, MAX_SCOPE * sizeof(Entry_T));
  new_st->items = (Entry_T *)malloc(SIZE * sizeof(struct SymbolTableEntry *));
  if (new_st->items == NULL) {
    exit(1);
  }
  int i;
  for (i = 0; i < SIZE; i++) {
    new_st->items[i] = NULL;
  }

  // new_st->scope_list = calloc(1, sizeof(sizeof(struct SymbolTableEntry*)));
  new_st->count = 0;
  new_st->scope_size = 0;

  return new_st;
}

Entry_T Entry_new(const char *name, int scope, int line, Type type,
                  scopespace_t scopespace, unsigned int offset) {
  Entry_T new_entry = (Entry_T)malloc(sizeof(struct SymbolTableEntry));
  if (new_entry == NULL) {
    printf("Malloc fail\n");
    exit(1);
  }
  new_entry->isActive = true;
  new_entry->type = type;
  new_entry->next_in_scope = NULL;
  new_entry->next_in_collision = NULL;
  new_entry->name = strdup(name);
  new_entry->scope = scope;
  new_entry->line = line;
  new_entry->space = scopespace;
  new_entry->offset = offset;

  return new_entry;
}

void SymbolTable_insert(const char *name, int scope, int line, Type type,
                        scopespace_t scopespace, unsigned int offset) {
  if (scope >= MAX_SCOPE) {
    printf("Error: Max scope %d reached.\n", MAX_SCOPE - 1);
    exit(1);
  }
  Entry_T current, new_entry;

  new_entry = Entry_new(name, scope, line, type, scopespace, offset);
  if (type == ST_USERFUNC) {
    symtable->scope_is_func[scope + 1] = true;
    new_entry->isfunction = malloc(sizeof(struct isFunction));
    new_entry->isfunction->return_list = NULL;
  }
  int index = hash_function(name);
  current = symtable->items[index];
  /* insert in hashtable */
  if (current == NULL) {
    symtable->items[index] = new_entry;
  } else {
    while (current->next_in_collision != NULL) {
      current = current->next_in_collision;
    }
    current->next_in_collision = new_entry;
  }

  if (symtable->scope_list[scope] == NULL) {
    symtable->scope_list[scope] = new_entry;
  } else {
    current = symtable->scope_list[scope];
    while (current->next_in_scope != NULL)
      current = current->next_in_scope;
    current->next_in_scope = new_entry;
  }
}

Entry_T ScopeLookupFunction(const char *name, int scope) {
  Entry_T current =
      scope > symtable->scope_size ? NULL : symtable->scope_list[scope];
  while (current != NULL) {
    if ((current->type == ST_USERFUNC || current->type == ST_LIBFUNC) &&
        current->isActive && strcmp(current->name, name) == 0)
      break;
    current = current->next_in_scope;
  }
  if (current != NULL && !current->isActive) {
    return NULL;
  }
  return current;
}

Entry_T ScopeLookupLibraryFunction(const char *name) {
  int scope = 0;
  Entry_T current =
      scope > symtable->scope_size ? NULL : symtable->scope_list[scope];
  while (current != NULL) {
    if (current->type == ST_LIBFUNC && strcmp(current->name, name) == 0)
      break;
    current = current->next_in_scope;
  }
  if (current != NULL && !current->isActive) {
    return NULL;
  }
  return current;
}

Entry_T ScopeLookupVariable(const char *name, int scope) {
  Entry_T current =
      scope > symtable->scope_size ? NULL : symtable->scope_list[scope];
  while (current != NULL) {
    if ((current->type == ST_GLOBAL || current->type == ST_LOCAL ||
         current->type == ST_FORMAL) &&
        current->isActive && strcmp(current->name, name) == 0)
      break;
    current = current->next_in_scope;
  }
  if (current != NULL && !current->isActive) {
    return NULL;
  }
  return current;
}
Entry_T ScopeLookup(const char *name, int scope) {
  Entry_T current = symtable->scope_list[scope];
  while (current != NULL) {
    if (strcmp(current->name, name) == 0 && current->isActive) {
      return current;
    }
    current = current->next_in_scope;
  }
  return NULL;
}
Entry_T ST_GLOBALLookup(const char *name) { return ScopeLookup(name, 0); }

void hide_scope(int scope) {
  Entry_T current = symtable->scope_list[scope];
  while (current != NULL) {
    current->isActive = false;
    current = current->next_in_scope;
  }
}

bool is_scope_function(int scope) { return symtable->scope_is_func[scope]; }

void printElement(Entry_T current) {
  printf("name: %s scope: %d line: %d type: %s\n", current->name,
         current->scope, current->line, getType(current->type));
}

void printTable() {
  Entry_T current;
  printf("--- Symbol Table -------------------------------------\n");
  for (int i = 0; i < SIZE; i++) {
    current = symtable->items[i];
    while (current != NULL) {
      printElement(current);
      current = current->next_in_collision;
    }
  }
  printf("--- Scope Lists -------------------------------------\n");
  for (int i = 0; i <= symtable->scope_size; i++) {
    printf("--- Scope %d ---------------------------------------\n", i);
    printf(" Name                    | Line | Type        \n");
    current = symtable->scope_list[i];
    while (current != NULL) {
      Entry_T c = current;
      printf("%-25s|%-6u|%s\n", c->name, c->line, getType(c->type));
      // printElement(current);
      current = current->next_in_scope;
    }
  }
}

unsigned getGlobalsNum() {
  Entry_T current = symtable->scope_list[0];
  unsigned num = 0;
  while (current != NULL) {
    if (current->type == ST_GLOBAL) {
      num++;
    }
    current = current->next_in_scope;
  }
  return num;
}
