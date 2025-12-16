%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

extern int yylex();
extern int line_num;
extern char* yytext;
extern FILE* yyin;

int yyerror(const char *msg);

ASTNode *syntax_tree = NULL;
%}

%union {
    int intval;
    char *strval;
    ASTNode *node;
    DataType dtype;
    OpType optype;
    NodeList *list;
}

/* Tokens */
%token <intval> NUM
%token <strval> ID
%token IF ELSE WHILE RETURN INT VOID
%token LE GE EQ NE LT GT
%token PLUS MINUS TIMES OVER
%token ASSIGN SEMI COMMA
%token LPAREN RPAREN LBRACKET RBRACKET LBRACE RBRACE

/* Não-terminais */
%type <node> program declaration_list declaration var_declaration fun_declaration
%type <node> params param_list param compound_stmt local_declarations
%type <node> statement_list statement expression_stmt selection_stmt
%type <node> iteration_stmt return_stmt expression var simple_expression
%type <node> additive_expression term factor call args arg_list
%type <dtype> type_specifier
%type <optype> relop addop mulop
%type <list> statement_list_builder local_declarations_list

/* Precedência e associatividade */
%right ASSIGN
%left EQ NE
%left LT LE GT GE
%left PLUS MINUS
%left TIMES OVER
%nonassoc LPAREN RPAREN LBRACKET RBRACKET

%%

program
    : declaration_list
        { 
            syntax_tree = create_program_node($1);
        }
    ;

declaration_list
    : declaration_list declaration
        {
            if ($1 == NULL) {
                $$ = $2;
            } else {
                ASTNode *t = $1;
                while (t->sibling != NULL) t = t->sibling;
                t->sibling = $2;
                $$ = $1;
            }
        }
    | declaration
        { $$ = $1; }
    ;

declaration
    : var_declaration
        { $$ = $1; }
    | fun_declaration
        { $$ = $1; }
    ;

var_declaration
    : type_specifier ID SEMI
        { $$ = create_var_decl_node($1, $2, 0, 0, line_num); free($2); }
    | type_specifier ID LBRACKET NUM RBRACKET SEMI
        { $$ = create_var_decl_node($1, $2, 1, $4, line_num); free($2); }
    ;

type_specifier
    : INT
        { $$ = TYPE_INT; }
    | VOID
        { $$ = TYPE_VOID; }
    ;

fun_declaration
    : type_specifier ID LPAREN params RPAREN compound_stmt
        { $$ = create_fun_decl_node($1, $2, $4, $6, line_num); free($2); }
    ;

params
    : param_list
        { $$ = $1; }
    | VOID
        { $$ = NULL; }
    ;

param_list
    : param_list COMMA param
        {
            if ($1 == NULL) {
                $$ = $3;
            } else {
                ASTNode *t = $1;
                while (t->sibling != NULL) t = t->sibling;
                t->sibling = $3;
                $$ = $1;
            }
        }
    | param
        { $$ = $1; }
    ;

param
    : type_specifier ID
        { $$ = create_param_node($1, $2, 0, line_num); free($2); }
    | type_specifier ID LBRACKET RBRACKET
        { $$ = create_param_node($1, $2, 1, line_num); free($2); }
    ;

compound_stmt
    : LBRACE local_declarations statement_list RBRACE
        {
            NodeList *decl_list = (NodeList*)$2;
            NodeList *stmt_list = (NodeList*)$3;
            
            $$ = create_compound_stmt_node(
                decl_list ? decl_list->nodes : NULL,
                decl_list ? decl_list->count : 0,
                stmt_list ? stmt_list->nodes : NULL,
                stmt_list ? stmt_list->count : 0,
                line_num
            );
            
            if (decl_list) { free(decl_list); }
            if (stmt_list) { free(stmt_list); }
        }
    ;

local_declarations
    : local_declarations_list
        { $$ = (ASTNode*)$1; }
    | /* empty */
        { $$ = NULL; }
    ;

local_declarations_list
    : local_declarations_list var_declaration
        {
            NodeList *list = $1;
            add_to_list(list, $2);
            $$ = list;
        }
    | var_declaration
        {
            NodeList *list = create_list();
            add_to_list(list, $1);
            $$ = list;
        }
    ;

statement_list
    : statement_list_builder
        { $$ = (ASTNode*)$1; }
    | /* empty */
        { $$ = NULL; }
    ;

statement_list_builder
    : statement_list_builder statement
        {
            NodeList *list = $1;
            if ($2) add_to_list(list, $2);
            $$ = list;
        }
    | statement
        {
            NodeList *list = create_list();
            if ($1) add_to_list(list, $1);
            $$ = list;
        }
    ;

statement
    : expression_stmt
        { $$ = $1; }
    | compound_stmt
        { $$ = $1; }
    | selection_stmt
        { $$ = $1; }
    | iteration_stmt
        { $$ = $1; }
    | return_stmt
        { $$ = $1; }
    ;

expression_stmt
    : expression SEMI
        { $$ = create_expr_stmt_node($1, line_num); }
    | SEMI
        { $$ = create_expr_stmt_node(NULL, line_num); }
    ;

selection_stmt
    : IF LPAREN expression RPAREN statement
        { $$ = create_if_stmt_node($3, $5, NULL, line_num); }
    | IF LPAREN expression RPAREN statement ELSE statement
        { $$ = create_if_stmt_node($3, $5, $7, line_num); }
    ;

iteration_stmt
    : WHILE LPAREN expression RPAREN statement
        { $$ = create_while_stmt_node($3, $5, line_num); }
    ;

return_stmt
    : RETURN SEMI
        { $$ = create_return_stmt_node(NULL, line_num); }
    | RETURN expression SEMI
        { $$ = create_return_stmt_node($2, line_num); }
    ;

expression
    : var ASSIGN expression
        { $$ = create_assign_node($1, $3, line_num); }
    | simple_expression
        { $$ = $1; }
    ;

var
    : ID
        { $$ = create_var_node($1, NULL, line_num); free($1); }
    | ID LBRACKET expression RBRACKET
        { $$ = create_var_node($1, $3, line_num); free($1); }
    ;

simple_expression
    : additive_expression relop additive_expression
        { $$ = create_binary_op_node($2, $1, $3, line_num); }
    | additive_expression
        { $$ = $1; }
    ;

relop
    : LE    { $$ = OP_LE; }
    | LT    { $$ = OP_LT; }
    | GT    { $$ = OP_GT; }
    | GE    { $$ = OP_GE; }
    | EQ    { $$ = OP_EQ; }
    | NE    { $$ = OP_NE; }
    ;

additive_expression
    : additive_expression addop term
        { $$ = create_binary_op_node($2, $1, $3, line_num); }
    | term
        { $$ = $1; }
    ;

addop
    : PLUS  { $$ = OP_ADD; }
    | MINUS { $$ = OP_SUB; }
    ;

term
    : term mulop factor
        { $$ = create_binary_op_node($2, $1, $3, line_num); }
    | factor
        { $$ = $1; }
    ;

mulop
    : TIMES { $$ = OP_MUL; }
    | OVER  { $$ = OP_DIV; }
    ;

factor
    : LPAREN expression RPAREN
        { $$ = $2; }
    | var
        { $$ = $1; }
    | call
        { $$ = $1; }
    | NUM
        { $$ = create_const_node($1, line_num); }
    ;

call
    : ID LPAREN args RPAREN
        {
            NodeList *arg_list = (NodeList*)$3;
            $$ = create_call_node($1, 
                                  arg_list ? arg_list->nodes : NULL,
                                  arg_list ? arg_list->count : 0,
                                  line_num);
            free($1);
            if (arg_list) free(arg_list);
        }
    ;

args
    : arg_list
        { $$ = (ASTNode*)$1; }
    | /* empty */
        { $$ = NULL; }
    ;

arg_list
    : arg_list COMMA expression
        {
            NodeList *list = $1;
            add_to_list(list, $3);
            $$ = list;
        }
    | expression
        {
            NodeList *list = create_list();
            add_to_list(list, $1);
            $$ = list;
        }
    ;

%%

int yyerror(const char *msg) {
    fprintf(stderr, "ERRO SINTATICO: token inesperado '%s' - LINHA: %d\n", 
            yytext, line_num);
    return 0;
}
