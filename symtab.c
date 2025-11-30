#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"

/* Criar uma nova tabela de símbolos */
SymbolTable* create_symbol_table() {
    SymbolTable *table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (!table) {
        fprintf(stderr, "Erro: falha ao alocar memória para tabela de símbolos\n");
        exit(1);
    }
    
    /* Criar escopo global */
    Scope *global = (Scope*)malloc(sizeof(Scope));
    if (!global) {
        fprintf(stderr, "Erro: falha ao alocar memória para escopo global\n");
        exit(1);
    }
    
    global->name = strdup("global");
    global->symbols = NULL;
    global->parent = NULL;
    global->next = NULL;
    
    table->global_scope = global;
    table->current_scope = global;
    table->num_symbols = 0;
    
    /* Inicializar array de escopos */
    table->scopes_capacity = 10;
    table->num_scopes = 1;
    table->all_scopes = (Scope**)malloc(sizeof(Scope*) * table->scopes_capacity);
    table->all_scopes[0] = global;
    
    return table;
}

/* Entrar em um novo escopo */
void enter_scope(SymbolTable *table, char *scope_name) {
    Scope *new_scope = (Scope*)malloc(sizeof(Scope));
    if (!new_scope) {
        fprintf(stderr, "Erro: falha ao alocar memória para novo escopo\n");
        exit(1);
    }
    
    new_scope->name = strdup(scope_name);
    new_scope->symbols = NULL;
    new_scope->parent = table->current_scope;
    new_scope->next = NULL;
    
    table->current_scope = new_scope;
    
    /* Adicionar ao array de todos os escopos */
    if (table->num_scopes >= table->scopes_capacity) {
        table->scopes_capacity *= 2;
        table->all_scopes = (Scope**)realloc(table->all_scopes, 
                                             sizeof(Scope*) * table->scopes_capacity);
    }
    table->all_scopes[table->num_scopes++] = new_scope;
}

/* Sair do escopo atual */
void exit_scope(SymbolTable *table) {
    if (table->current_scope->parent != NULL) {
        table->current_scope = table->current_scope->parent;
    }
}

/* Inserir um símbolo na tabela */
Symbol* insert_symbol(SymbolTable *table, char *name, DataType type, 
                      SymbolKind kind, int is_array, int array_size, int line_num) {
    /* Verificar se já existe no escopo atual */
    Symbol *existing = lookup_symbol_current_scope(table, name);
    if (existing != NULL) {
        return NULL; /* Símbolo já existe no escopo atual */
    }
    
    /* Criar novo símbolo */
    Symbol *symbol = (Symbol*)malloc(sizeof(Symbol));
    if (!symbol) {
        fprintf(stderr, "Erro: falha ao alocar memória para símbolo\n");
        exit(1);
    }
    
    symbol->name = strdup(name);
    symbol->type = type;
    symbol->kind = kind;
    symbol->is_array = is_array;
    symbol->array_size = array_size;
    symbol->scope = strdup(table->current_scope->name);
    symbol->line_num = line_num;
    
    /* Inserir no início da lista de símbolos do escopo atual */
    symbol->next = table->current_scope->symbols;
    table->current_scope->symbols = symbol;
    
    table->num_symbols++;
    
    return symbol;
}

/* Procurar símbolo em um escopo específico */
Symbol* lookup_symbol_in_scope(Scope *scope, char *name) {
    Symbol *current = scope->symbols;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

/* Procurar símbolo apenas no escopo atual */
Symbol* lookup_symbol_current_scope(SymbolTable *table, char *name) {
    return lookup_symbol_in_scope(table->current_scope, name);
}

/* Procurar símbolo na tabela (escopo atual e escopos pais) */
Symbol* lookup_symbol(SymbolTable *table, char *name) {
    Scope *scope = table->current_scope;
    
    /* Procurar no escopo atual e nos escopos pais */
    while (scope != NULL) {
        Symbol *symbol = lookup_symbol_in_scope(scope, name);
        if (symbol != NULL) {
            return symbol;
        }
        scope = scope->parent;
    }
    
    return NULL;
}

/* Converter tipo de símbolo para string */
char* get_symbol_kind_string(SymbolKind kind) {
    switch(kind) {
        case SYM_VARIABLE:  return "variavel";
        case SYM_FUNCTION:  return "funcao";
        case SYM_PARAMETER: return "parametro";
        default:            return "desconhecido";
    }
}

/* Imprimir um símbolo */
void print_symbol(Symbol *symbol, int indent) {
    for (int i = 0; i < indent; i++) printf("  ");
    
    printf("%-15s | %-10s | %-10s | %-15s",
           symbol->name,
           get_type_string(symbol->type),
           get_symbol_kind_string(symbol->kind),
           symbol->scope);
    
    if (symbol->is_array) {
        if (symbol->array_size > 0) {
            printf(" | array[%d]", symbol->array_size);
        } else {
            printf(" | array[]");
        }
    }
    
    printf(" | linha %d\n", symbol->line_num);
}

/* Imprimir todos os símbolos de um escopo */
void print_scope_symbols(Scope *scope, int indent) {
    Symbol *symbol = scope->symbols;
    while (symbol != NULL) {
        print_symbol(symbol, indent);
        symbol = symbol->next;
    }
}

/* Imprimir um escopo e seus sub-escopos */
void print_scope(Scope *scope, int indent) {
    if (scope == NULL) return;
    
    for (int i = 0; i < indent; i++) printf("  ");
    printf("Escopo: %s\n", scope->name);
    
    if (scope->symbols != NULL) {
        for (int i = 0; i < indent; i++) printf("  ");
        printf("%-15s | %-10s | %-10s | %-15s | Info Adicional | Linha\n",
               "Nome", "Tipo", "Categoria", "Escopo");
        for (int i = 0; i < indent; i++) printf("  ");
        printf("--------------------------------------------------------------------------------\n");
        print_scope_symbols(scope, indent);
    }
}

/* Imprimir toda a tabela de símbolos */
void print_symbol_table(SymbolTable *table) {
    printf("\n=================================================\n");
    printf("TABELA DE SÍMBOLOS\n");
    printf("=================================================\n");
    printf("Total de símbolos: %d\n\n", table->num_symbols);
    
    /* Imprimir escopo global */
    print_scope(table->global_scope, 0);
    
    /* Para imprimir todos os escopos, precisaríamos manter uma lista de todos */
    /* Por simplicidade, estamos mostrando apenas o escopo global */
    /* Em uma implementação completa, manteríamos todos os escopos para impressão */
    
    printf("\n");
}

/* Liberar memória de um símbolo */
void free_symbol(Symbol *symbol) {
    if (symbol == NULL) return;
    free(symbol->name);
    free(symbol->scope);
    free_symbol(symbol->next);
    free(symbol);
}

/* Liberar memória de um escopo */
void free_scope(Scope *scope) {
    if (scope == NULL) return;
    free(scope->name);
    free_symbol(scope->symbols);
    free_scope(scope->next);
    free(scope);
}

/* Liberar memória da tabela de símbolos */
void free_symbol_table(SymbolTable *table) {
    if (table == NULL) return;
    
    /* Liberar todos os escopos */
    for (int i = 0; i < table->num_scopes; i++) {
        Scope *scope = table->all_scopes[i];
        free(scope->name);
        free_symbol(scope->symbols);
        free(scope);
    }
    
    free(table->all_scopes);
    free(table);
}
