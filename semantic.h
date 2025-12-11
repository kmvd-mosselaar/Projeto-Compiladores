/* semantic.h - Analisador Semântico */

#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "ast.h"
#include "symtab.h"

/* Estrutura do Analisador Semântico */
typedef struct {
    SymbolTable *symtab;
    int has_errors;
    char current_function[256];
} SemanticAnalyzer;

/* Inicialização e finalização */
SemanticAnalyzer* semantic_create(void);
void semantic_destroy(SemanticAnalyzer *analyzer);

/* Função principal de análise */
int semantic_analyze(SemanticAnalyzer *analyzer, ASTNode *root);

/* Impressão da tabela de símbolos */
void semantic_print_table(SemanticAnalyzer *analyzer);

#endif /* SEMANTIC_H */