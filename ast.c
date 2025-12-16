#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ast.h"

/* Canvas maior para árvores complexas */
#define MAX_LINES 60
#define MAX_WIDTH 200
static char canvas[MAX_LINES][MAX_WIDTH];

/* Espaçamento mínimo entre nós */
#define MIN_NODE_SPACING 3

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

/* === FUNÇÕES PARA IMPRESSÃO VISUAL MELHORADA === */

/* Obtém o texto do nó */
static void get_node_text(ASTNode *node, char *buffer, int max_len)
{
    if (!node)
    {
        snprintf(buffer, max_len, "NULL");
        return;
    }

    switch (node->type)
    {
    case NODE_PROGRAM:
        snprintf(buffer, max_len, "PROGRAM");
        break;
    case NODE_VAR_DECL:
        if (node->data.var_decl.is_array)
            snprintf(buffer, max_len, "%s:%s[%d]", node->data.var_decl.name,
                     get_type_string(node->data.var_decl.data_type),
                     node->data.var_decl.array_size);
        else
            snprintf(buffer, max_len, "%s:%s", node->data.var_decl.name,
                     get_type_string(node->data.var_decl.data_type));
        break;
    case NODE_FUN_DECL:
        snprintf(buffer, max_len, "FUN:%s", node->data.fun_decl.name);
        break;
    case NODE_PARAM:
        if (node->data.param.is_array)
            snprintf(buffer, max_len, "PARAM:%s[]", node->data.param.name);
        else
            snprintf(buffer, max_len, "PARAM:%s", node->data.param.name);
        break;
    case NODE_COMPOUND_STMT:
        snprintf(buffer, max_len, "BLOCK");
        break;
    case NODE_IF_STMT:
        snprintf(buffer, max_len, "IF");
        break;
    case NODE_WHILE_STMT:
        snprintf(buffer, max_len, "WHILE");
        break;
    case NODE_RETURN_STMT:
        snprintf(buffer, max_len, "RETURN");
        break;
    case NODE_ASSIGN:
        snprintf(buffer, max_len, "=");
        break;
    case NODE_BINARY_OP:
        snprintf(buffer, max_len, "(%s)", get_op_string(node->data.binary_op.op));
        break;
    case NODE_VAR:
        if (node->data.var.index)
            snprintf(buffer, max_len, "%s[]", node->data.var.name);
        else
            snprintf(buffer, max_len, "%s", node->data.var.name);
        break;
    case NODE_CALL:
        snprintf(buffer, max_len, "CALL:%s", node->data.call.name);
        break;
    case NODE_CONST:
        snprintf(buffer, max_len, "%d", node->data.const_val.value);
        break;
    case NODE_EXPR_STMT:
        snprintf(buffer, max_len, "STMT");
        break;
    default:
        snprintf(buffer, max_len, "?");
    }
}

/* Calcula a largura necessária para uma subárvore */
static int calculate_width(ASTNode *node)
{
    if (!node)
        return 0;

    char text[40];
    get_node_text(node, text, sizeof(text));
    int node_width = strlen(text) + MIN_NODE_SPACING;

    int children_width = 0;
    int num_children = 0;

    switch (node->type)
    {
    case NODE_FUN_DECL:
        /* Contar TODOS os parâmetros (percorrendo siblings) */
        if (node->data.fun_decl.params)
        {
            ASTNode *param = node->data.fun_decl.params;
            while (param)
            {
                children_width += calculate_width(param);
                num_children++;
                param = param->sibling;
            }
        }
        if (node->data.fun_decl.body)
        {
            children_width += calculate_width(node->data.fun_decl.body);
            num_children++;
        }
        break;
    case NODE_COMPOUND_STMT:
        for (int i = 0; i < node->data.compound.num_local_decls; i++)
        {
            children_width += calculate_width(node->data.compound.local_decls[i]);
            num_children++;
        }
        for (int i = 0; i < node->data.compound.num_statements; i++)
        {
            children_width += calculate_width(node->data.compound.statements[i]);
            num_children++;
        }
        break;
    case NODE_IF_STMT:
        if (node->data.if_stmt.condition)
        {
            children_width += calculate_width(node->data.if_stmt.condition);
            num_children++;
        }
        if (node->data.if_stmt.then_stmt)
        {
            children_width += calculate_width(node->data.if_stmt.then_stmt);
            num_children++;
        }
        if (node->data.if_stmt.else_stmt)
        {
            children_width += calculate_width(node->data.if_stmt.else_stmt);
            num_children++;
        }
        break;
    case NODE_WHILE_STMT:
        if (node->data.while_stmt.condition)
        {
            children_width += calculate_width(node->data.while_stmt.condition);
            num_children++;
        }
        if (node->data.while_stmt.body)
        {
            children_width += calculate_width(node->data.while_stmt.body);
            num_children++;
        }
        break;
    case NODE_RETURN_STMT:
        if (node->data.return_stmt.expr)
        {
            children_width = calculate_width(node->data.return_stmt.expr);
            num_children = 1;
        }
        break;
    case NODE_ASSIGN:
        children_width = calculate_width(node->data.assign.var) +
                         calculate_width(node->data.assign.expr);
        num_children = 2;
        break;
    case NODE_BINARY_OP:
        children_width = calculate_width(node->data.binary_op.left) +
                         calculate_width(node->data.binary_op.right);
        num_children = 2;
        break;
    case NODE_VAR:
        if (node->data.var.index)
        {
            children_width = calculate_width(node->data.var.index);
            num_children = 1;
        }
        break;
    case NODE_CALL:
        for (int i = 0; i < node->data.call.num_args; i++)
        {
            children_width += calculate_width(node->data.call.args[i]);
            num_children++;
        }
        break;
    case NODE_EXPR_STMT:
        if (node->data.expr_stmt.expr)
        {
            children_width = calculate_width(node->data.expr_stmt.expr);
            num_children = 1;
        }
        break;
    default:
        break;
    }

    /* A largura do nó é o máximo entre sua própria largura e a soma dos filhos */
    int total_width = (children_width > node_width) ? children_width : node_width;

    /* Garantir largura mínima */
    if (total_width < 8)
        total_width = 8;

    return total_width;
}

/* Escreve texto centralizado em uma posição */
static void write_centered(int row, int col, const char *text)
{
    if (row < 0 || row >= MAX_LINES)
        return;

    int len = strlen(text);
    int start = col - len / 2;

    if (start < 0)
        start = 0;
    if (start + len >= MAX_WIDTH)
        start = MAX_WIDTH - len - 1;

    for (int i = 0; i < len && start + i < MAX_WIDTH - 1; i++)
    {
        canvas[row][start + i] = text[i];
    }
}

/* Desenha uma linha de conexão */
static void draw_connector(int row, int from_col, int to_col)
{
    if (row < 0 || row >= MAX_LINES)
        return;

    if (from_col == to_col)
    {
        if (from_col >= 0 && from_col < MAX_WIDTH - 1)
            canvas[row][from_col] = '|';
        return;
    }

    int start = (from_col < to_col) ? from_col : to_col;
    int end = (from_col < to_col) ? to_col : from_col;

    for (int col = start; col <= end; col++)
    {
        if (col < 0 || col >= MAX_WIDTH - 1)
            continue;

        if (col == from_col || col == to_col)
        {
            if (col == from_col && from_col < to_col)
                canvas[row][col] = '/';
            else if (col == to_col && from_col < to_col)
                canvas[row][col] = '\\';
            else if (col == from_col && from_col > to_col)
                canvas[row][col] = '\\';
            else if (col == to_col && from_col > to_col)
                canvas[row][col] = '/';
        }
        else
        {
            if (canvas[row][col] == ' ')
                canvas[row][col] = '-';
        }
    }
}

/* Preenche recursivamente com melhor espaçamento */
static void fill_buffer_improved(ASTNode *node, int level, int left, int right)
{
    if (!node || level >= MAX_LINES / 3)
        return;

    int width = right - left;
    int center = left + width / 2;

    /* Escreve o nó atual */
    char text[40];
    get_node_text(node, text, sizeof(text));
    write_centered(level * 3, center, text);

    /* Processa filhos */
    typedef struct
    {
        ASTNode *node;
        int width;
    } ChildInfo;

    ChildInfo children[20];
    int num_children = 0;

#define ADD_CHILD(child)                                       \
    if (child && num_children < 20)                            \
    {                                                          \
        children[num_children].node = child;                   \
        children[num_children].width = calculate_width(child); \
        num_children++;                                        \
    }

    switch (node->type)
    {
    case NODE_FUN_DECL:
        /* Adicionar TODOS os parâmetros (percorrendo siblings) */
        if (node->data.fun_decl.params)
        {
            ASTNode *param = node->data.fun_decl.params;
            while (param)
            {
                ADD_CHILD(param);
                param = param->sibling;
            }
        }
        ADD_CHILD(node->data.fun_decl.body);
        break;
    case NODE_COMPOUND_STMT:
        for (int i = 0; i < node->data.compound.num_local_decls; i++)
            ADD_CHILD(node->data.compound.local_decls[i]);
        for (int i = 0; i < node->data.compound.num_statements; i++)
            ADD_CHILD(node->data.compound.statements[i]);
        break;
    case NODE_IF_STMT:
        ADD_CHILD(node->data.if_stmt.condition);
        ADD_CHILD(node->data.if_stmt.then_stmt);
        ADD_CHILD(node->data.if_stmt.else_stmt);
        break;
    case NODE_WHILE_STMT:
        ADD_CHILD(node->data.while_stmt.condition);
        ADD_CHILD(node->data.while_stmt.body);
        break;
    case NODE_RETURN_STMT:
        ADD_CHILD(node->data.return_stmt.expr);
        break;
    case NODE_ASSIGN:
        ADD_CHILD(node->data.assign.var);
        ADD_CHILD(node->data.assign.expr);
        break;
    case NODE_BINARY_OP:
        ADD_CHILD(node->data.binary_op.left);
        ADD_CHILD(node->data.binary_op.right);
        break;
    case NODE_VAR:
        ADD_CHILD(node->data.var.index);
        break;
    case NODE_CALL:
        for (int i = 0; i < node->data.call.num_args; i++)
            ADD_CHILD(node->data.call.args[i]);
        break;
    case NODE_EXPR_STMT:
        ADD_CHILD(node->data.expr_stmt.expr);
        break;
    default:
        break;
    }

#undef ADD_CHILD

    if (num_children == 0)
        return;

    /* Calcula posições dos filhos */
    int total_children_width = 0;
    for (int i = 0; i < num_children; i++)
    {
        total_children_width += children[i].width;
    }

    /* Espaço disponível */
    int available_width = width;
    if (total_children_width > available_width)
    {
        available_width = total_children_width;
    }

    /* Distribui espaço */
    int current_left = left;
    if (total_children_width < available_width)
    {
        current_left = left + (available_width - total_children_width) / 2;
    }

    /* Desenha filhos */
    for (int i = 0; i < num_children; i++)
    {
        int child_width = children[i].width;
        int child_center = current_left + child_width / 2;

        /* Desenha conector */
        draw_connector(level * 3 + 1, center, child_center);

        /* Desenha filho recursivamente */
        fill_buffer_improved(children[i].node, level + 1, current_left, current_left + child_width);

        current_left += child_width;
    }
}

/* Imprime a árvore com espaçamento melhorado */
void print_ast(ASTNode *node, int indent)
{
    (void)indent;

    /* Limpa o canvas */
    for (int i = 0; i < MAX_LINES; i++)
    {
        for (int j = 0; j < MAX_WIDTH; j++)
        {
            canvas[i][j] = ' ';
        }
        canvas[i][MAX_WIDTH - 1] = '\0';
    }

    int width = calculate_width(node);
    if (width > MAX_WIDTH - 10)
        width = MAX_WIDTH - 10;

    fill_buffer_improved(node, 0, 0, width);

    /* Imprime apenas linhas não vazias */
    for (int i = 0; i < MAX_LINES; i++)
    {
        int j = MAX_WIDTH - 2;
        while (j > 0 && canvas[i][j] == ' ')
            j--;

        /* Pula linhas completamente vazias */
        int has_content = 0;
        for (int k = 0; k <= j; k++)
        {
            if (canvas[i][k] != ' ')
            {
                has_content = 1;
                break;
            }
        }

        if (has_content)
        {
            canvas[i][j + 1] = '\0';
            printf("%s\n", canvas[i]);
        }
    }

    /* Processa irmãos */
    if (node && node->sibling)
    {
        printf("\n");
        print_ast(node->sibling, indent);
    }
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