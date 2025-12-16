#ifndef SYMTAB_H
#define SYMTAB_H

#include "ast.h"

/* Tipos de símbolos */
typedef enum {
    SYM_VARIABLE,
    SYM_FUNCTION,
    SYM_PARAMETER
} SymbolKind;

/* Estrutura de um símbolo */
typedef struct Symbol {
    char *name;              /* Nome do identificador */
    DataType type;           /* Tipo (int ou void) */
    SymbolKind kind;         /* Tipo de símbolo (variável, função, parâmetro) */
    int is_array;            /* 1 se for array, 0 caso contrário */
    int array_size;          /* Tamanho do array (se aplicável) */
    char *scope;             /* Nome do escopo (função/bloco) */
    int line_num;            /* Linha de declaração */
    struct Symbol *next;     /* Próximo símbolo (lista encadeada) */
} Symbol;

/* Estrutura de um escopo */
typedef struct Scope {
    char *name;              /* Nome do escopo */
    Symbol *symbols;         /* Lista de símbolos neste escopo */
    struct Scope *parent;    /* Escopo pai */
    struct Scope *next;      /* Próximo escopo irmão */
} Scope;

/* Tabela de símbolos */
typedef struct {
    Scope *global_scope;     /* Escopo global */
    Scope *current_scope;    /* Escopo atual */
    Scope **all_scopes;      /* Array de todos os escopos criados */
    int num_scopes;          /* Número de escopos */
    int scopes_capacity;     /* Capacidade do array de escopos */
    int num_symbols;         /* Número total de símbolos */
} SymbolTable;

/* Funções para gerenciamento de escopos */
SymbolTable* create_symbol_table();
void enter_scope(SymbolTable *table, char *scope_name);
void exit_scope(SymbolTable *table);

/* Funções para manipulação de símbolos */
Symbol* insert_symbol(SymbolTable *table, char *name, DataType type, 
                      SymbolKind kind, int is_array, int array_size, int line_num);
Symbol* lookup_symbol(SymbolTable *table, char *name);
Symbol* lookup_symbol_in_scope(Scope *scope, char *name);
Symbol* lookup_symbol_current_scope(SymbolTable *table, char *name);

/* Funções auxiliares */
void print_symbol_table(SymbolTable *table);
void print_scope(Scope *scope, int indent);
void free_symbol_table(SymbolTable *table);
char* get_symbol_kind_string(SymbolKind kind);

/* Construção da tabela de símbolos a partir da AST */
void build_symbol_table(ASTNode *root, SymbolTable *table);
void print_complete_symbol_table(SymbolTable *table, ASTNode *root);
void print_all_symbols_recursive(ASTNode *node, SymbolTable *table);

#endif
