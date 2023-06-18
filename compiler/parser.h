/* A Bison parser, made by GNU Bison 3.8.2.  */

/* Bison interface for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015, 2018-2021 Free Software Foundation,
   Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <https://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* DO NOT RELY ON FEATURES THAT ARE NOT DOCUMENTED in the manual,
   especially those whose name start with YY_ or yy_.  They are
   private implementation details that can be changed or removed.  */

#ifndef YY_YY_COMPILER_PARSER_H_INCLUDED
# define YY_YY_COMPILER_PARSER_H_INCLUDED
/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token kinds.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    YYEMPTY = -2,
    YYEOF = 0,                     /* "end of file"  */
    YYerror = 256,                 /* error  */
    YYUNDEF = 257,                 /* "invalid token"  */
    IF = 258,                      /* IF  */
    ELSE = 259,                    /* ELSE  */
    WHILE = 260,                   /* WHILE  */
    FOR = 261,                     /* FOR  */
    FUNCTION = 262,                /* FUNCTION  */
    RETURN = 263,                  /* RETURN  */
    BREAK = 264,                   /* BREAK  */
    CONTINUE = 265,                /* CONTINUE  */
    LOCAL = 266,                   /* LOCAL  */
    TRUE = 267,                    /* TRUE  */
    FALSE = 268,                   /* FALSE  */
    NIL = 269,                     /* NIL  */
    SEMICOLON = 270,               /* SEMICOLON  */
    COMMA = 271,                   /* COMMA  */
    COLON = 272,                   /* COLON  */
    DOUBLE_COLON = 273,            /* DOUBLE_COLON  */
    LEFT_BRACKET = 274,            /* LEFT_BRACKET  */
    RIGHT_BRACKET = 275,           /* RIGHT_BRACKET  */
    ASSIGN = 276,                  /* ASSIGN  */
    OR = 277,                      /* OR  */
    AND = 278,                     /* AND  */
    DOUBLE_EQ = 279,               /* DOUBLE_EQ  */
    NOT_EQ = 280,                  /* NOT_EQ  */
    GREATER = 281,                 /* GREATER  */
    GREATER_EQ = 282,              /* GREATER_EQ  */
    LESSER = 283,                  /* LESSER  */
    LESSER_EQ = 284,               /* LESSER_EQ  */
    ADD = 285,                     /* ADD  */
    SUB = 286,                     /* SUB  */
    MULTI = 287,                   /* MULTI  */
    DIV = 288,                     /* DIV  */
    MOD = 289,                     /* MOD  */
    NOT = 290,                     /* NOT  */
    DOUBLE_ADD = 291,              /* DOUBLE_ADD  */
    DOUBLE_SUB = 292,              /* DOUBLE_SUB  */
    UMINUS = 293,                  /* UMINUS  */
    DOT = 294,                     /* DOT  */
    DOUBLE_DOT = 295,              /* DOUBLE_DOT  */
    LEFT_SQUARE_BRACKET = 296,     /* LEFT_SQUARE_BRACKET  */
    RIGHT_SQUARE_BRACKET = 297,    /* RIGHT_SQUARE_BRACKET  */
    LEFT_PARENTHESIS = 298,        /* LEFT_PARENTHESIS  */
    RIGHT_PARENTHESIS = 299,       /* RIGHT_PARENTHESIS  */
    ID = 300,                      /* ID  */
    INTEGER = 301,                 /* INTEGER  */
    REAL = 302,                    /* REAL  */
    STRING = 303                   /* STRING  */
  };
  typedef enum yytokentype yytoken_kind_t;
#endif
/* Token kinds.  */
#define YYEMPTY -2
#define YYEOF 0
#define YYerror 256
#define YYUNDEF 257
#define IF 258
#define ELSE 259
#define WHILE 260
#define FOR 261
#define FUNCTION 262
#define RETURN 263
#define BREAK 264
#define CONTINUE 265
#define LOCAL 266
#define TRUE 267
#define FALSE 268
#define NIL 269
#define SEMICOLON 270
#define COMMA 271
#define COLON 272
#define DOUBLE_COLON 273
#define LEFT_BRACKET 274
#define RIGHT_BRACKET 275
#define ASSIGN 276
#define OR 277
#define AND 278
#define DOUBLE_EQ 279
#define NOT_EQ 280
#define GREATER 281
#define GREATER_EQ 282
#define LESSER 283
#define LESSER_EQ 284
#define ADD 285
#define SUB 286
#define MULTI 287
#define DIV 288
#define MOD 289
#define NOT 290
#define DOUBLE_ADD 291
#define DOUBLE_SUB 292
#define UMINUS 293
#define DOT 294
#define DOUBLE_DOT 295
#define LEFT_SQUARE_BRACKET 296
#define RIGHT_SQUARE_BRACKET 297
#define LEFT_PARENTHESIS 298
#define RIGHT_PARENTHESIS 299
#define ID 300
#define INTEGER 301
#define REAL 302
#define STRING 303

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED
union YYSTYPE
{
#line 78 "compiler/parser.y"

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
	

#line 180 "compiler/parser.h"

};
typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;


int yyparse (void);


#endif /* !YY_YY_COMPILER_PARSER_H_INCLUDED  */
