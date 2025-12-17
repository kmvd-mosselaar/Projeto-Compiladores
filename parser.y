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
void syntax_error_detail(const char *msg, const char *token, int line);

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

/* ========================================================
   FUNÇÃO DE ERRO SINTÁTICO COM MENSAGENS DETALHADAS
   ======================================================== */

void syntax_error_detail(const char *msg, const char *token, int line) {
    fprintf(stderr, "\n");
    fprintf(stderr, "╔═══════════════════════════════════════════════════════════════\n");
    fprintf(stderr, "║ ERRO SINTÁTICO\n");
    fprintf(stderr, "╠═══════════════════════════════════════════════════════════════\n");
    fprintf(stderr, "║ Linha: %d\n", line);
    fprintf(stderr, "║ Mensagem: %s\n", msg);
    fprintf(stderr, "║ Token encontrado: '%s'\n", token);
    fprintf(stderr, "╠═══════════════════════════════════════════════════════════════\n");
    fprintf(stderr, "║ Sugestões:\n");
    
    /* Análise contextual do token para sugestões específicas */
    
    /* Erros com ponto-e-vírgula */
    if (strcmp(token, ";") == 0) {
        fprintf(stderr, "║    - Verifique se não falta uma expressão antes do ';'\n");
        fprintf(stderr, "║    - Exemplo correto: x = 10;\n");
        fprintf(stderr, "║    - Exemplo incorreto: x = ;\n");
    }
    /* Erros com RETURN */
    else if (strcmp(token, "return") == 0) {
        fprintf(stderr, "║    - 'return' só pode aparecer dentro de funções\n");
        fprintf(stderr, "║    - Verifique se está dentro do corpo de uma função\n");
        fprintf(stderr, "║    - Pode estar faltando ';' antes do return\n");
    }
    /* Erros com ELSE */
    else if (strcmp(token, "else") == 0) {
        fprintf(stderr, "║    - 'else' deve vir após um comando 'if' completo\n");
        fprintf(stderr, "║    - Verifique se o 'if' tem um statement válido\n");
        fprintf(stderr, "║    - Sintaxe: if (cond) statement else statement\n");
    }
    /* Erros com fechamento de parênteses */
    else if (strcmp(token, ")") == 0) {
        fprintf(stderr, "║    - Verifique se os parênteses estão balanceados\n");
        fprintf(stderr, "║    - Pode estar faltando '(' correspondente\n");
        fprintf(stderr, "║    - Ou há ')' em excesso\n");
    }
    /* Erros com abertura de parênteses */
    else if (strcmp(token, "(") == 0) {
        fprintf(stderr, "║    - Parêntese '(' inesperado nesta posição\n");
        fprintf(stderr, "║    - Verifique a sintaxe da expressão\n");
    }
    /* Erros com fechamento de chaves */
    else if (strcmp(token, "}") == 0) {
        fprintf(stderr, "║    - Verifique se as chaves estão balanceadas\n");
        fprintf(stderr, "║    - Pode estar faltando '{' correspondente\n");
        fprintf(stderr, "║    - Ou há '}' em excesso\n");
        fprintf(stderr, "║    - Verifique se todos os comandos têm ';' no final\n");
    }
    /* Erros com abertura de chaves */
    else if (strcmp(token, "{") == 0) {
        fprintf(stderr, "║    - Chave '{' inesperada nesta posição\n");
        fprintf(stderr, "║    - Blocos só podem aparecer após: funções, if, while\n");
    }
    /* Erros com fechamento de colchetes */
    else if (strcmp(token, "]") == 0) {
        fprintf(stderr, "║    - Verifique se os colchetes estão balanceados\n");
        fprintf(stderr, "║    - Sintaxe de array: nome[expressão]\n");
    }
    /* Erros com vírgula */
    else if (strcmp(token, ",") == 0) {
        fprintf(stderr, "║    - Vírgula inesperada\n");
        fprintf(stderr, "║    - Vírgulas são usadas para separar:\n");
        fprintf(stderr, "║      • Parâmetros de função\n");
        fprintf(stderr, "║      • Argumentos em chamadas de função\n");
    }
    /* Erros com operadores */
    else if (strcmp(token, "+") == 0 || strcmp(token, "-") == 0 || 
             strcmp(token, "*") == 0 || strcmp(token, "/") == 0) {
        fprintf(stderr, "║    - Operador '%s' inesperado\n", token);
        fprintf(stderr, "║    - Verifique se há operandos antes e depois\n");
        fprintf(stderr, "║    - Exemplo correto: a %s b\n", token);
    }
    /* Erros com atribuição */
    else if (strcmp(token, "=") == 0) {
        fprintf(stderr, "║    - Verifique a sintaxe da atribuição\n");
        fprintf(stderr, "║    - Formato correto: variavel = expressao;\n");
        fprintf(stderr, "║    - Lembre-se: use '==' para comparação\n");
    }
    /* Erros com IF */
    else if (strcmp(token, "if") == 0) {
        fprintf(stderr, "║    - Sintaxe correta: if (expressao) statement\n");
        fprintf(stderr, "║    - Verifique se não falta ';' antes do if\n");
    }
    /* Erros com WHILE */
    else if (strcmp(token, "while") == 0) {
        fprintf(stderr, "║    - Sintaxe correta: while (expressao) statement\n");
        fprintf(stderr, "║    - Verifique se não falta ';' antes do while\n");
    }
    /* Erros com tipos */
    else if (strcmp(token, "int") == 0 || strcmp(token, "void") == 0) {
        fprintf(stderr, "║    - Tipo '%s' inesperado nesta posição\n", token);
        fprintf(stderr, "║    - Declarações devem estar no início de blocos\n");
        fprintf(stderr, "║    - Funções devem ter tipo de retorno\n");
    }
    /* Erros com identificadores */
    else if (yytext && (yytext[0] >= 'a' && yytext[0] <= 'z') || 
                       (yytext[0] >= 'A' && yytext[0] <= 'Z') ||
                       (yytext[0] == '_')) {
        fprintf(stderr, "║    - Identificador '%s' em posição inválida\n", token);
        fprintf(stderr, "║    - Verifique se a declaração está correta\n");
        fprintf(stderr, "║    - Pode estar faltando ';' no comando anterior\n");
    }
    /* Erros com números */
    else if (yytext && yytext[0] >= '0' && yytext[0] <= '9') {
        fprintf(stderr, "║    - Número '%s' em posição inválida\n", token);
        fprintf(stderr, "║    - Verifique a sintaxe da expressão\n");
    }
    /* Erros genéricos */
    else {
        fprintf(stderr, "║    - Verifique:\n");
        fprintf(stderr, "║      • Ponto-e-vírgula (;) ao final de comandos\n");
        fprintf(stderr, "║      • Parênteses (), colchetes [] e chaves {} balanceados\n");
        fprintf(stderr, "║      • Sintaxe de declarações e expressões\n");
        fprintf(stderr, "║      • Uso correto de palavras-chave\n");
    }
    
    fprintf(stderr, "╚═══════════════════════════════════════════════════════════════\n\n");
}

int yyerror(const char *msg) {
    syntax_error_detail(msg, yytext, line_num);
    return 0;
}