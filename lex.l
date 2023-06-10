%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <assert.h>
    #include <stdbool.h>
    #include "lex_token.h"
    #include "parser.h"
    
    enum TokenType {
        KEYWORD, OPERATOR, CONST_INT, CONST_REAL, STRING_TYPE, PUNCTUATION, ID_TYPE, COMMENT_TYPE, ERROR
    };
    
    struct lex_old_token {                  
        int lineNumber; //Represents the line where the token is found
        int tokenCounter; //Counts how many tokens have been found
        char* tokenContent;  //Represents the content of the token
        enum TokenType tokenType; //Represents the type of the token

        char* tokenName;
        char* tokenTypeOfValue;

        struct lex_old_token* next;
    };
    

int tokenCount = 1;
lex_token xval; 
struct lex_old_token *head = NULL;
struct lex_old_token *tmp = NULL;



char *string_check(int line_number);
void token_link(void* x);
const char* get_type(enum TokenType token_type);
int alpha_yylex(void* ylval);
void set_token(int line_number, int token_counter, char* token_content,enum TokenType tokenType, char* caps, char* token_type);
void consume_block_comment(int line);
%}


%option yylineno
%option noyywrap

%x COMMENT


IF          "if"
ELSE        "else"
WHILE       "while"
FOR         "for"
FUNCTION    "function"
RETURN      "return"
BREAK       "break"
CONTINUE    "continue"
AND         "and"
NOT         "not"
OR          "or"
LOCAL       "local"
TRUE        "true"
FALSE       "false"
NIL         "nil"

ASSIGN       "="
ADD          "+"
SUB          "-"
MULTI        "*"
DIV          "/"
MOD          "%"
DOUBLE_EQ    "=="
NOT_EQ       "!="
DOUBLE_ADD   "++"
DOUBLE_SUB   "--"
GREATER      ">"
LESSER       "<"
GREATER_EQ   ">="
LESSER_EQ    "<="

INTEGER      [0-9]+

REAL          [0-9]+\.[0-9]+

STRING       "\""

LEFT_BRACKET \{
RIGHT_BRACKET \}
LEFT_SQUARE_BRACKET \[
RIGHT_SQUARE_BRACKET \]
LEFT_PARENTHESIS   \(
RIGHT_PARENTHESIS  \)

ID          [a-zA-Z][a-zA-Z0-9_]*

SEMICOLON ";"
COMMA ","
COLON ":"
DOUBLE_COLON "::"
DOT "."
DOUBLE_DOT ".."

LINE_COMMENT      "//".*
NEW_LINE        "\n"
SPACE           " "
TAB             "\t"
CR "\r"


%%



{IF}        {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"IF","enumerated"); */    return IF;}
{ELSE}      {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"ELSE","enumerated"); */ return ELSE;}
{WHILE}     {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"WHILE","enumerated"); */    return WHILE;}
{FOR}       {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"FOR","enumerated"); */  return FOR;}
{FUNCTION}  {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"FUNCTION","enumerated"); */ return FUNCTION;}
{RETURN}    {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"RETURN","enumerated"); */   return RETURN;}
{BREAK}     {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"BREAK","enumerated"); */    return BREAK;}
{CONTINUE}  {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"CONTINUE","enumerated"); */ return CONTINUE;}
{AND}       {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"AND","enumerated"); */  return AND;}
{NOT}       {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"NOT","enumerated"); */  return NOT;}
{OR}        {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"OR","enumerated"); */   return OR;}
{LOCAL}     {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"LOCAL","enumerated"); */    return LOCAL;}
{TRUE}      {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"TRUE","enumerated"); */ return TRUE;}
{FALSE}     {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"FALSE","enumerated"); */    return FALSE;}
{NIL}       {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,KEYWORD,"NIL","enumerated"); */  return NIL;}

{ASSIGN}     {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"ASSIGN","enumerated"); */   return ASSIGN;}
{ADD}        {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"ADD","enumerated"); */  return ADD;}
{SUB}        {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"SUB","enumerated"); */  return SUB;}
{MULTI}      {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"MULTI","enumerated"); */    return MULTI;}
{DIV}        {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"DIV","enumerated"); */  return DIV;}
{MOD}        {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"MOD","enumerated"); */  return MOD;}
{DOUBLE_EQ}  {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"DOUBLE_EQ","enumerated"); */    return DOUBLE_EQ;}
{NOT_EQ}     {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"NOT_EQ","enumerated"); */   return NOT_EQ;}
{DOUBLE_ADD} {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"DOUBLE_ADD","enumerated"); */  return DOUBLE_ADD;}
{DOUBLE_SUB} {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"DOUBLE_SUB","enumerated"); */  return DOUBLE_SUB;}
{GREATER}    {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"GREATER","enumerated"); */  return GREATER;}
{LESSER}     {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"LESSER","enumerated"); */   return LESSER;}
{GREATER_EQ} {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"GREATER_EQ","enumerated"); */  return GREATER_EQ;}
{LESSER_EQ}  {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,OPERATOR,"LESSER_EQ","enumerated"); */    return LESSER_EQ;}

{INTEGER}   {   
                xval.line_num = yylineno;
                xval.token_num = tokenCount;
                xval.content = yytext;
                xval.type = INT_TKN_TYPE;
                yylval.lex_token = &xval;
                xval.intVal = atoi(yytext);
                // set_token(yylineno,tokenCount,yytext,CONST_INT,yytext,"integer"); 
                return INTEGER;
            }

{REAL}      { 
                xval.line_num = yylineno;
                xval.token_num = tokenCount;
                xval.content = yytext;
                xval.type = REAL_TKN_TYPE;
                yylval.lex_token = &xval;
                xval.doubleVal = atof(yytext);
                // set_token(yylineno,tokenCount,yytext,CONST_REAL,yytext,"integer");  
                return REAL;
            }

{STRING}    {           
                int line = yylineno;
                char *string = string_check(yylineno);
                xval.line_num = line;
                xval.token_num = tokenCount;
                xval.content = string;
                xval.type = STR_TKN_TYPE;
                yylval.lex_token = &xval;
                xval.stringVal = string;


                // set_token(yylineno,tokenCount,string,STRING_TYPE,string,"char*");
                return STRING;
            }

{LEFT_BRACKET}          {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"LEFT_BRACKET","enumerated"); */ return LEFT_BRACKET;}
{RIGHT_BRACKET}         {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"RIGHT_BRACKET","enumerated"); */    return RIGHT_BRACKET;}
{LEFT_SQUARE_BRACKET}   {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"LEFT_SQUARE_BRACKET","enumerated"); */  return LEFT_SQUARE_BRACKET;}
{RIGHT_SQUARE_BRACKET}  {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"RIGHT_SQUARE_BRACKET","enumerated"); */ return RIGHT_SQUARE_BRACKET;}
{LEFT_PARENTHESIS}      {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"LEFT_PARENTHSESIS","enumerated"); */ return LEFT_PARENTHESIS;}
{RIGHT_PARENTHESIS}     {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"RIGHT_PARENTHESIS","enumerated"); */ return RIGHT_PARENTHESIS;}
{SEMICOLON}             {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"SEMICOLON","enumerated"); */    return SEMICOLON;}
{COMMA}                 {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"COMMA","enumerated"); */     return COMMA;}
{COLON}                 {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"COLON","enumerated"); */    return COLON; }
{DOUBLE_COLON}          {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"DOUBLE_COLON","enumerated"); */ return DOUBLE_COLON; }
{DOT}                   {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"DOT","enumerated"); */  return DOT; }
{DOUBLE_DOT}            {yylval.lex_token = NULL; /* set_token(yylineno,tokenCount,yytext,PUNCTUATION,"DOUBLE_DOT","enumerated"); */   return DOUBLE_DOT; }

{ID}                    {
                            xval.line_num = yylineno;
                            xval.token_num = tokenCount;
                            xval.content = yytext;
                            xval.type = ID_TKN_TYPE;
                            yylval.lex_token = &xval;

                            // set_token(yylineno,tokenCount,yytext,ID_TYPE,yytext,"char*");
                            return ID; 
                        }

{NEW_LINE}              
{SPACE}
{TAB}
{LINE_COMMENT}          {/*printf("%d: #%d \"\" COMMENT LINE_COMMENT <-enumerated\n",yylineno, tokenCount++); */ }

"/*"                  { BEGIN(COMMENT); }
<COMMENT>"*/"         { BEGIN(INITIAL); }
<COMMENT>.|\n         {}

{CR}
.                       {fprintf(stderr,"Cannot match character '%s' in line %d with any rule\n",yytext,yylineno); exit(1); }

%%



const char* get_type(enum TokenType token_type) {
  switch (token_type) {
    case KEYWORD:
      return "KEYWORD";
    case OPERATOR:
      return "OPERATOR";
    case CONST_INT:
        return "CONST_INT";
    case CONST_REAL:
        return "CONST_REAL";
    case STRING_TYPE: 
        return "STRING";
    case PUNCTUATION:
        return "PUNCTUATION";
    case ID_TYPE: 
        return "ID";
    case COMMENT_TYPE:
        return "COMMENT";
    default:
        assert(0);
  }
}


void set_token(int line_number, int token_counter, char* token_content,enum TokenType tokenType, char* caps, char* token_type){
    
    struct lex_old_token* new_token = (struct lex_old_token*)malloc(sizeof(struct lex_old_token));

    new_token->lineNumber = line_number; 
    new_token->tokenCounter = token_counter;
    new_token->tokenContent = strdup(token_content);
    new_token->tokenType = tokenType;
    new_token->tokenTypeOfValue = token_type;
    new_token->next = NULL;
    tokenCount++;
    // printf("%d: #%d \"%s\" %s %s <-%s\n",new_token->lineNumber, new_token->tokenCounter, new_token->tokenContent,get_type(new_token->tokenType),caps,new_token->tokenTypeOfValue);
    token_link(new_token);
    return;
}


void token_link(void* x){
    struct lex_old_token* newToken = x;

    if(head == NULL){
        head = newToken;
        tmp = head;
    }else{
        if(head->next == NULL){
            head->next = newToken;
            tmp = newToken;
        }else{
            tmp->next = newToken;
            tmp = newToken;
        }
    }
}

char *string_check(int line_number){
    int c;
    int length = 0;
    char* string = malloc(sizeof(char));
    bool has_closed = false;

    /* end of string */
    while((c = input()) != 0){
        if(c == '\"'){
            *(string + length) = '\0';
            has_closed = true;
            return string;
        }

        /* escape chars */
        if(c == '\\'){
            c = input();
            if(c == 'n'){
                *(string + length) = '\n';
            }else if(c == 't'){
                *(string + length) = '\t';
            }else if(c == '\\'){
                *(string + length) = '\\';
            }else if(c == '\"'){
                *(string + length) = '\"';
            }else if(c == 'r'){
                *(string + length) = '\r';
            }else if(c == '\''){
                *(string + length) = '\'';
            }else if(c == 0){
                printf("Error reached EOF");
                free(string);
                exit(1);
            }else{
                *(string + length) = c;
                printf("Invalid escape sequence\n");
            }
        }else{
            *(string + length) = c;
        }
        length++;
        string = realloc(string,sizeof(char) * (length + 1));
    }
    if(has_closed == false){
        printf("Error reached EOF, string doesn't close\n");
        exit(1);
    }
    return NULL;
} 



void consume_block_comment(int line){
    const int line_start = line;
    int comment_depth = 1;
    bool prev_was_slash = false;
    bool prev_was_asterisk = false;
    int c;
    while ((c = input()) != 0) {
        switch (c) {
        case '*':
            if (prev_was_slash){
                comment_depth++;
            }
            prev_was_slash = false;
            prev_was_asterisk = true;
            break;
        case '/':
            if(prev_was_asterisk){
                comment_depth--;
                if(comment_depth != 0){
                    // printf("%d: #%d COMMENT NESTED_COMMENT <-enumerated\n",yylineno,tokenCount++);
                }
            }
            prev_was_slash = true;
            prev_was_asterisk = false;
            break;
        default:
            prev_was_asterisk = false;
            prev_was_slash = false;
            break;
        }
        if (comment_depth == 0) {
            // printf("%d: #%d \"%d - %d\" COMMENT BLOCK_COMMENT <-enumerated\n",yylineno,tokenCount++,line_start,yylineno);
            return;
        }
    }
    exit(1);
}
