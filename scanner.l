%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "parser.tab.h"

int line_num = 1;
extern int yyerror(const char *msg);

void lex_error(char *lexeme) {
    fprintf(stderr, "ERRO LEXICO: '%s' - LINHA: %d\n", lexeme, line_num);
    exit(1);
}
%}

%option noyywrap
%option yylineno

/* Definições */
DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          {LETTER}({LETTER}|{DIGIT})*
NUM         {DIGIT}+
WHITESPACE  [ \t\r]+

%%

    /* Comentários */
"/*"([^*]|\*+[^*/])*\*+"/"  { /* Ignora comentários */ }

    /* Palavras-chave */
"if"        { return IF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"return"    { return RETURN; }
"int"       { return INT; }
"void"      { return VOID; }

    /* Operadores relacionais */
"<="        { return LE; }
">="        { return GE; }
"=="        { return EQ; }
"!="        { return NE; }
"<"         { return LT; }
">"         { return GT; }

    /* Operadores aritméticos */
"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return TIMES; }
"/"         { return OVER; }

    /* Atribuição */
"="         { return ASSIGN; }

    /* Delimitadores */
";"         { return SEMI; }
","         { return COMMA; }
"("         { return LPAREN; }
")"         { return RPAREN; }
"["         { return LBRACKET; }
"]"         { return RBRACKET; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }

    /* Identificadores */
{ID}        { 
                yylval.strval = strdup(yytext); 
                return ID; 
            }

    /* Números */
{NUM}       { 
                yylval.intval = atoi(yytext); 
                return NUM; 
            }

    /* Espaços em branco */
{WHITESPACE}    { /* Ignora espaços */ }

    /* Nova linha */
\n          { line_num++; }

    /* Caracteres inválidos */
.           { lex_error(yytext); }

%%

/* Função auxiliar para obter o nome do token */
char* get_token_name(int token) {
    switch(token) {
        case IF: return "IF";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case RETURN: return "RETURN";
        case INT: return "INT";
        case VOID: return "VOID";
        case LE: return "LE";
        case GE: return "GE";
        case EQ: return "EQ";
        case NE: return "NE";
        case LT: return "LT";
        case GT: return "GT";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case TIMES: return "TIMES";
        case OVER: return "OVER";
        case ASSIGN: return "ASSIGN";
        case SEMI: return "SEMI";
        case COMMA: return "COMMA";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
        case ID: return "ID";
        case NUM: return "NUM";
        default: return "UNKNOWN";
    }
    {
        fprintf(stderr, "\n╔══════════════════════════════════════════════════════\n");
        fprintf(stderr, "║ ERRO LÉXICO\n");
        fprintf(stderr, "╠══════════════════════════════════════════════════════\n");
        fprintf(stderr, "║ Linha: %d\n", line_num);
        fprintf(stderr, "║ Caractere inválido: '%c' (código ASCII: %d)\n", 
                yytext[0], (int)yytext[0]);
        fprintf(stderr, "║ Sugestão: Verifique se não há caracteres especiais\n");
        fprintf(stderr, "╚══════════════════════════════════════════════════════\n\n");
        return ERROR;
    }
}
