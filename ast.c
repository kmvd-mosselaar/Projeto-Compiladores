#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Função auxiliar para criar um nó básico */
static ASTNode *create_node(NodeType type, int line)
{
    ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
    if (!node)
    {
        fprintf(stderr, "Erro de alocação de memória\n");
        exit(1);
    }
    node->type = type;
    node->line_num = line;
    node->sibling = NULL;
    return node;
}

ASTNode *create_program_node(ASTNode *decl_list)
{
    ASTNode *node = create_node(NODE_PROGRAM, 0);
    node->sibling = decl_list;
    return node;
}

ASTNode *create_var_decl_node(DataType type, char *name, int is_array, int array_size, int line)
{
    ASTNode *node = create_node(NODE_VAR_DECL, line);
    node->data.var_decl.data_type = type;
    node->data.var_decl.name = strdup(name);
    node->data.var_decl.is_array = is_array;
    node->data.var_decl.array_size = array_size;
    return node;
}

ASTNode *create_fun_decl_node(DataType return_type, char *name, ASTNode *params, ASTNode *body, int line)
{
    ASTNode *node = create_node(NODE_FUN_DECL, line);
    node->data.fun_decl.return_type = return_type;
    node->data.fun_decl.name = strdup(name);
    node->data.fun_decl.params = params;
    node->data.fun_decl.body = body;
    return node;
}

ASTNode *create_param_node(DataType type, char *name, int is_array, int line)
{
    ASTNode *node = create_node(NODE_PARAM, line);
    node->data.param.data_type = type;
    node->data.param.name = strdup(name);
    node->data.param.is_array = is_array;
    return node;
}

ASTNode *create_compound_stmt_node(ASTNode **local_decls, int num_local_decls,
                                   ASTNode **statements, int num_statements, int line)
{
    ASTNode *node = create_node(NODE_COMPOUND_STMT, line);
    node->data.compound.local_decls = local_decls;
    node->data.compound.num_local_decls = num_local_decls;
    node->data.compound.statements = statements;
    node->data.compound.num_statements = num_statements;
    return node;
}

ASTNode *create_if_stmt_node(ASTNode *condition, ASTNode *then_stmt, ASTNode *else_stmt, int line)
{
    ASTNode *node = create_node(NODE_IF_STMT, line);
    node->data.if_stmt.condition = condition;
    node->data.if_stmt.then_stmt = then_stmt;
    node->data.if_stmt.else_stmt = else_stmt;
    return node;
}

ASTNode *create_while_stmt_node(ASTNode *condition, ASTNode *body, int line)
{
    ASTNode *node = create_node(NODE_WHILE_STMT, line);
    node->data.while_stmt.condition = condition;
    node->data.while_stmt.body = body;
    return node;
}

ASTNode *create_return_stmt_node(ASTNode *expr, int line)
{
    ASTNode *node = create_node(NODE_RETURN_STMT, line);
    node->data.return_stmt.expr = expr;
    return node;
}

ASTNode *create_assign_node(ASTNode *var, ASTNode *expr, int line)
{
    ASTNode *node = create_node(NODE_ASSIGN, line);
    node->data.assign.var = var;
    node->data.assign.expr = expr;
    return node;
}

ASTNode *create_binary_op_node(OpType op, ASTNode *left, ASTNode *right, int line)
{
    ASTNode *node = create_node(NODE_BINARY_OP, line);
    node->data.binary_op.op = op;
    node->data.binary_op.left = left;
    node->data.binary_op.right = right;
    return node;
}

ASTNode *create_var_node(char *name, ASTNode *index, int line)
{
    ASTNode *node = create_node(NODE_VAR, line);
    node->data.var.name = strdup(name);
    node->data.var.index = index;
    return node;
}

ASTNode *create_call_node(char *name, ASTNode **args, int num_args, int line)
{
    ASTNode *node = create_node(NODE_CALL, line);
    node->data.call.name = strdup(name);
    node->data.call.args = args;
    node->data.call.num_args = num_args;
    return node;
}

ASTNode *create_const_node(int value, int line)
{
    ASTNode *node = create_node(NODE_CONST, line);
    node->data.const_val.value = value;
    return node;
}

ASTNode *create_expr_stmt_node(ASTNode *expr, int line)
{
    ASTNode *node = create_node(NODE_EXPR_STMT, line);
    node->data.expr_stmt.expr = expr;
    return node;
}

char *get_op_string(OpType op)
{
    switch (op)
    {
    case OP_ADD:
        return "+";
    case OP_SUB:
        return "-";
    case OP_MUL:
        return "*";
    case OP_DIV:
        return "/";
    case OP_LT:
        return "<";
    case OP_LE:
        return "<=";
    case OP_GT:
        return ">";
    case OP_GE:
        return ">=";
    case OP_EQ:
        return "==";
    case OP_NE:
        return "!=";
    default:
        return "?";
    }
}

char *get_type_string(DataType type)
{
    switch (type)
    {
    case TYPE_VOID:
        return "void";
    case TYPE_INT:
        return "int";
    default:
        return "unknown";
    }
}

/* ========== IMPRESSÃO VISUAL DA ÁRVORE - VERSÃO CORRIGIDA ========== */

#define MAX_DEPTH 50
static int prefix_stack[MAX_DEPTH];

static void print_prefix(int level, int is_last)
{
    for (int i = 0; i < level - 1; i++)
    {
        if (prefix_stack[i])
        {
            printf("│   ");
        }
        else
        {
            printf("    ");
        }
    }

    if (level > 0)
    {
        if (is_last)
        {
            printf("└── ");
        }
        else
        {
            printf("├── ");
        }
    }
}

static void print_ast_node(ASTNode *node, int level, int is_last);

static void print_ast_hierarchical(ASTNode *node, int level, int is_last)
{
    if (!node)
        return;

    /* Atualiza o stack de prefixos */
    if (level > 0)
    {
        prefix_stack[level - 1] = !is_last;
    }

    print_prefix(level, is_last);

    switch (node->type)
    {
    case NODE_PROGRAM:
        printf("PROGRAM\n");
        /* Processa declarações como filhos */
        if (node->sibling)
        {
            ASTNode *decl = node->sibling;
            while (decl)
            {
                int is_last_decl = (decl->sibling == NULL);
                print_ast_hierarchical(decl, level + 1, is_last_decl);
                decl = decl->sibling;
            }
        }
        return;

    case NODE_VAR_DECL:
        printf("VAR: %s", node->data.var_decl.name);
        if (node->data.var_decl.is_array)
            printf("[%d]", node->data.var_decl.array_size);
        printf(" : %s\n", get_type_string(node->data.var_decl.data_type));
        break;

    case NODE_FUN_DECL:
    {
        printf("FUNCTION: %s\n", node->data.fun_decl.name);

        ASTNode *param = node->data.fun_decl.params;
        int has_params = (param != NULL);
        int has_body = (node->data.fun_decl.body != NULL);

        if (has_params)
        {
            prefix_stack[level] = has_body;
            print_prefix(level + 1, !has_body);
            printf("PARAMS\n");

            while (param)
            {
                prefix_stack[level] = has_body;
                int is_last_param = (param->sibling == NULL);
                print_ast_hierarchical(param, level + 2, is_last_param);
                param = param->sibling;
            }
        }

        if (has_body)
        {
            prefix_stack[level] = 0;
            print_prefix(level + 1, 1);
            printf("BODY\n");
            print_ast_hierarchical(node->data.fun_decl.body, level + 2, 1);
        }
        break;
    }

    case NODE_PARAM:
        printf("PARAM: %s", node->data.param.name);
        if (node->data.param.is_array)
            printf("[]");
        printf(" : %s\n", get_type_string(node->data.param.data_type));
        break;

    case NODE_COMPOUND_STMT:
    {
        printf("BLOCK\n");

        int has_locals = (node->data.compound.num_local_decls > 0);
        int has_stmts = (node->data.compound.num_statements > 0);

        if (has_locals)
        {
            prefix_stack[level] = has_stmts;
            print_prefix(level + 1, !has_stmts);
            printf("LOCAL_VARS\n");

            for (int i = 0; i < node->data.compound.num_local_decls; i++)
            {
                prefix_stack[level] = has_stmts;
                int is_last = (i == node->data.compound.num_local_decls - 1);
                print_ast_hierarchical(node->data.compound.local_decls[i], level + 2, is_last);
            }
        }

        if (has_stmts)
        {
            prefix_stack[level] = 0;
            print_prefix(level + 1, 1);
            printf("STATEMENTS\n");

            for (int i = 0; i < node->data.compound.num_statements; i++)
            {
                prefix_stack[level] = 0;
                int is_last = (i == node->data.compound.num_statements - 1);
                print_ast_hierarchical(node->data.compound.statements[i], level + 2, is_last);
            }
        }
        break;
    }

    case NODE_IF_STMT:
        printf("IF\n");
        prefix_stack[level] = 1;
        print_prefix(level + 1, 0);
        printf("CONDITION\n");
        print_ast_hierarchical(node->data.if_stmt.condition, level + 2, 1);

        prefix_stack[level] = (node->data.if_stmt.else_stmt != NULL);
        print_prefix(level + 1, !node->data.if_stmt.else_stmt);
        printf("THEN\n");
        print_ast_hierarchical(node->data.if_stmt.then_stmt, level + 2, 1);

        if (node->data.if_stmt.else_stmt)
        {
            prefix_stack[level] = 0;
            print_prefix(level + 1, 1);
            printf("ELSE\n");
            print_ast_hierarchical(node->data.if_stmt.else_stmt, level + 2, 1);
        }
        break;

    case NODE_WHILE_STMT:
        printf("WHILE\n");
        prefix_stack[level] = 1;
        print_prefix(level + 1, 0);
        printf("CONDITION\n");
        print_ast_hierarchical(node->data.while_stmt.condition, level + 2, 1);

        prefix_stack[level] = 0;
        print_prefix(level + 1, 1);
        printf("BODY\n");
        print_ast_hierarchical(node->data.while_stmt.body, level + 2, 1);
        break;

    case NODE_RETURN_STMT:
        printf("RETURN\n");
        if (node->data.return_stmt.expr)
        {
            print_ast_hierarchical(node->data.return_stmt.expr, level + 1, 1);
        }
        break;

    case NODE_ASSIGN:
        printf("ASSIGN (=)\n");
        prefix_stack[level] = 1;
        print_prefix(level + 1, 0);
        printf("LEFT\n");
        print_ast_hierarchical(node->data.assign.var, level + 2, 1);

        prefix_stack[level] = 0;
        print_prefix(level + 1, 1);
        printf("RIGHT\n");
        print_ast_hierarchical(node->data.assign.expr, level + 2, 1);
        break;

    case NODE_BINARY_OP:
        printf("OP: %s\n", get_op_string(node->data.binary_op.op));
        prefix_stack[level] = 1;
        print_prefix(level + 1, 0);
        printf("LEFT\n");
        print_ast_hierarchical(node->data.binary_op.left, level + 2, 1);

        prefix_stack[level] = 0;
        print_prefix(level + 1, 1);
        printf("RIGHT\n");
        print_ast_hierarchical(node->data.binary_op.right, level + 2, 1);
        break;

    case NODE_VAR:
        printf("VAR: %s", node->data.var.name);
        if (node->data.var.index)
        {
            printf("[]\n");
            prefix_stack[level] = 0;
            print_prefix(level + 1, 1);
            printf("INDEX\n");
            print_ast_hierarchical(node->data.var.index, level + 2, 1);
        }
        else
        {
            printf("\n");
        }
        break;

    case NODE_CALL:
        printf("CALL: %s()\n", node->data.call.name);
        if (node->data.call.num_args > 0)
        {
            prefix_stack[level] = 0;
            print_prefix(level + 1, 1);
            printf("ARGS\n");

            for (int i = 0; i < node->data.call.num_args; i++)
            {
                prefix_stack[level] = 0;
                int is_last = (i == node->data.call.num_args - 1);
                print_ast_hierarchical(node->data.call.args[i], level + 2, is_last);
            }
        }
        break;

    case NODE_CONST:
        printf("CONST: %d\n", node->data.const_val.value);
        break;

    case NODE_EXPR_STMT:
        printf("EXPR_STMT\n");
        if (node->data.expr_stmt.expr)
        {
            print_ast_hierarchical(node->data.expr_stmt.expr, level + 1, 1);
        }
        break;

    default:
        printf("UNKNOWN\n");
    }
}

void print_ast(ASTNode *node, int indent)
{
    (void)indent;

    /* Inicializa o stack */
    for (int i = 0; i < MAX_DEPTH; i++)
    {
        prefix_stack[i] = 0;
    }

    printf("\n");
    printf("=================================================\n");
    printf("ARVORE DE ANALISE SINTATICA (AST)\n");
    printf("=================================================\n\n");

    if (node)
    {
        print_ast_hierarchical(node, 0, 1);
    }
    else
    {
        printf("(vazia)\n");
    }

    printf("\n=================================================\n\n");
}

/* Liberar memória da AST */
void free_ast(ASTNode *node)
{
    if (!node)
        return;

    switch (node->type)
    {
    case NODE_VAR_DECL:
        free(node->data.var_decl.name);
        break;
    case NODE_FUN_DECL:
        free(node->data.fun_decl.name);
        free_ast(node->data.fun_decl.params);
        free_ast(node->data.fun_decl.body);
        break;
    case NODE_PARAM:
        free(node->data.param.name);
        break;
    case NODE_COMPOUND_STMT:
        for (int i = 0; i < node->data.compound.num_local_decls; i++)
            free_ast(node->data.compound.local_decls[i]);
        if (node->data.compound.local_decls)
            free(node->data.compound.local_decls);
        for (int i = 0; i < node->data.compound.num_statements; i++)
            free_ast(node->data.compound.statements[i]);
        if (node->data.compound.statements)
            free(node->data.compound.statements);
        break;
    case NODE_IF_STMT:
        free_ast(node->data.if_stmt.condition);
        free_ast(node->data.if_stmt.then_stmt);
        free_ast(node->data.if_stmt.else_stmt);
        break;
    case NODE_WHILE_STMT:
        free_ast(node->data.while_stmt.condition);
        free_ast(node->data.while_stmt.body);
        break;
    case NODE_RETURN_STMT:
        free_ast(node->data.return_stmt.expr);
        break;
    case NODE_ASSIGN:
        free_ast(node->data.assign.var);
        free_ast(node->data.assign.expr);
        break;
    case NODE_BINARY_OP:
        free_ast(node->data.binary_op.left);
        free_ast(node->data.binary_op.right);
        break;
    case NODE_VAR:
        free(node->data.var.name);
        free_ast(node->data.var.index);
        break;
    case NODE_CALL:
        free(node->data.call.name);
        for (int i = 0; i < node->data.call.num_args; i++)
            free_ast(node->data.call.args[i]);
        if (node->data.call.args)
            free(node->data.call.args);
        break;
    case NODE_EXPR_STMT:
        free_ast(node->data.expr_stmt.expr);
        break;
    default:
        break;
    }

    free_ast(node->sibling);
    free(node);
}