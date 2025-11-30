#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "symtab.h"
#include "common.h"

/* Protótipos de funções auxiliares */
static void build_symtab_declaration(ASTNode *node, SymbolTable *table);
static void build_symtab_statement(ASTNode *node, SymbolTable *table);
static void build_symtab_expression(ASTNode *node, SymbolTable *table);

/* Construir tabela de símbolos a partir da AST */
void build_symbol_table(ASTNode *root, SymbolTable *table) {
    if (root == NULL || table == NULL) return;
    
    /* Processar o programa (lista de declarações) */
    if (root->type == NODE_PROGRAM) {
        ASTNode *decl = root->sibling;
        while (decl != NULL) {
            build_symtab_declaration(decl, table);
            decl = decl->sibling;
        }
    }
}

/* Processar uma declaração */
static void build_symtab_declaration(ASTNode *node, SymbolTable *table) {
    if (node == NULL) return;
    
    switch(node->type) {
        case NODE_VAR_DECL:
            /* Inserir variável na tabela */
            insert_symbol(table,
                         node->data.var_decl.name,
                         node->data.var_decl.data_type,
                         SYM_VARIABLE,
                         node->data.var_decl.is_array,
                         node->data.var_decl.array_size,
                         node->line_num);
            break;
            
        case NODE_FUN_DECL:
            /* Inserir função no escopo global */
            insert_symbol(table,
                         node->data.fun_decl.name,
                         node->data.fun_decl.return_type,
                         SYM_FUNCTION,
                         0, 0,
                         node->line_num);
            
            /* Entrar no escopo da função */
            enter_scope(table, node->data.fun_decl.name);
            
            /* Processar parâmetros */
            ASTNode *param = node->data.fun_decl.params;
            while (param != NULL) {
                if (param->type == NODE_PARAM) {
                    insert_symbol(table,
                                 param->data.param.name,
                                 param->data.param.data_type,
                                 SYM_PARAMETER,
                                 param->data.param.is_array,
                                 0,
                                 param->line_num);
                }
                param = param->sibling;
            }
            
            /* Processar corpo da função */
            if (node->data.fun_decl.body != NULL) {
                build_symtab_statement(node->data.fun_decl.body, table);
            }
            
            /* Sair do escopo da função */
            exit_scope(table);
            break;
            
        default:
            break;
    }
}

/* Processar um statement */
static void build_symtab_statement(ASTNode *node, SymbolTable *table) {
    if (node == NULL) return;
    
    switch(node->type) {
        case NODE_COMPOUND_STMT:
            /* Processar declarações locais */
            for (int i = 0; i < node->data.compound.num_local_decls; i++) {
                build_symtab_declaration(node->data.compound.local_decls[i], table);
            }
            
            /* Processar statements */
            for (int i = 0; i < node->data.compound.num_statements; i++) {
                build_symtab_statement(node->data.compound.statements[i], table);
            }
            break;
            
        case NODE_IF_STMT:
            build_symtab_expression(node->data.if_stmt.condition, table);
            build_symtab_statement(node->data.if_stmt.then_stmt, table);
            if (node->data.if_stmt.else_stmt != NULL) {
                build_symtab_statement(node->data.if_stmt.else_stmt, table);
            }
            break;
            
        case NODE_WHILE_STMT:
            build_symtab_expression(node->data.while_stmt.condition, table);
            build_symtab_statement(node->data.while_stmt.body, table);
            break;
            
        case NODE_RETURN_STMT:
            if (node->data.return_stmt.expr != NULL) {
                build_symtab_expression(node->data.return_stmt.expr, table);
            }
            break;
            
        case NODE_EXPR_STMT:
            if (node->data.expr_stmt.expr != NULL) {
                build_symtab_expression(node->data.expr_stmt.expr, table);
            }
            break;
            
        default:
            break;
    }
}

/* Processar uma expressão (para referências futuras) */
static void build_symtab_expression(ASTNode *node, SymbolTable *table) {
    if (node == NULL) return;
    
    /* Por enquanto, não fazemos nada com expressões na construção da tabela */
    /* Isso será útil para a análise semântica */
    
    switch(node->type) {
        case NODE_ASSIGN:
            build_symtab_expression(node->data.assign.var, table);
            build_symtab_expression(node->data.assign.expr, table);
            break;
            
        case NODE_BINARY_OP:
            build_symtab_expression(node->data.binary_op.left, table);
            build_symtab_expression(node->data.binary_op.right, table);
            break;
            
        case NODE_VAR:
            if (node->data.var.index != NULL) {
                build_symtab_expression(node->data.var.index, table);
            }
            break;
            
        case NODE_CALL:
            for (int i = 0; i < node->data.call.num_args; i++) {
                build_symtab_expression(node->data.call.args[i], table);
            }
            break;
            
        default:
            break;
    }
}

/* Imprimir tabela de símbolos completa (incluindo todos os escopos) */
void print_complete_symbol_table(SymbolTable *table, ASTNode *root) {
    printf("\n=================================================\n");
    printf("TABELA DE SÍMBOLOS COMPLETA\n");
    printf("=================================================\n");
    printf("Total de símbolos: %d\n", table->num_symbols);
    printf("Total de escopos: %d\n\n", table->num_scopes);
    
    /* Cabeçalho */
    printf("%-20s | %-8s | %-12s | %-15s | %-15s | Linha\n",
           "Nome", "Tipo", "Categoria", "Escopo", "Info");
    printf("------------------------------------------------------------------------------------\n");
    
    /* Imprimir símbolos de todos os escopos */
    for (int i = 0; i < table->num_scopes; i++) {
        Scope *scope = table->all_scopes[i];
        Symbol *symbol = scope->symbols;
        
        while (symbol != NULL) {
            printf("%-20s | %-8s | %-12s | %-15s | ",
                   symbol->name,
                   get_type_string(symbol->type),
                   get_symbol_kind_string(symbol->kind),
                   symbol->scope);
            
            if (symbol->is_array) {
                if (symbol->array_size > 0) {
                    printf("array[%-2d]       ", symbol->array_size);
                } else {
                    printf("array[]         ");
                }
            } else {
                printf("%-15s ", "-");
            }
            
            printf(" | %d\n", symbol->line_num);
            symbol = symbol->next;
        }
    }
    
    printf("\n");
}

/* Imprimir símbolos recursivamente da AST */
void print_all_symbols_recursive(ASTNode *node, SymbolTable *table) {
    /* Esta função não é mais necessária, mantida para compatibilidade */
    return;
}
