%{
    #include "symtable.h"
    #include "lex_token.h"
	#include "intermediate.h"
	#include "target.h"


	enum ScopeType {
		NEITHER_ = 0, FUNCTION_ = 1, LOOP_ = 2 
	};

	enum ScopeType parent_scopes[MAX_SCOPE] = {};


    int yyerror(char* yaccProvidedMessage);
    int yylex(void);

    extern char* yytext;
    extern FILE* yyin;


    int blankVariable = 0;
    int haveAccess = 0;
    int isGlobal = 0;
    int variableScope = 0;
    int curr_scope = 0;
    int lambda_counter = 0;
	int foundEqual = 0;
	int PrimaryRuleManage = 0;
	struct stack *newentry = NULL;
	struct stack *jumpStack = NULL;
	bool functionRefFound;
	unsigned int loopcounter = 0;
	struct stack *loopstack = NULL;
	struct stack *contstack = NULL;
	unsigned int inLoop = 0;
	unsigned int breakFound = 0;	
	bool contFound = false;
	loop *loop_head= NULL;	
	bool inFunction = false;

    char *librariesFunctions[12] = {"print", "input", "objectmemberkeys", "objecttotalmembers", "objectcopy",
    "totalarguments", "argument", "typeof", "strtonum","sqrt", "cos", "sin" };
    const char* translate_scope_type(enum ScopeType scope_type) {
		switch(scope_type) {
		case NEITHER_: return "NEITHER";
		case FUNCTION_: return "FUNCTION";
		case LOOP_: return "LOOP";
		default:
			abort();
		}
	}
    
    // typedef struct alpha_token_t token;



    const char *getVariableType[5] = {
		"global variable", "local variable", "formal argument" , "user function", "library function"
	};

	enum ScopeType what_am_i_inside();
	void assert_in_func();
	void error_if_const(bool is_const);
	void insertArgument(const lex_token* token);
	char* insertFunction(const lex_token* token);
	bool insertVariable(const lex_token* token, int* pos);
	void scope_plus();
	void scope_minus();
	void scope_minus_hide();
	void checkGlobal(const lex_token* token);
	void checkLocal(const lex_token* token);
	void check_in_loop_br();
	void check_in_loop_co();
%}


%union {
	char* string;
	int integerValue;
	double	doubleValue;
	void* entry;
    struct lex_token *lex_token;
    int argc;
	bool is_ref_to_const;
	struct Expression *expr;
	struct Stmt_T *stmt;
	struct ForLoop *loops;
	struct functionCall *fcall;
	unsigned int unInt;
	struct SymbolTableEntry *symbol;
	
}

%start program

%token IF ELSE WHILE FOR FUNCTION RETURN BREAK 
	CONTINUE LOCAL TRUE FALSE NIL SEMICOLON 
	COMMA COLON DOUBLE_COLON LEFT_BRACKET 
	RIGHT_BRACKET

%right ASSIGN
%left OR
%left AND
%nonassoc DOUBLE_EQ NOT_EQ
%nonassoc GREATER GREATER_EQ LESSER LESSER_EQ 
%left ADD SUB
%left MULTI DIV MOD
%right NOT DOUBLE_ADD DOUBLE_SUB UMINUS
%left DOT DOUBLE_DOT
%left LEFT_SQUARE_BRACKET RIGHT_SQUARE_BRACKET
%left LEFT_PARENTHESIS RIGHT_PARENTHESIS


%token <lex_token> ID INTEGER REAL STRING
%type <fcall> methodcall normcall callsuffix

	 
%type <expr> expr primary assignexpr term const lvalue member elist call objectdef objectdeftemp indexed indexedelem idlist returnstmt 
%type <string> funcname 
%type <symbol> funcprefix funcdef 
%type <unInt> funcbody whilestart whilecond M N ifprefix elseprefix
%type <stmt> stmt forstmt whilestmt loopstmt stmts block ifstmt
%type <loops> forprefix
%%

program:	
		stmts{}



stmt:	 	expr SEMICOLON {resettemp();}
			 | ifstmt  {resettemp();}
			 | whilestmt { loop_head = loop_head->prev;resettemp();}
			 | forstmt {  loop_head = loop_head->prev;resettemp();}
			 | returnstmt {resettemp();}
			 | BREAK SEMICOLON {
				if (inLoop == 0 || inFunction) {
					printf("Error, break outside of loop\n");
					exit(0);
				}
				if(!loop_head->breaklist) {
					initList(&loop_head->breaklist, &loop_head->break_exists);
				}
				push(loop_head->breaklist, nextQuadLabel());
				emit(jump, NULL, NULL, NULL, 0, yylineno);
				resettemp();
				}
			 | CONTINUE SEMICOLON { 
				if (inLoop == 0 || inFunction) {
					printf("Error, break outside of loop\n");
					exit(0);
				}
				if(!loop_head->contlist) {
					initList(&loop_head->contlist, &loop_head->cont_exists);
				}
				push(loop_head->contlist, nextQuadLabel());
				emit(jump, NULL, NULL, NULL, 0, yylineno);
				resettemp();}
			 | block {resettemp();}
			 | funcdef {inFunction = false; resettemp();}
			 | SEMICOLON {resettemp();}

stmts:		stmt {$$ = $1;

}
		|stmts stmt {$$ = $1;
			;
		}


expr:	 	assignexpr { $$ = $1;}
		 	 | expr ADD expr {
					Entry_T sym = newtemp();
					$$ = newexpr(arithexpr_e);
					$$->sym = sym;
					emit(add, $$, $1, $3, nextQuadLabel(),yylineno);
			 		}
		 	 | expr SUB expr {
					Entry_T sym = newtemp();
					$$ = newexpr(arithexpr_e);
					$$->sym = sym;
					emit(sub, $$, $1, $3, nextQuadLabel(),yylineno);
			 		}
		 	 | expr MULTI expr {
					Entry_T sym = newtemp();
					$$ = newexpr(arithexpr_e);
					$$->sym = sym;
					emit(mul, $$, $1, $3, nextQuadLabel(),yylineno);
			 }
		 	 | expr DIV expr {
					Entry_T sym = newtemp();
					$$ = newexpr(arithexpr_e);
					$$->sym = sym;
					emit(divop, $$, $1, $3, nextQuadLabel(),yylineno);
			 }
		 	 | expr MOD expr {
					Entry_T sym = newtemp();
					$$ = newexpr(arithexpr_e);
					$$->sym = sym;
					emit(mod, $$, $1, $3, nextQuadLabel(),yylineno);
			 }
		 	 | expr GREATER expr {
                                        Entry_T sym = newtemp();
					$$ = newexpr(boolexpr_e);
					$$->sym = sym;
					emit(if_greater, NULL, $1, $3, nextQuadLabel() + 3, yylineno);
					emit(assign, $$, newexpr_constBool(0), NULL, nextQuadLabel(), yylineno);
					emit(jump, NULL, NULL, NULL, nextQuadLabel() + 2, yylineno);emit(assign, $$, newexpr_constBool(1), NULL, nextQuadLabel(), yylineno);
			 }
		 	 | expr GREATER_EQ expr {
                                        Entry_T sym = newtemp();
					$$ = newexpr(boolexpr_e);
					$$->sym = sym;
					emit(if_greatereq, NULL, $1, $3, nextQuadLabel() + 3, yylineno);
					emit(assign, $$, newexpr_constBool(0), NULL, nextQuadLabel(), yylineno);
					emit(jump, NULL, NULL, NULL, nextQuadLabel() + 2, yylineno);emit(assign, $$, newexpr_constBool(1), NULL, nextQuadLabel(), yylineno);
			 }
		 	 | expr LESSER expr {
					Entry_T sym = newtemp();
					$$ = newexpr(boolexpr_e);
					$$->sym = sym;
					emit(if_less, NULL, $1, $3, nextQuadLabel() + 3, yylineno);
					emit(assign, $$, newexpr_constBool(0), NULL, nextQuadLabel(), yylineno);
					emit(jump, NULL, NULL, NULL, nextQuadLabel() + 2, yylineno);emit(assign, $$, newexpr_constBool(1), NULL, nextQuadLabel(), yylineno);

			 }
		 	 | expr LESSER_EQ expr {

					Entry_T sym = newtemp();
					$$ = newexpr(boolexpr_e);
					$$->sym = sym;
					emit(if_lesseq, NULL, $1, $3, nextQuadLabel() + 3, yylineno);
					emit(assign, $$, newexpr_constBool(0), NULL, nextQuadLabel(), yylineno);
					emit(jump, NULL, NULL, NULL, nextQuadLabel() + 2, yylineno);emit(assign, $$, newexpr_constBool(1), NULL, nextQuadLabel(), yylineno);
			 }
		 	 | expr DOUBLE_EQ expr {

					Entry_T sym = newtemp();
					$$ = newexpr(boolexpr_e);
					$$->sym = sym;
					emit(if_eq, NULL, $1, $3, nextQuadLabel() + 3, yylineno);
					emit(assign, $$, newexpr_constBool(0), NULL, nextQuadLabel(), yylineno);
					emit(jump, NULL, NULL, NULL, nextQuadLabel() + 2, yylineno);emit(assign, $$, newexpr_constBool(1), NULL, nextQuadLabel(), yylineno);
			 }
		 	 | expr NOT_EQ expr {
					Entry_T sym = newtemp();
					$$ = newexpr(boolexpr_e);
					$$->sym = sym;
					emit(if_noteq, NULL, $1, $3, nextQuadLabel() + 3, yylineno);
					emit(assign, $$, newexpr_constBool(0), NULL, nextQuadLabel(), yylineno);
					emit(jump, NULL, NULL, NULL, nextQuadLabel() + 2, yylineno);emit(assign, $$, newexpr_constBool(1), NULL, nextQuadLabel(), yylineno);
			 }
		 	 | expr AND expr {
					Entry_T sym = newtemp();
					$$ = newexpr(boolexpr_e);
					$$->sym = sym;
					emit(andop, $$, $1, $3, nextQuadLabel(), yylineno);
			 }
		 	 | expr OR expr {
					Entry_T sym = newtemp();
					$$ = newexpr(boolexpr_e);
					$$->sym = sym;
					emit(orop, $$, $1, $3, nextQuadLabel(), yylineno);
			 }
		 	 | term {$$=$1;}

term:	 	LEFT_PARENTHESIS expr RIGHT_PARENTHESIS { $$ = $2; }
			 | SUB expr %prec UMINUS {check_arith($2);
				check_arith($2);
			 	Entry_T sym = newtemp();
				 $$ = newexpr(arithexpr_e);
                   		 $$->sym = sym;
                   		 emit(uminus,$$,$2,NULL,nextQuadLabel(),yylineno);
			 }
			 | NOT expr {
				$$ = newexpr(boolexpr_e);
				$$->sym = newtemp();
				emit(notop, $$, $2, NULL, nextQuadLabel(), yylineno);
			 }
			 | DOUBLE_ADD lvalue {
						check_arith($2);
						if($2->type == tableitem_e){ 
                                                    $$ = emit_iftableitem($2);
                                                    emit(add,$$,$$,newexpr_constNum(1),nextQuadLabel(),yylineno);
                                                    emit(tablesetelem,$2,$$,$2->index,nextQuadLabel(),yylineno);
                                                }else{ 
                                                    emit(add,$2,$2,newexpr_constNum(1),nextQuadLabel(),yylineno);
                                                    Entry_T sym = newtemp();
													$$ = newexpr(arithexpr_e);
                                                    $$->sym = sym;
                                                    emit(assign,$$,$2,NULL,nextQuadLabel(),yylineno);
                                                }
					}
			 | lvalue DOUBLE_ADD {			 
						check_arith($1);
						$$ = newexpr(var_e);
                                                $$->sym = newtemp();
                                                if($1->type == tableitem_e){ 
                                                    expr* value = emit_iftableitem($1);
                                                    emit(assign,$$,value,NULL,nextQuadLabel(),yylineno);
                                                    emit(add,value,value,newexpr_constNum(1),nextQuadLabel(),yylineno);
                                                    emit(tablesetelem,$1,value,$1->index,nextQuadLabel(),yylineno);
                                                }else{ 
                                                    emit(assign,$$,$1,NULL,nextQuadLabel(),yylineno);
                                                    emit(add,$1,$1,newexpr_constNum(1),nextQuadLabel(),yylineno);
                                                }
									}
	 		 | DOUBLE_SUB lvalue {						check_arith($2);
									if($2->type == tableitem_e){ 
                                            $$ = emit_iftableitem($2);
                                            emit(sub,$$,$$,newexpr_constNum(1),nextQuadLabel(),yylineno);
                                            emit(tablesetelem, $2, $$,$2->index,nextQuadLabel(),yylineno);
                                    }else{ 
                                     	emit(sub,$2,$2,newexpr_constNum(1),nextQuadLabel(),yylineno);
										Entry_T sym = newtemp();
                                        $$ = newexpr(arithexpr_e);
                                        $$->sym = sym;
                                    }
								}
	 		 | lvalue DOUBLE_SUB 
					{check_arith($1);
					$$ = newexpr(var_e);
                                    $$->sym = newtemp();

										if($1->type == tableitem_e){ 
                                            expr* value = emit_iftableitem($1);
                                            emit(assign,$$,value,NULL,nextQuadLabel(),yylineno);
                                            emit(sub,value,value,newexpr_constNum(1),nextQuadLabel(),yylineno);
                                            emit(tablesetelem,$1, value ,$1->index,nextQuadLabel(),yylineno);
                                        }
										else{ 
                                        	emit(assign,$$,$1,NULL,nextQuadLabel(),yylineno);
                                            emit(sub,$1,$1,newexpr_constNum(1),nextQuadLabel(),yylineno);
                                        }
								}
			 | primary {$$ = $1;}


assignexpr:	lvalue {}
		ASSIGN expr {
				if($1->type == tableitem_e) {
					emit(tablesetelem, $1, $1->index, $4, nextQuadLabel(), yylineno);
					$$ = emit_iftableitem($1);
					$$->type = assignexpr_e;
				} else {
				if($4 == NULL){

				}
					$1 = lvalue_expr($1->sym);
					emit(assign, $1, $4, NULL, nextQuadLabel(), yylineno);
					$$ = newexpr(assignexpr_e);
					$$->sym = newtemp();
					emit(assign, $$, $1, NULL, nextQuadLabel(), yylineno);
				}
			}


primary:	lvalue {$$ = emit_iftableitem($1);}
			 | call {$$ = $1;}
			 | objectdef {$$ = $1;}
			 | LEFT_PARENTHESIS  funcdef RIGHT_PARENTHESIS {$$ = newexpr(programfunc_e);
									$$->sym = $2;}
			 | const {$$ = $1;}

lvalue: 	ID {	int pos = -1; functionRefFound = insertVariable($1, &pos); 
			Entry_T sym = ScopeLookup($1->content, curr_scope);	
			if(pos != -1) {
			sym = ScopeLookup($1->content, pos);
			$$ = lvalue_expr(sym);
			}else {
            	Entry_T sym = ScopeLookup($1->content, curr_scope);
            	$$ = lvalue_expr(sym);    
            }
		}
			 | LOCAL ID { checkLocal($2); Entry_T sym = ScopeLookup($2->content, curr_scope);	
			$$ = lvalue_expr(sym);	}

			 | DOUBLE_COLON ID { checkGlobal($2); Entry_T sym = ScopeLookup($2->content, curr_scope);	
			$$ = lvalue_expr(sym);	}

			 | member {  $$ = $1;}

member: 	lvalue DOT ID {
				$$ = member_item($1, $3->content);
			}| lvalue LEFT_SQUARE_BRACKET expr RIGHT_SQUARE_BRACKET {
				$1 = emit_iftableitem($1);
				$$ = newexpr(tableitem_e);
				$$->sym = $1->sym;
				$$->index = $3;
			 }
			 | call DOT ID {}
			 | call LEFT_SQUARE_BRACKET expr RIGHT_SQUARE_BRACKET {}

call:		call LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {$$ = make_call($1, $3);}
			| lvalue callsuffix	{
					$1 = emit_iftableitem($1);
					if($2->method) {
						$1->next = NULL;
						$1 = emit_iftableitem(member_item($1, $2->name));

						expr* current = $2->elist;
						if (current != NULL) {
							while (current->next)
								current = current->next;
							current->next = $1;
						}
						else
							$2->elist = $1;

					}
					$$ = make_call($1, $2->elist);
			}			 		
			| LEFT_PARENTHESIS funcdef RIGHT_PARENTHESIS LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {
				expr* func = newexpr(programfunc_e);
				func->sym = $2;
				$$ = make_call(func, $5);
			}

callsuffix:		normcall { $$ = $1; }
				| methodcall { $$ = $1; }

normcall:		LEFT_PARENTHESIS elist RIGHT_PARENTHESIS {
				$$ = (fcall_t*)malloc(sizeof(struct functionCall));
				$$->elist = (expr*)malloc(sizeof(struct Expression));
				$$->elist = $2;
				$$->method = 0;
				$$->name = NULL;
				}

methodcall:		DOUBLE_DOT ID LEFT_PARENTHESIS elist RIGHT_PARENTHESIS { 
	  			printf("%s\n\n\n", $2->content);
				$$ = (fcall_t*)malloc(sizeof(struct functionCall));
				$$->elist = (expr*)malloc(sizeof(struct Expression));
				$$->elist = $4;
				$$->method = 1;
				$$->name = $2->content;
}

elist:		{$$ = NULL;}
		|expr	{ $$ = $1; $$->next = NULL; }
		| elist COMMA expr	{
			$$ = $1;
			expr * current = $$;
			while(current->next != NULL) {
				current = current->next;
			}
			current->next = $3;
			$3->next = NULL;
		}

objectdef:		LEFT_SQUARE_BRACKET objectdeftemp RIGHT_SQUARE_BRACKET {$$ =$2;}

objectdeftemp:	elist {
			expr* t = newexpr(newtable_e);
			t->sym =  newtemp();
			emit(tablecreate, t, NULL, NULL, nextQuadLabel(), yylineno);
			int i = 0;
			expr* current = $1;
			while(current != NULL) {
				emit(tablesetelem, t, newexpr_constNum(i++), current, nextQuadLabel(), yylineno);
				current = current->next;
			}
			$$ = t;
	     }
		|indexed {
			expr * t = newexpr(newtable_e);
			t->sym = newtemp();
			emit(tablecreate, t, NULL, NULL, nextQuadLabel(), yylineno);
			expr *current = $1;
			while(current != NULL) {
				emit(tablesetelem, t, current->index, current->value, nextQuadLabel(), yylineno);
				current = current->next;
			}
			$$ = $1;
		}

indexed:		indexed COMMA indexedelem {
				$$ = $1;
				expr* current = $1;
				while(current != NULL) {
					current = current->next;
				}
				current = $3;
       }
			|indexedelem { $$ = $1; }

indexedelem:	LEFT_BRACKET expr COLON expr RIGHT_BRACKET { 
			$$ = newexpr(tableitem_e);
			$$->index = $2;
			$$->value = $4;
}

block:			LEFT_BRACKET {if(inLoop > 0){inLoop++;}curr_scope++; checkScopeSize(curr_scope);} stmts RIGHT_BRACKET {$$ = $3;scope_minus_hide(); if(inLoop > 0){inLoop--;}}
			|LEFT_BRACKET {if(inLoop > 0){inLoop++;}curr_scope++; checkScopeSize(curr_scope);} RIGHT_BRACKET {scope_minus_hide(); if(inLoop > 0){inLoop--;}}	

			

funblockstart:	{push(loopstack, loopcounter); loopcounter = 0;}
funblockend:	{loopcounter = pop(loopstack);}
funcname:		ID{
					$$ = insertFunction($1);
				}
				|{
					$$ = insertFunction(NULL);
				}

funcprefix: FUNCTION funcname{
				inFunction = true;
				$$ = ScopeLookup($2,curr_scope);
				$$->isfunction->label = nextQuadLabel();
				struct stack *jumpSt = malloc(sizeof(struct stack));
				push(jumpSt, nextQuadLabel());
				$$->stackj = jumpSt;
				emit(jump, NULL, NULL, NULL, nextQuadLabel(),yylineno);
				emit(funcstart, lvalue_expr($$), NULL, NULL, nextQuadLabel(), yylineno);
				struct stack *a = malloc(sizeof(struct stack));
				push(a, currScopespaceOffset());
				$$->stackf = a;
				enterScopespace();
				resetFormalArgOffset();
				}

funcargs:		LEFT_PARENTHESIS {curr_scope++;} idlist RIGHT_PARENTHESIS {scope_minus(); parent_scopes[curr_scope + 1] = FUNCTION_; }{
														enterScopespace();
														resetFunctionLocalOffset();
														}

funcbody:	block{
				$$ = currScopespaceOffset();
				exitScopespace();
				}


funcdef:		funcprefix funcargs funblockstart funcbody funblockend {
						patchLabel(pop($$->stackj), nextQuadLabel()+1);
						exitScopespace();
						$1->isfunction->locals = $4;
						int offset = pop($$->stackf);
						restoreCurrScopeOffset(offset);
						$$ = $1;
						emit(funcend, lvalue_expr($1), NULL, NULL, nextQuadLabel(), yylineno);
					}

				
const:			INTEGER { $$ = newexpr_constNum($1->intVal); }
				| REAL {$$ = newexpr_constNum($1->doubleVal);}
				| STRING {$$ = newexpr_constString($1->stringVal);}
				| NIL {$$ = newexpr_nil();}
				| TRUE {$$ = newexpr_constBool(1);}
				| FALSE{$$ = newexpr_constBool(0);}

idlist:			idlist COMMA ID { insertArgument($3);}
				| ID { insertArgument($1);}
				| {}

ifstmt:			ifprefix stmt {
					patchLabel($1, nextQuadLabel());
				} 
			|ifprefix stmt elseprefix stmt {
					
				patchLabel($1, $3 + 1);
				patchLabel($3, nextQuadLabel());
			}

ifprefix:		IF LEFT_PARENTHESIS expr RIGHT_PARENTHESIS {
				emit(if_eq, NULL, $3, newexpr_constBool(1), nextQuadLabel() + 2, yylineno);
				$$ = nextQuadLabel();
				emit(jump, NULL, NULL, NULL, 0, yylineno);
			}

elseprefix:		ELSE	 {$$ = nextQuadLabel(); emit(jump, NULL, NULL, NULL, 0, yylineno);}


loopstart:	{
		if(!loop_head) {
			loop_head = malloc(sizeof(struct LoopStruct));
			loop_head->breaklist = NULL;
			loop_head->break_exists = false;
			loop_head->contlist = NULL;
			loop_head->cont_exists = false;
			loop_head->next = NULL;
			loop_head->prev = NULL;
		} else {
			while(loop_head->next) {
				loop_head = loop_head->next;
			}
			loop_head->next = malloc(sizeof(struct LoopStruct));
			loop_head->next->breaklist = NULL;
			loop_head->break_exists = false;
			loop_head->next->contlist = NULL;
			loop_head->cont_exists = false;
			loop_head->next->next = NULL;
			loop_head->next->prev = loop_head;
			loop_head = loop_head->next;
		}
		++loopcounter;}
loopend:	{--loopcounter;}
loopstmt:	loopstart stmt loopend {}

whilestart:		WHILE {inLoop++; $$ = nextQuadLabel();}

whilecond:		LEFT_PARENTHESIS expr RIGHT_PARENTHESIS {
				emit(if_eq, NULL, $2, newexpr_constBool(1), nextQuadLabel() + 2, yylineno); 
				$$ = nextQuadLabel();
				emit(jump, NULL, NULL, NULL, nextQuadLabel(), yylineno);

	}

whilestmt:		whilestart whilecond loopstmt {
				emit(jump, NULL, NULL, NULL, $1, yylineno);
				patchLabel($2, nextQuadLabel());
				while(loop_head->break_exists && loop_head->breaklist->top) {
					patchlist(pop(loop_head->breaklist), nextQuadLabel());
				}
				while(loop_head->cont_exists && loop_head->contlist->top) {
					patchlist(pop(loop_head->contlist), $1);
				}
			}

N:              { $$ = nextQuadLabel(); emit(jump,NULL,NULL,NULL,0,yylineno); }
M:              { $$ = nextQuadLabel(); }

forprefix:	FOR LEFT_PARENTHESIS elist SEMICOLON M expr SEMICOLON{
			inLoop++;
			$$ = malloc(sizeof(struct ForLoop));
			$$->test = $5;
			$$->enter = nextQuadLabel();
			emit(if_eq, NULL, $6, newexpr_constBool(1), 0, yylineno);
	 }

forstmt:	forprefix N elist RIGHT_PARENTHESIS N {parent_scopes[curr_scope + 1] = LOOP_;} loopstmt N {
			patchLabel($1->enter, $5 + 1);
			patchLabel($2, nextQuadLabel());
			patchLabel($5, $1->test);
			patchLabel($8, $2 + 1);	
			while(loop_head->break_exists && loop_head->breaklist->top) {
					patchlist(pop(loop_head->breaklist), nextQuadLabel());
			}
			while(loop_head->cont_exists && loop_head->contlist->top) {
				patchlist(pop(loop_head->contlist), $2+1);
			}
		}


returnstmt:		RETURN SEMICOLON { assert_in_func(); 
						emit(ret, NULL, NULL, NULL, nextQuadLabel(),yylineno);
}
			|RETURN expr SEMICOLON {assert_in_func();
						emit(ret, NULL, $2, NULL, nextQuadLabel(),yylineno);
						$$ = $2;
			}
        
%%

enum ScopeType what_am_i_inside() {
	for (int scope_idx = curr_scope; scope_idx > 0; scope_idx--)
		if (parent_scopes[scope_idx] != NEITHER_)
			return parent_scopes[scope_idx];
	return NEITHER_;
}

void check_in_loop_br(){
	if(what_am_i_inside() != LOOP_){
		printf("Error : break outside loop function\n");
	}
}

void check_in_loop_co(){
	if(what_am_i_inside() != LOOP_){
		printf("Error : continue outside loop function\n");
	}
}

void assert_in_func() {
	for (int scope = curr_scope; scope >= 0; scope--)
		if (is_scope_function(scope))
			return;
	printf("Error: line %d return statement not in function\n", yylineno);
	
}

void scope_plus() {
	if (curr_scope >= MAX_SCOPE - 1) {
		printf("Max scope reached\n");
		abort();
	}
	curr_scope++;
}

void scope_minus() {
	parent_scopes[curr_scope] = NEITHER_;
	if (curr_scope == 0)
		printf("Error: line %d dropped below scope 0\n", yylineno);
	else
		curr_scope--;
}

void scope_minus_hide() {
	hide_scope(curr_scope);
	scope_minus();
}

void error_if_const(bool is_const) {
	if (is_const) {
		printf("Error: line %d cannot write to const symbol (function)\n", yylineno);
	}
}

void insertArgument(const lex_token* token){
	const char* name = token->content;
	Entry_T found;
	found = ScopeLookup(name, curr_scope);
	if (found != NULL) {
		printf("Error: line %i formal redeclaration\n", yylineno);
		return;
	}
	found = ScopeLookupLibraryFunction(name);
	if (found != NULL) {
		printf("Error: line %i formal shadows libfunc\n", yylineno);
		return;
	}
	SymbolTable_insert(token->content, curr_scope, token->line_num, ST_FORMAL, currScopespace(), currScopespaceOffset());
			inCurrScopespaceOffset();
}

char* insertFunction(const lex_token* token){
	// anonym function
	if (token == NULL) {
		char *buffer = malloc(20 * sizeof(char));
		sprintf(buffer, "%s%d", "$", lambda_counter++);
		SymbolTable_insert(buffer, curr_scope, yylineno, ST_USERFUNC, currScopespace(), currScopespaceOffset());
		return buffer;
	}

	const char* func_name = token->content;
		Entry_T any = ScopeLookup(func_name, curr_scope);
		if (any != NULL) {
			printf("Error: Function %s defined at line %u is a redefinition of lvalue "
				"defined at line %u in scope %u\n", func_name, token->line_num, 
				any->line, any->scope); 
			exit(0);
		}
		any = ScopeLookupLibraryFunction(func_name);
		if (any != NULL) {
			printf("Error: Cannot shadow library function!");
		}else{
			SymbolTable_insert(token->content, curr_scope, token->line_num, ST_USERFUNC, currScopespace(), currScopespaceOffset());
		return token->content;
		}
}

// returns true only if token is a reference to a function
bool insertVariable(const lex_token* token, int * pos) {
	const char* var_name = token->content;
	bool in_function = false;
	for (int scope = curr_scope; scope >= 1; scope--) {
		Entry_T any = ScopeLookup(var_name, scope);
		if (any != NULL && any->type == ST_USERFUNC) {
			*pos =(int) any->scope;
			return true;
		}
		if (any != NULL) {
			if (in_function) {
				printf("Error: at line %u, cannot access token %s defined at line %u "
					"in scope %u\n", token->line_num, var_name, any->line, any->scope);
			}
			return false;
		}
		if (is_scope_function(scope))
			in_function = true;
	}
	Entry_T any = ScopeLookup(var_name, 0);
	if (any != NULL){
			*pos =(int) any->scope;
		return any->type == ST_USERFUNC || any->type == ST_LIBFUNC;
	} else {
		SymbolTable_insert(token->content, curr_scope, token->line_num, 
			curr_scope == 0 ? ST_GLOBAL : ST_LOCAL, currScopespace(), currScopespaceOffset());
			inCurrScopespaceOffset();
	}
	return false;
}

void checkGlobal(const lex_token* token) {
	const char* var_name = token->content;
	if (ScopeLookup(var_name, 0) == NULL) {
		printf("Error: No global variable named %s found\n", var_name);
		
	}
}

void checkLocal(const lex_token* token) {
	const char* var_name = token->content;
	if(ScopeLookup(var_name, curr_scope) != NULL) {
		return;
	}
	if (ScopeLookupLibraryFunction(var_name) != NULL) {
		printf("Error: cannot shadow libfunc");
		return;
	}
	SymbolTable_insert(token->content, curr_scope, token->line_num, curr_scope == 0 ? ST_GLOBAL : ST_LOCAL, currScopespace(), currScopespaceOffset());
			inCurrScopespaceOffset();
}

int yyerror(char* yaccProvidedMessage) {
    fprintf(stderr,"%s: at line %d, before token: %s\n",yaccProvidedMessage,yylineno,yytext);
    fprintf(stderr,"INPUT NOT VALID\n");
}

void init() {
	memset(parent_scopes, 0, sizeof(enum ScopeType) * MAX_SCOPE);
}


int main(int argc,char** argv) {
    if (argc != 2) {
        printf("Expected 1 argument.\nRun as: %s <alpha-src-file>\n", argv[0]);
        return 1;
    }
    if ((yyin = fopen(argv[1],"r")) == NULL) {
        fprintf(stderr,"Cannot read file: %s\n",argv[1]);
        return 1;
    }
	loopstack = malloc(sizeof(struct stack));
	init();
    symtable = SymbolTable_new();
    libfunctions_init();
    yyparse();
    printTable();
	printQuads();
	generate_all();
	serialize(getGlobalsNum());

    return 0;
}
