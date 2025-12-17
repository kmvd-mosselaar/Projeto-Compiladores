%{
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "parser.tab.h"

extern int line_num;

void lex_error(char *lexeme);
void comment_error(int start_line);
%}

%option noyywrap
%option yylineno

/* Definições */
DIGIT       [0-9]
LETTER      [a-zA-Z]
ID          {LETTER}({LETTER}|{DIGIT}|_)*
NUM         {DIGIT}+
WHITESPACE  [ \t\r]+

%%

    /* ============================================
       COMENTÁRIOS COM TRATAMENTO DE ERRO
       ============================================ */
"/*"        {
                int c;
                int start_line = line_num;
                int prev_char = 0;
                
                while ((c = input()) != 0) {
                    if (c == '\n') {
                        line_num++;
                    }
                    else if (prev_char == '*' && c == '/') {
                        break; /* Comentário fechado corretamente */
                    }
                    prev_char = c;
                }
                
                if (c == 0) {
                    /* EOF alcançado sem fechar comentário */
                    comment_error(start_line);
                }
            }

    /* ============================================
       PALAVRAS-CHAVE
       ============================================ */
"if"        { return IF; }
"else"      { return ELSE; }
"while"     { return WHILE; }
"return"    { return RETURN; }
"int"       { return INT; }
"void"      { return VOID; }

    /* ============================================
       OPERADORES RELACIONAIS
       ============================================ */
"<="        { return LE; }
">="        { return GE; }
"=="        { return EQ; }
"!="        { return NE; }
"<"         { return LT; }
">"         { return GT; }

    /* ============================================
       OPERADORES ARITMÉTICOS
       ============================================ */
"+"         { return PLUS; }
"-"         { return MINUS; }
"*"         { return TIMES; }
"/"         { return OVER; }

    /* ============================================
       ATRIBUIÇÃO
       ============================================ */
"="         { return ASSIGN; }

    /* ============================================
       DELIMITADORES
       ============================================ */
";"         { return SEMI; }
","         { return COMMA; }
"("         { return LPAREN; }
")"         { return RPAREN; }
"["         { return LBRACKET; }
"]"         { return RBRACKET; }
"{"         { return LBRACE; }
"}"         { return RBRACE; }

    /* ============================================
       IDENTIFICADORES
       ============================================ */
{ID}        { 
                yylval.strval = strdup(yytext); 
                return ID; 
            }

    /* ============================================
       NÚMEROS
       ============================================ */
{NUM}       { 
                yylval.intval = atoi(yytext); 
                return NUM; 
            }

    /* ============================================
       ESPAÇOS EM BRANCO
       ============================================ */
{WHITESPACE}    { /* Ignora espaços, tabs e \r */ }

    /* ============================================
       NOVA LINHA
       ============================================ */
\n          { line_num++; }

    /* ============================================
       CARACTERES INVÁLIDOS
       ============================================ */
.           { lex_error(yytext); }

%%

/* ========================================================
   FUNÇÃO DE ERRO LÉXICO COM MENSAGENS DETALHADAS
   ======================================================== */
void lex_error(char *lexeme) {
    char c = lexeme[0];
    
    fprintf(stderr, "\n");
    fprintf(stderr, "╔═══════════════════════════════════════════════════════════════\n");
    fprintf(stderr, "║ ERRO LÉXICO\n");
    fprintf(stderr, "╠═══════════════════════════════════════════════════════════════\n");
    fprintf(stderr, "║ Linha: %d\n", line_num);
    fprintf(stderr, "║ Caractere inválido: '%c' (código ASCII: %d)\n", c, (int)c);
    fprintf(stderr, "╠═══════════════════════════════════════════════════════════════\n");
    fprintf(stderr, "║ Sugestões:\n");
    
    /* Sugestões contextuais baseadas no caractere */
    if (c == '@' || c == '$' || c == '#' || c == '%' || c == '&') {
        fprintf(stderr, "║    - O caractere '%c' não é permitido em C-\n", c);
        fprintf(stderr, "║    - Caracteres válidos: a-z, A-Z, 0-9, _, +, -, *, /, =\n");
        fprintf(stderr, "║    - Delimitadores: ( ) [ ] { } ; ,\n");
    }
    else if (c == '!') {
        fprintf(stderr, "║    - O operador '!' sozinho não existe em C-\n");
        fprintf(stderr, "║    - Use '!=' para comparação de diferente\n");
    }
    else if (c == '|' || c == '&') {
        fprintf(stderr, "║    - Operadores lógicos '||' e '&&' não existem em C-\n");
        fprintf(stderr, "║    - C- não suporta operações lógicas bit a bit\n");
    }
    else if (c == '"' || c == '\'') {
        fprintf(stderr, "║    - Strings e caracteres não são suportados em C-\n");
        fprintf(stderr, "║    - Use apenas números inteiros\n");
    }
    else if (c == '.') {
        fprintf(stderr, "║    - Números decimais não são suportados em C-\n");
        fprintf(stderr, "║    - Use apenas números inteiros\n");
    }
    else if (c == '\\') {
        fprintf(stderr, "║    - O caractere '\\' não é permitido\n");
        fprintf(stderr, "║    - Verifique se não está tentando usar strings\n");
    }
    else if (c == '~' || c == '`') {
        fprintf(stderr, "║    - Caractere '%c' não é reconhecido\n", c);
        fprintf(stderr, "║    - Verifique o código fonte\n");
    }
    else {
        fprintf(stderr, "║    - Verifique se não há caracteres especiais\n");
        fprintf(stderr, "║    - Alfabeto C-: letras, dígitos, operadores básicos\n");
    }
    
    fprintf(stderr, "╚═══════════════════════════════════════════════════════════════\n\n");
    
    exit(1);
}

/* ========================================================
   ERRO DE COMENTÁRIO NÃO FECHADO
   ======================================================== */
void comment_error(int start_line) {
    fprintf(stderr, "\n");
    fprintf(stderr, "╔═══════════════════════════════════════════════════════════════\n");
    fprintf(stderr, "║ ERRO LÉXICO: Comentário não fechado\n");
    fprintf(stderr, "╠═══════════════════════════════════════════════════════════════\n");
    fprintf(stderr, "║ Linha inicial do comentário: %d\n", start_line);
    fprintf(stderr, "║ Linha atual: %d\n", line_num);
    fprintf(stderr, "║ Chegou ao fim do arquivo sem encontrar */\n");
    fprintf(stderr, "╚═══════════════════════════════════════════════════════════════\n");
    
    exit(1);
}

/* ========================================================
   FUNÇÃO AUXILIAR PARA OBTER NOME DO TOKEN
   ======================================================== */
char* get_token_name(int token) {
    switch(token) {
        case IF: return "IF";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case RETURN: return "RETURN";
        case INT: return "INT";
        case VOID: return "VOID";
        case LE: return "<=";
        case GE: return ">=";
        case EQ: return "==";
        case NE: return "!=";
        case LT: return "<";
        case GT: return ">";
        case PLUS: return "+";
        case MINUS: return "-";
        case TIMES: return "*";
        case OVER: return "/";
        case ASSIGN: return "=";
        case SEMI: return ";";
        case COMMA: return ",";
        case LPAREN: return "(";
        case RPAREN: return ")";
        case LBRACKET: return "[";
        case RBRACKET: return "]";
        case LBRACE: return "{";
        case RBRACE: return "}";
        case ID: return "IDENTIFICADOR";
        case NUM: return "NUMERO";
        default: return "DESCONHECIDO";
    }
}