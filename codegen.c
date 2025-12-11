/* codegen.c - Implementação do Gerador de Código Intermediário */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

/* Protótipos internos */
static void gen_program(CodeGenerator *g, ASTNode *node);
static void gen_var_decl(CodeGenerator *g, ASTNode *node);
static void gen_fun_decl(CodeGenerator *g, ASTNode *node);
static void gen_compound_stmt(CodeGenerator *g, ASTNode *node);
static void gen_if_stmt(CodeGenerator *g, ASTNode *node);
static void gen_while_stmt(CodeGenerator *g, ASTNode *node);
static void gen_return_stmt(CodeGenerator *g, ASTNode *node);
static void gen_assign(CodeGenerator *g, ASTNode *node);
static void gen_expr(CodeGenerator *g, ASTNode *node, char *result);
static void gen_var(CodeGenerator *g, ASTNode *node, char *result);
static void gen_call(CodeGenerator *g, ASTNode *node, char *result);
static void gen_op(CodeGenerator *g, ASTNode *node, char *result);
static void emit(CodeGenerator *g, const char *line);
static void new_temp(CodeGenerator *g, char *temp);
static void new_label(CodeGenerator *g, char *label);
static const char* op_to_string(OpType op);

/* Criação do gerador */
CodeGenerator* codegen_create(void) {
    CodeGenerator *gen = (CodeGenerator*)malloc(sizeof(CodeGenerator));
    if (!gen) {
        fprintf(stderr, "Erro ao alocar memória para gerador de código\n");
        exit(1);
    }
    
    gen->line_count = 0;
    gen->temp_counter = 0;
    gen->label_counter = 0;
    
    return gen;
}

/* Destruição do gerador */
void codegen_destroy(CodeGenerator *gen) {
    if (gen) {
        free(gen);
    }
}

/* Geração de código */
void codegen_generate(CodeGenerator *gen, ASTNode *root) {
    if (!gen || !root) {
        return;
    }
    
    emit(gen, "# Codigo Intermediario C-");
    emit(gen, "");
    
    gen_program(gen, root);
}

/* Impressão do código */
void codegen_print(CodeGenerator *gen) {
    if (!gen) {
        return;
    }
    
    printf("\n========== CODIGO INTERMEDIARIO ==========\n");
    for (int i = 0; i < gen->line_count; i++) {
        printf("%s\n", gen->code[i]);
    }
    printf("==========================================\n\n");
}

/* Emite uma linha de código */
static void emit(CodeGenerator *g, const char *line) {
    if (g->line_count >= MAX_CODE_LINES) {
        fprintf(stderr, "Erro: número máximo de linhas excedido\n");
        return;
    }
    strncpy(g->code[g->line_count], line, MAX_LINE_LENGTH - 1);
    g->code[g->line_count][MAX_LINE_LENGTH - 1] = '\0';
    g->line_count++;
}

/* Cria novo temporário */
static void new_temp(CodeGenerator *g, char *temp) {
    sprintf(temp, "t%d", g->temp_counter++);
}

/* Cria novo label */
static void new_label(CodeGenerator *g, char *label) {
    sprintf(label, "L%d", g->label_counter++);
}

/* Converte operador para string */
static const char* op_to_string(OpType op) {
    switch (op) {
        case OP_ADD: return "+";
        case OP_SUB: return "-";
        case OP_MUL: return "*";
        case OP_DIV: return "/";
        case OP_LT: return "<";
        case OP_LE: return "<=";
        case OP_GT: return ">";
        case OP_GE: return ">=";
        case OP_EQ: return "==";
        case OP_NE: return "!=";
        default: return "?";
    }
}

/* Gera código para o programa */
static void gen_program(CodeGenerator *g, ASTNode *node) {
    if (node->kind != NODE_PROGRAM) {
        return;
    }
    
    ASTNode *child = node->child;
    while (child) {
        if (child->kind == NODE_VAR_DECL) {
            gen_var_decl(g, child);
        } else if (child->kind == NODE_FUN_DECL) {
            gen_fun_decl(g, child);
        }
        child = child->sibling;
    }
}

/* Gera código para declaração de variável */
static void gen_var_decl(CodeGenerator *g, ASTNode *node) {
    char line[MAX_LINE_LENGTH];
    
    if (node->is_array) {
        sprintf(line, "declare_array %s[%d]", node->name, node->array_size);
    } else {
        sprintf(line, "declare %s", node->name);
    }
    emit(g, line);
}

/* Gera código para declaração de função */
static void gen_fun_decl(CodeGenerator *g, ASTNode *node) {
    char line[MAX_LINE_LENGTH];
    
    emit(g, "");
    sprintf(line, "function %s:", node->name);
    emit(g, line);
    
    /* Parâmetros */
    ASTNode *param = node->child;
    while (param && param->kind == NODE_PARAM) {
        sprintf(line, "  param %s", param->name);
        emit(g, line);
        param = param->sibling;
    }
    
    /* Corpo */
    ASTNode *body = node->child;
    while (body && body->kind == NODE_PARAM) {
        body = body->sibling;
    }
    
    if (body && body->kind == NODE_COMPOUND_STMT) {
        gen_compound_stmt(g, body);
    }
    
    sprintf(line, "end_function %s", node->name);
    emit(g, line);
}

/* Gera código para bloco composto */
static void gen_compound_stmt(CodeGenerator *g, ASTNode *node) {
    ASTNode *child = node->child;
    
    /* Declarações locais */
    while (child && child->kind == NODE_VAR_DECL) {
        emit(g, "  # local var");
        gen_var_decl(g, child);
        child = child->sibling;
    }
    
    /* Statements */
    while (child) {
        switch (child->kind) {
            case NODE_IF_STMT:
                gen_if_stmt(g, child);
                break;
            case NODE_WHILE_STMT:
                gen_while_stmt(g, child);
                break;
            case NODE_RETURN_STMT:
                gen_return_stmt(g, child);
                break;
            case NODE_ASSIGN:
                gen_assign(g, child);
                break;
            case NODE_CALL:
                {
                    char result[64];
                    gen_call(g, child, result);
                }
                break;
            default:
                break;
        }
        child = child->sibling;
    }
}

/* Gera código para if */
static void gen_if_stmt(CodeGenerator *g, ASTNode *node) {
    char cond[64], label_else[64], label_end[64], line[MAX_LINE_LENGTH];
    
    /* Avalia condição */
    if (node->child) {
        gen_expr(g, node->child, cond);
    }
    
    new_label(g, label_else);
    new_label(g, label_end);
    
    sprintf(line, "  if_false %s goto %s", cond, label_else);
    emit(g, line);
    
    /* Then */
    ASTNode *then_stmt = node->child ? node->child->sibling : NULL;
    if (then_stmt && then_stmt->kind == NODE_COMPOUND_STMT) {
        gen_compound_stmt(g, then_stmt);
    }
    
    /* Else */
    ASTNode *else_stmt = then_stmt ? then_stmt->sibling : NULL;
    if (else_stmt) {
        sprintf(line, "  goto %s", label_end);
        emit(g, line);
        sprintf(line, "%s:", label_else);
        emit(g, line);
        
        if (else_stmt->kind == NODE_COMPOUND_STMT) {
            gen_compound_stmt(g, else_stmt);
        }
        
        sprintf(line, "%s:", label_end);
        emit(g, line);
    } else {
        sprintf(line, "%s:", label_else);
        emit(g, line);
    }
}

/* Gera código para while */
static void gen_while_stmt(CodeGenerator *g, ASTNode *node) {
    char cond[64], label_start[64], label_end[64], line[MAX_LINE_LENGTH];
    
    new_label(g, label_start);
    new_label(g, label_end);
    
    sprintf(line, "%s:", label_start);
    emit(g, line);
    
    /* Condição */
    if (node->child) {
        gen_expr(g, node->child, cond);
    }
    
    sprintf(line, "  if_false %s goto %s", cond, label_end);
    emit(g, line);
    
    /* Corpo */
    ASTNode *body = node->child ? node->child->sibling : NULL;
    if (body && body->kind == NODE_COMPOUND_STMT) {
        gen_compound_stmt(g, body);
    }
    
    sprintf(line, "  goto %s", label_start);
    emit(g, line);
    sprintf(line, "%s:", label_end);
    emit(g, line);
}

/* Gera código para return */
static void gen_return_stmt(CodeGenerator *g, ASTNode *node) {
    char line[MAX_LINE_LENGTH];
    
    if (node->child) {
        char result[64];
        gen_expr(g, node->child, result);
        sprintf(line, "  return %s", result);
        emit(g, line);
    } else {
        emit(g, "  return");
    }
}

/* Gera código para atribuição */
static void gen_assign(CodeGenerator *g, ASTNode *node) {
    char var[64], expr[64], line[MAX_LINE_LENGTH];
    
    gen_var(g, node->child, var);
    
    ASTNode *expr_node = node->child ? node->child->sibling : NULL;
    if (expr_node) {
        gen_expr(g, expr_node, expr);
        sprintf(line, "  %s = %s", var, expr);
        emit(g, line);
    }
}

/* Gera código para expressão */
static void gen_expr(CodeGenerator *g, ASTNode *node, char *result) {
    if (!node) {
        strcpy(result, "");
        return;
    }
    
    switch (node->kind) {
        case NODE_VAR:
            gen_var(g, node, result);
            break;
        case NODE_CALL:
            gen_call(g, node, result);
            break;
        case NODE_BINARY_OP:
            gen_op(g, node, result);
            break;
        case NODE_CONST:
            sprintf(result, "%d", node->value);
            break;
        default:
            strcpy(result, "");
            break;
    }
}

/* Gera código para variável */
static void gen_var(CodeGenerator *g, ASTNode *node, char *result) {
    if (node->child) {
        /* Acesso a array */
        char index[64], temp[64], line[MAX_LINE_LENGTH];
        gen_expr(g, node->child, index);
        new_temp(g, temp);
        sprintf(line, "  %s = %s[%s]", temp, node->name, index);
        emit(g, line);
        strcpy(result, temp);
    } else {
        strcpy(result, node->name);
    }
}

/* Gera código para chamada de função */
static void gen_call(CodeGenerator *g, ASTNode *node, char *result) {
    char line[MAX_LINE_LENGTH];
    
    /* Gera código para argumentos */
    ASTNode *arg = node->child;
    while (arg) {
        char arg_result[64];
        gen_expr(g, arg, arg_result);
        sprintf(line, "  push %s", arg_result);
        emit(g, line);
        arg = arg->sibling;
    }
    
    /* Funções built-in */
    if (strcmp(node->name, "output") == 0) {
        /* output já tem argumento em push */
        arg = node->child;
        if (arg) {
            char arg_result[64];
            gen_expr(g, arg, arg_result);
            sprintf(line, "  output %s", arg_result);
            emit(g, line);
        }
        strcpy(result, "");
    } else if (strcmp(node->name, "input") == 0) {
        char temp[64];
        new_temp(g, temp);
        sprintf(line, "  %s = input", temp);
        emit(g, line);
        strcpy(result, temp);
    } else {
        char temp[64];
        new_temp(g, temp);
        sprintf(line, "  %s = call %s", temp, node->name);
        emit(g, line);
        strcpy(result, temp);
    }
}

/* Gera código para operação binária */
static void gen_op(CodeGenerator *g, ASTNode *node, char *result) {
    char left[64], right[64], temp[64], line[MAX_LINE_LENGTH];
    
    if (node->child) {
        gen_expr(g, node->child, left);
    }
    if (node->child && node->child->sibling) {
        gen_expr(g, node->child->sibling, right);
    }
    
    new_temp(g, temp);
    sprintf(line, "  %s = %s %s %s", temp, left, op_to_string(node->op), right);
    emit(g, line);
    strcpy(result, temp);
}