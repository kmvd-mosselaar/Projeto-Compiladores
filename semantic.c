/* semantic.c - Implementação do Analisador Semântico */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

/* Protótipos internos */
static void analyze_program(SemanticAnalyzer *a, ASTNode *node);
static void analyze_var_decl(SemanticAnalyzer *a, ASTNode *node);
static void analyze_fun_decl(SemanticAnalyzer *a, ASTNode *node);
static void analyze_compound_stmt(SemanticAnalyzer *a, ASTNode *node);
static void analyze_if_stmt(SemanticAnalyzer *a, ASTNode *node);
static void analyze_while_stmt(SemanticAnalyzer *a, ASTNode *node);
static void analyze_return_stmt(SemanticAnalyzer *a, ASTNode *node);
static void analyze_assign(SemanticAnalyzer *a, ASTNode *node);
static DataType analyze_expr(SemanticAnalyzer *a, ASTNode *node);
static DataType analyze_var(SemanticAnalyzer *a, ASTNode *node);
static DataType analyze_call(SemanticAnalyzer *a, ASTNode *node);
static DataType analyze_op(SemanticAnalyzer *a, ASTNode *node);
static void semantic_error(const char *msg, int lineno);

/* Criação do analisador semântico */
SemanticAnalyzer* semantic_create(void) {
    SemanticAnalyzer *analyzer = (SemanticAnalyzer*)malloc(sizeof(SemanticAnalyzer));
    if (!analyzer) {
        fprintf(stderr, "Erro ao alocar memória para analisador semântico\n");
        exit(1);
    }
    
    analyzer->symtab = symtab_create();
    analyzer->has_errors = 0;
    strcpy(analyzer->current_function, "");
    
    return analyzer;
}

/* Destruição do analisador */
void semantic_destroy(SemanticAnalyzer *analyzer) {
    if (analyzer) {
        if (analyzer->symtab) {
            symtab_destroy(analyzer->symtab);
        }
        free(analyzer);
    }
}

/* Função principal de análise */
int semantic_analyze(SemanticAnalyzer *analyzer, ASTNode *root) {
    if (!root || !analyzer) {
        return 0;
    }
    
    analyze_program(analyzer, root);
    
    /* Verifica se main() existe */
    Symbol *main_sym = symtab_lookup(analyzer->symtab, "main");
    if (!main_sym || main_sym->kind != SYMBOL_FUNCTION) {
        fprintf(stderr, "ERRO SEMANTICO: funcao 'main' nao declarada\n");
        analyzer->has_errors = 1;
    }
    
    return !analyzer->has_errors;
}

/* Impressão da tabela de símbolos */
void semantic_print_table(SemanticAnalyzer *analyzer) {
    if (analyzer && analyzer->symtab) {
        symtab_print(analyzer->symtab);
    }
}

/* Reporta erro semântico */
static void semantic_error(const char *msg, int lineno) {
    fprintf(stderr, "ERRO SEMANTICO: %s - LINHA: %d\n", msg, lineno);
}

/* Análise do programa */
static void analyze_program(SemanticAnalyzer *a, ASTNode *node) {
    if (node->kind != NODE_PROGRAM) {
        return;
    }
    
    ASTNode *child = node->child;
    while (child) {
        if (child->kind == NODE_VAR_DECL) {
            analyze_var_decl(a, child);
        } else if (child->kind == NODE_FUN_DECL) {
            analyze_fun_decl(a, child);
        }
        child = child->sibling;
    }
}

/* Análise de declaração de variável */
static void analyze_var_decl(SemanticAnalyzer *a, ASTNode *node) {
    if (node->type == TYPE_VOID) {
        char msg[256];
        snprintf(msg, 256, "variavel '%s' nao pode ser void", node->name);
        semantic_error(msg, node->lineno);
        a->has_errors = 1;
        return;
    }
    
    SymbolKind kind = node->is_array ? SYMBOL_ARRAY : SYMBOL_VARIABLE;
    
    if (!symtab_insert(a->symtab, node->name, node->type, kind, 
                       node->array_size, node->lineno)) {
        char msg[256];
        snprintf(msg, 256, "identificador '%s' ja declarado", node->name);
        semantic_error(msg, node->lineno);
        a->has_errors = 1;
    }
}

/* Análise de declaração de função */
static void analyze_fun_decl(SemanticAnalyzer *a, ASTNode *node) {
    strcpy(a->current_function, node->name);
    
    /* Insere função na tabela global */
    if (!symtab_insert(a->symtab, node->name, node->type, 
                       SYMBOL_FUNCTION, 0, node->lineno)) {
        char msg[256];
        snprintf(msg, 256, "funcao '%s' ja declarada", node->name);
        semantic_error(msg, node->lineno);
        a->has_errors = 1;
        return;
    }
    
    /* Entra no escopo da função */
    symtab_enter_scope(a->symtab, node->name);
    
    /* Insere parâmetros */
    ASTNode *param = node->child;
    while (param && param->kind == NODE_PARAM) {
        SymbolKind kind = param->is_array ? SYMBOL_ARRAY : SYMBOL_PARAM;
        if (!symtab_insert(a->symtab, param->name, param->type, 
                          kind, 0, param->lineno)) {
            char msg[256];
            snprintf(msg, 256, "parametro '%s' duplicado", param->name);
            semantic_error(msg, param->lineno);
            a->has_errors = 1;
        }
        param = param->sibling;
    }
    
    /* Analisa corpo da função */
    ASTNode *body = node->child;
    while (body && body->kind == NODE_PARAM) {
        body = body->sibling;
    }
    
    if (body && body->kind == NODE_COMPOUND_STMT) {
        analyze_compound_stmt(a, body);
    }
    
    /* Sai do escopo */
    symtab_exit_scope(a->symtab);
}

/* Análise de bloco composto */
static void analyze_compound_stmt(SemanticAnalyzer *a, ASTNode *node) {
    /* Processa declarações locais */
    ASTNode *child = node->child;
    while (child) {
        if (child->kind == NODE_VAR_DECL) {
            analyze_var_decl(a, child);
        } else {
            break; /* Acabaram as declarações */
        }
        child = child->sibling;
    }
    
    /* Processa statements */
    while (child) {
        switch (child->kind) {
            case NODE_IF_STMT:
                analyze_if_stmt(a, child);
                break;
            case NODE_WHILE_STMT:
                analyze_while_stmt(a, child);
                break;
            case NODE_RETURN_STMT:
                analyze_return_stmt(a, child);
                break;
            case NODE_ASSIGN:
                analyze_assign(a, child);
                break;
            case NODE_CALL:
                analyze_call(a, child);
                break;
            case NODE_COMPOUND_STMT:
                analyze_compound_stmt(a, child);
                break;
            default:
                if (child->kind == NODE_BINARY_OP || 
                    child->kind == NODE_VAR || 
                    child->kind == NODE_CONST) {
                    analyze_expr(a, child);
                }
                break;
        }
        child = child->sibling;
    }
}

/* Análise de if */
static void analyze_if_stmt(SemanticAnalyzer *a, ASTNode *node) {
    /* Analisa condição */
    if (node->child) {
        analyze_expr(a, node->child);
    }
    
    /* Analisa then */
    ASTNode *then_stmt = node->child ? node->child->sibling : NULL;
    if (then_stmt) {
        if (then_stmt->kind == NODE_COMPOUND_STMT) {
            analyze_compound_stmt(a, then_stmt);
        }
    }
    
    /* Analisa else (se existir) */
    ASTNode *else_stmt = then_stmt ? then_stmt->sibling : NULL;
    if (else_stmt) {
        if (else_stmt->kind == NODE_COMPOUND_STMT) {
            analyze_compound_stmt(a, else_stmt);
        }
    }
}

/* Análise de while */
static void analyze_while_stmt(SemanticAnalyzer *a, ASTNode *node) {
    /* Analisa condição */
    if (node->child) {
        analyze_expr(a, node->child);
    }
    
    /* Analisa corpo */
    ASTNode *body = node->child ? node->child->sibling : NULL;
    if (body && body->kind == NODE_COMPOUND_STMT) {
        analyze_compound_stmt(a, body);
    }
}

/* Análise de return */
static void analyze_return_stmt(SemanticAnalyzer *a, ASTNode *node) {
    if (node->child) {
        analyze_expr(a, node->child);
    }
}

/* Análise de atribuição */
static void analyze_assign(SemanticAnalyzer *a, ASTNode *node) {
    DataType var_type = analyze_var(a, node->child);
    
    ASTNode *expr = node->child ? node->child->sibling : NULL;
    if (expr) {
        DataType expr_type = analyze_expr(a, expr);
        
        if (var_type != TYPE_INT && var_type != TYPE_VOID) {
            semantic_error("atribuicao invalida", node->lineno);
            a->has_errors = 1;
        }
    }
}

/* Análise de expressão */
static DataType analyze_expr(SemanticAnalyzer *a, ASTNode *node) {
    if (!node) {
        return TYPE_VOID;
    }
    
    switch (node->kind) {
        case NODE_VAR:
            return analyze_var(a, node);
        case NODE_CALL:
            return analyze_call(a, node);
        case NODE_BINARY_OP:
            return analyze_op(a, node);
        case NODE_CONST:
            return TYPE_INT;
        default:
            return TYPE_VOID;
    }
}

/* Análise de variável */
static DataType analyze_var(SemanticAnalyzer *a, ASTNode *node) {
    Symbol *sym = symtab_lookup(a->symtab, node->name);
    if (!sym) {
        char msg[256];
        snprintf(msg, 256, "identificador '%s' nao declarado", node->name);
        semantic_error(msg, node->lineno);
        a->has_errors = 1;
        return TYPE_VOID;
    }
    
    /* Se tem índice, é acesso a array */
    if (node->child) {
        analyze_expr(a, node->child);
        if (sym->kind != SYMBOL_ARRAY) {
            char msg[256];
            snprintf(msg, 256, "'%s' nao e um array", node->name);
            semantic_error(msg, node->lineno);
            a->has_errors = 1;
        }
        return TYPE_INT;
    }
    
    return sym->type;
}

/* Análise de chamada de função */
static DataType analyze_call(SemanticAnalyzer *a, ASTNode *node) {
    Symbol *sym = symtab_lookup(a->symtab, node->name);
    if (!sym) {
        char msg[256];
        snprintf(msg, 256, "funcao '%s' nao declarada", node->name);
        semantic_error(msg, node->lineno);
        a->has_errors = 1;
        return TYPE_VOID;
    }
    
    if (sym->kind != SYMBOL_FUNCTION) {
        char msg[256];
        snprintf(msg, 256, "'%s' nao e uma funcao", node->name);
        semantic_error(msg, node->lineno);
        a->has_errors = 1;
        return TYPE_VOID;
    }
    
    /* Analisa argumentos */
    ASTNode *arg = node->child;
    while (arg) {
        analyze_expr(a, arg);
        arg = arg->sibling;
    }
    
    /* Funções built-in */
    if (strcmp(node->name, "input") == 0) {
        return TYPE_INT;
    }
    
    return TYPE_VOID;
}

/* Análise de operação binária */
static DataType analyze_op(SemanticAnalyzer *a, ASTNode *node) {
    if (node->child) {
        analyze_expr(a, node->child);
    }
    if (node->child && node->child->sibling) {
        analyze_expr(a, node->child->sibling);
    }
    return TYPE_INT;
}