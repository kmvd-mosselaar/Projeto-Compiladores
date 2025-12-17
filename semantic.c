/* semantic.c - Implementação do Analisador Semântico CORRIGIDO */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "semantic.h"

/* Protótipos internos */
static void analyze_node(SemanticAnalyzer *a, ASTNode *node);
static void analyze_var_decl(SemanticAnalyzer *a, ASTNode *node);
static void analyze_fun_decl(SemanticAnalyzer *a, ASTNode *node);
static void analyze_param(SemanticAnalyzer *a, ASTNode *node);
static void analyze_compound_stmt(SemanticAnalyzer *a, ASTNode *node);
static void analyze_statement(SemanticAnalyzer *a, ASTNode *node);
static DataType analyze_expression(SemanticAnalyzer *a, ASTNode *node);
static int check_return_exists(ASTNode *node);
static void semantic_error(const char *msg, int lineno);

/* Criação do analisador semântico */
SemanticAnalyzer *semantic_create(void)
{
    SemanticAnalyzer *analyzer = (SemanticAnalyzer *)malloc(sizeof(SemanticAnalyzer));
    if (!analyzer)
    {
        fprintf(stderr, "Erro ao alocar memória para analisador semântico\n");
        exit(1);
    }

    analyzer->symtab = create_symbol_table();
    analyzer->has_errors = 0;
    strcpy(analyzer->current_function, "");
    analyzer->current_function_type = TYPE_VOID;

    return analyzer;
}

/* Destruição do analisador */
void semantic_destroy(SemanticAnalyzer *analyzer)
{
    if (analyzer)
    {
        if (analyzer->symtab)
        {
            free_symbol_table(analyzer->symtab);
        }
        free(analyzer);
    }
}

/* Função principal de análise */
int semantic_analyze(SemanticAnalyzer *analyzer, ASTNode *root)
{
    if (!root || !analyzer)
    {
        return 0;
    }

    /* Processa todas as declarações do programa */
    if (root->type == NODE_PROGRAM)
    {
        ASTNode *decl = root->sibling;
        while (decl != NULL)
        {
            analyze_node(analyzer, decl);
            decl = decl->sibling;
        }
    }

    /* Verifica se main() existe */
    Symbol *main_sym = lookup_symbol(analyzer->symtab, "main");
    if (!main_sym || main_sym->kind != SYM_FUNCTION)
    {
        fprintf(stderr, "ERRO SEMANTICO: funcao 'main' nao declarada\n");
        analyzer->has_errors = 1;
    }

    return !analyzer->has_errors;
}

/* Impressão da tabela de símbolos */
void semantic_print_table(SemanticAnalyzer *analyzer)
{
    if (analyzer && analyzer->symtab)
    {
        print_complete_symbol_table(analyzer->symtab, NULL);
    }
}

/* Reporta erro semântico */
static void semantic_error(const char *msg, int lineno)
{
    fprintf(stderr, "ERRO SEMANTICO: %s - LINHA: %d\n", msg, lineno);
}

/* Análise de nó genérico */
static void analyze_node(SemanticAnalyzer *a, ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_VAR_DECL:
        analyze_var_decl(a, node);
        break;
    case NODE_FUN_DECL:
        analyze_fun_decl(a, node);
        break;
    case NODE_COMPOUND_STMT:
        analyze_compound_stmt(a, node);
        break;
    case NODE_IF_STMT:
    case NODE_WHILE_STMT:
    case NODE_RETURN_STMT:
    case NODE_EXPR_STMT:
        analyze_statement(a, node);
        break;
    case NODE_ASSIGN:
    case NODE_BINARY_OP:
    case NODE_VAR:
    case NODE_CALL:
    case NODE_CONST:
        analyze_expression(a, node);
        break;
    default:
        break;
    }
}

/* Análise de declaração de variável */
static void analyze_var_decl(SemanticAnalyzer *a, ASTNode *node)
{
    if (node->data.var_decl.data_type == TYPE_VOID)
    {
        char msg[256];
        snprintf(msg, 256, "variavel '%s' nao pode ser void", node->data.var_decl.name);
        semantic_error(msg, node->line_num);
        a->has_errors = 1;
        return;
    }

    Symbol *existing = lookup_symbol_current_scope(a->symtab, node->data.var_decl.name);
    if (existing)
    {
        char msg[256];
        snprintf(msg, 256, "identificador '%s' ja declarado na linha %d",
                 node->data.var_decl.name, existing->line_num);
        semantic_error(msg, node->line_num);
        a->has_errors = 1;
        return;
    }

    insert_symbol(a->symtab,
                  node->data.var_decl.name,
                  node->data.var_decl.data_type,
                  SYM_VARIABLE,
                  node->data.var_decl.is_array,
                  node->data.var_decl.array_size,
                  node->line_num);
}

/* CORREÇÃO 2: Função auxiliar para verificar se existe return no corpo */
static int check_return_exists(ASTNode *node)
{
    if (!node)
        return 0;

    /* Se é um return, encontrou */
    if (node->type == NODE_RETURN_STMT)
    {
        return 1;
    }

    /* Se é compound statement, verifica todos os statements */
    if (node->type == NODE_COMPOUND_STMT)
    {
        for (int i = 0; i < node->data.compound.num_statements; i++)
        {
            if (check_return_exists(node->data.compound.statements[i]))
            {
                return 1;
            }
        }
    }

    /* Se é if, verifica then e else */
    if (node->type == NODE_IF_STMT)
    {
        if (check_return_exists(node->data.if_stmt.then_stmt))
        {
            return 1;
        }
        if (node->data.if_stmt.else_stmt &&
            check_return_exists(node->data.if_stmt.else_stmt))
        {
            return 1;
        }
    }

    /* Se é while, verifica corpo */
    if (node->type == NODE_WHILE_STMT)
    {
        if (check_return_exists(node->data.while_stmt.body))
        {
            return 1;
        }
    }

    /* Se é expression statement, não tem return */
    if (node->type == NODE_EXPR_STMT)
    {
        return 0;
    }

    return 0;
}

/* Análise de declaração de função */
static void analyze_fun_decl(SemanticAnalyzer *a, ASTNode *node)
{
    strcpy(a->current_function, node->data.fun_decl.name);
    a->current_function_type = node->data.fun_decl.return_type;

    /* Verifica se função já existe */
    Symbol *existing = lookup_symbol(a->symtab, node->data.fun_decl.name);
    if (existing)
    {
        char msg[256];
        snprintf(msg, 256, "funcao '%s' ja declarada na linha %d",
                 node->data.fun_decl.name, existing->line_num);
        semantic_error(msg, node->line_num);
        a->has_errors = 1;
        return;
    }

    /* Insere função na tabela global */
    insert_symbol(a->symtab,
                  node->data.fun_decl.name,
                  node->data.fun_decl.return_type,
                  SYM_FUNCTION,
                  0, 0,
                  node->line_num);

    /* Entra no escopo da função */
    enter_scope(a->symtab, node->data.fun_decl.name);

    /* Analisa parâmetros */
    ASTNode *param = node->data.fun_decl.params;
    while (param)
    {
        analyze_param(a, param);
        param = param->sibling;
    }

    /* Analisa corpo da função */
    if (node->data.fun_decl.body)
    {
        analyze_compound_stmt(a, node->data.fun_decl.body);

        /* CORREÇÃO 2: Verifica se função não-void tem return */
        if (node->data.fun_decl.return_type != TYPE_VOID)
        {
            if (!check_return_exists(node->data.fun_decl.body))
            {
                char msg[256];
                snprintf(msg, 256, "funcao '%s' deve retornar um valor",
                         node->data.fun_decl.name);
                semantic_error(msg, node->line_num);
                a->has_errors = 1;
            }
        }
    }

    /* Sai do escopo */
    exit_scope(a->symtab);
}

/* Análise de parâmetro */
static void analyze_param(SemanticAnalyzer *a, ASTNode *node)
{
    if (node->type != NODE_PARAM)
        return;

    if (node->data.param.data_type == TYPE_VOID)
    {
        char msg[256];
        snprintf(msg, 256, "parametro '%s' nao pode ser void", node->data.param.name);
        semantic_error(msg, node->line_num);
        a->has_errors = 1;
        return;
    }

    Symbol *existing = lookup_symbol_current_scope(a->symtab, node->data.param.name);
    if (existing)
    {
        char msg[256];
        snprintf(msg, 256, "parametro '%s' duplicado", node->data.param.name);
        semantic_error(msg, node->line_num);
        a->has_errors = 1;
        return;
    }

    insert_symbol(a->symtab,
                  node->data.param.name,
                  node->data.param.data_type,
                  SYM_PARAMETER,
                  node->data.param.is_array,
                  0,
                  node->line_num);
}

/* Análise de bloco composto */
static void analyze_compound_stmt(SemanticAnalyzer *a, ASTNode *node)
{
    /* Processa declarações locais */
    for (int i = 0; i < node->data.compound.num_local_decls; i++)
    {
        analyze_var_decl(a, node->data.compound.local_decls[i]);
    }

    /* Processa statements */
    for (int i = 0; i < node->data.compound.num_statements; i++)
    {
        analyze_statement(a, node->data.compound.statements[i]);
    }
}

/* Análise de statement */
static void analyze_statement(SemanticAnalyzer *a, ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_COMPOUND_STMT:
        analyze_compound_stmt(a, node);
        break;

    case NODE_IF_STMT:
        /* Analisa condição */
        analyze_expression(a, node->data.if_stmt.condition);
        /* Analisa then */
        analyze_statement(a, node->data.if_stmt.then_stmt);
        /* Analisa else (se existir) */
        if (node->data.if_stmt.else_stmt)
        {
            analyze_statement(a, node->data.if_stmt.else_stmt);
        }
        break;

    case NODE_WHILE_STMT:
        /* Analisa condição */
        analyze_expression(a, node->data.while_stmt.condition);
        /* Analisa corpo */
        analyze_statement(a, node->data.while_stmt.body);
        break;

    case NODE_RETURN_STMT:
        if (node->data.return_stmt.expr)
        {
            DataType expr_type = analyze_expression(a, node->data.return_stmt.expr);
            if (a->current_function_type == TYPE_VOID && expr_type != TYPE_VOID)
            {
                char msg[256];
                snprintf(msg, 256, "funcao void nao pode retornar valor");
                semantic_error(msg, node->line_num);
                a->has_errors = 1;
            }
        }
        else
        {
            if (a->current_function_type != TYPE_VOID)
            {
                char msg[256];
                snprintf(msg, 256, "funcao deve retornar um valor");
                semantic_error(msg, node->line_num);
                a->has_errors = 1;
            }
        }
        break;

    case NODE_EXPR_STMT:
        if (node->data.expr_stmt.expr)
        {
            analyze_expression(a, node->data.expr_stmt.expr);
        }
        break;

    default:
        break;
    }
}

/* Análise de expressão */
static DataType analyze_expression(SemanticAnalyzer *a, ASTNode *node)
{
    if (!node)
        return TYPE_VOID;

    switch (node->type)
    {
    case NODE_ASSIGN:
    {
        /* Verifica se está tentando atribuir a um array inteiro */
        if (node->data.assign.var->type == NODE_VAR)
        {
            Symbol *sym = lookup_symbol(a->symtab, node->data.assign.var->data.var.name);
            if (sym && sym->is_array && !node->data.assign.var->data.var.index &&
                sym->kind == SYM_VARIABLE)
            {
                char msg[256];
                snprintf(msg, 256, "atribuicao invalida: '%s' e um array",
                         node->data.assign.var->data.var.name);
                semantic_error(msg, node->line_num);
                a->has_errors = 1;
                return TYPE_VOID;
            }
        }

        DataType var_type = analyze_expression(a, node->data.assign.var);
        DataType expr_type = analyze_expression(a, node->data.assign.expr);

        if (var_type == TYPE_VOID || expr_type == TYPE_VOID)
        {
            semantic_error("atribuicao invalida com tipo void", node->line_num);
            a->has_errors = 1;
        }
        return var_type;
    }

    case NODE_BINARY_OP:
    {
        /* Verifica se os operandos são variáveis e se são arrays usados incorretamente */
        if (node->data.binary_op.left->type == NODE_VAR)
        {
            Symbol *sym = lookup_symbol(a->symtab, node->data.binary_op.left->data.var.name);
            if (sym && sym->is_array && !node->data.binary_op.left->data.var.index &&
                sym->kind == SYM_VARIABLE)
            {
                char msg[256];
                snprintf(msg, 256, "operacao invalida: '%s' e um array",
                         node->data.binary_op.left->data.var.name);
                semantic_error(msg, node->line_num);
                a->has_errors = 1;
            }
        }
        if (node->data.binary_op.right->type == NODE_VAR)
        {
            Symbol *sym = lookup_symbol(a->symtab, node->data.binary_op.right->data.var.name);
            if (sym && sym->is_array && !node->data.binary_op.right->data.var.index &&
                sym->kind == SYM_VARIABLE)
            {
                char msg[256];
                snprintf(msg, 256, "operacao invalida: '%s' e um array",
                         node->data.binary_op.right->data.var.name);
                semantic_error(msg, node->line_num);
                a->has_errors = 1;
            }
        }

        analyze_expression(a, node->data.binary_op.left);
        analyze_expression(a, node->data.binary_op.right);
        return TYPE_INT;
    }

    case NODE_VAR:
    {
        Symbol *sym = lookup_symbol(a->symtab, node->data.var.name);
        if (!sym)
        {
            char msg[256];
            snprintf(msg, 256, "identificador '%s' nao declarado", node->data.var.name);
            semantic_error(msg, node->line_num);
            a->has_errors = 1;
            return TYPE_VOID;
        }

        /* CORREÇÃO 1: Verifica uso correto de arrays */
        if (node->data.var.index)
        {
            /* Tem índice - verifica se é array */
            if (!sym->is_array)
            {
                char msg[256];
                snprintf(msg, 256, "'%s' nao e um array", node->data.var.name);
                semantic_error(msg, node->line_num);
                a->has_errors = 1;
            }
            analyze_expression(a, node->data.var.index);
        }
        /* Arrays podem ser usados sem índice como argumentos de função */
        /* Não geramos erro aqui - o contexto determina se é válido */

        return sym->type;
    }

    case NODE_CALL:
    {
        Symbol *sym = lookup_symbol(a->symtab, node->data.call.name);

        /* Funções built-in */
        if (strcmp(node->data.call.name, "input") == 0)
        {
            return TYPE_INT;
        }
        if (strcmp(node->data.call.name, "output") == 0)
        {
            if (node->data.call.num_args != 1)
            {
                semantic_error("output espera 1 argumento", node->line_num);
                a->has_errors = 1;
            }
            for (int i = 0; i < node->data.call.num_args; i++)
            {
                analyze_expression(a, node->data.call.args[i]);
            }
            return TYPE_VOID;
        }

        if (!sym)
        {
            char msg[256];
            snprintf(msg, 256, "funcao '%s' nao declarada", node->data.call.name);
            semantic_error(msg, node->line_num);
            a->has_errors = 1;
            return TYPE_VOID;
        }

        if (sym->kind != SYM_FUNCTION)
        {
            char msg[256];
            snprintf(msg, 256, "'%s' nao e uma funcao", node->data.call.name);
            semantic_error(msg, node->line_num);
            a->has_errors = 1;
            return TYPE_VOID;
        }

        /* Analisa argumentos */
        for (int i = 0; i < node->data.call.num_args; i++)
        {
            analyze_expression(a, node->data.call.args[i]);
        }

        return sym->type;
    }

    case NODE_CONST:
        return TYPE_INT;

    default:
        return TYPE_VOID;
    }
}