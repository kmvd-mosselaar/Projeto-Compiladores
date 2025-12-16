/* codegen.c - Implementação do Gerador de Código Intermediário CORRIGIDO */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"

/* Protótipos internos */
static void gen_node(CodeGenerator *g, ASTNode *node);
static void gen_var_decl(CodeGenerator *g, ASTNode *node);
static void gen_fun_decl(CodeGenerator *g, ASTNode *node);
static void gen_compound_stmt(CodeGenerator *g, ASTNode *node);
static void gen_statement(CodeGenerator *g, ASTNode *node);
static void gen_expr(CodeGenerator *g, ASTNode *node, char *result);
static void emit(CodeGenerator *g, const char *line);
static void new_temp(CodeGenerator *g, char *temp);
static void new_label(CodeGenerator *g, char *label);

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
    
    /* Processa programa */
    if (root->type == NODE_PROGRAM) {
        ASTNode *decl = root->sibling;
        while (decl) {
            gen_node(gen, decl);
            decl = decl->sibling;
        }
    }
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

/* Gera código para nó genérico */
static void gen_node(CodeGenerator *g, ASTNode *node) {
    if (!node) return;
    
    switch(node->type) {
        case NODE_VAR_DECL:
            gen_var_decl(g, node);
            break;
        case NODE_FUN_DECL:
            gen_fun_decl(g, node);
            break;
        case NODE_COMPOUND_STMT:
            gen_compound_stmt(g, node);
            break;
        default:
            gen_statement(g, node);
            break;
    }
}

/* Gera código para declaração de variável */
static void gen_var_decl(CodeGenerator *g, ASTNode *node) {
    char line[MAX_LINE_LENGTH];
    
    if (node->data.var_decl.is_array) {
        sprintf(line, "  declare_array %s[%d]", 
                node->data.var_decl.name, 
                node->data.var_decl.array_size);
    } else {
        sprintf(line, "  declare %s", node->data.var_decl.name);
    }
    emit(g, line);
}

/* Gera código para declaração de função */
static void gen_fun_decl(CodeGenerator *g, ASTNode *node) {
    char line[MAX_LINE_LENGTH];
    
    emit(g, "");
    sprintf(line, "function %s:", node->data.fun_decl.name);
    emit(g, line);
    
    /* Parâmetros */
    ASTNode *param = node->data.fun_decl.params;
    while (param) {
        sprintf(line, "  param %s", param->data.param.name);
        emit(g, line);
        param = param->sibling;
    }
    
    /* Corpo */
    if (node->data.fun_decl.body) {
        gen_compound_stmt(g, node->data.fun_decl.body);
    }
    
    sprintf(line, "end_function %s", node->data.fun_decl.name);
    emit(g, line);
}

/* Gera código para bloco composto */
static void gen_compound_stmt(CodeGenerator *g, ASTNode *node) {
    /* Declarações locais */
    for (int i = 0; i < node->data.compound.num_local_decls; i++) {
        gen_var_decl(g, node->data.compound.local_decls[i]);
    }
    
    /* Statements */
    for (int i = 0; i < node->data.compound.num_statements; i++) {
        gen_statement(g, node->data.compound.statements[i]);
    }
}

/* Gera código para statement */
static void gen_statement(CodeGenerator *g, ASTNode *node) {
    if (!node) return;
    
    char line[MAX_LINE_LENGTH];
    
    switch(node->type) {
        case NODE_COMPOUND_STMT:
            gen_compound_stmt(g, node);
            break;
            
        case NODE_IF_STMT: {
            char cond[64], label_else[64], label_end[64];
            
            /* Avalia condição */
            gen_expr(g, node->data.if_stmt.condition, cond);
            
            new_label(g, label_else);
            new_label(g, label_end);
            
            sprintf(line, "  if_false %s goto %s", cond, label_else);
            emit(g, line);
            
            /* Then */
            gen_statement(g, node->data.if_stmt.then_stmt);
            
            /* Else */
            if (node->data.if_stmt.else_stmt) {
                sprintf(line, "  goto %s", label_end);
                emit(g, line);
                sprintf(line, "%s:", label_else);
                emit(g, line);
                
                gen_statement(g, node->data.if_stmt.else_stmt);
                
                sprintf(line, "%s:", label_end);
                emit(g, line);
            } else {
                sprintf(line, "%s:", label_else);
                emit(g, line);
            }
            break;
        }
        
        case NODE_WHILE_STMT: {
            char cond[64], label_start[64], label_end[64];
            
            new_label(g, label_start);
            new_label(g, label_end);
            
            sprintf(line, "%s:", label_start);
            emit(g, line);
            
            /* Condição */
            gen_expr(g, node->data.while_stmt.condition, cond);
            
            sprintf(line, "  if_false %s goto %s", cond, label_end);
            emit(g, line);
            
            /* Corpo */
            gen_statement(g, node->data.while_stmt.body);
            
            sprintf(line, "  goto %s", label_start);
            emit(g, line);
            sprintf(line, "%s:", label_end);
            emit(g, line);
            break;
        }
        
        case NODE_RETURN_STMT:
            if (node->data.return_stmt.expr) {
                char result[64];
                gen_expr(g, node->data.return_stmt.expr, result);
                sprintf(line, "  return %s", result);
                emit(g, line);
            } else {
                emit(g, "  return");
            }
            break;
            
        case NODE_EXPR_STMT:
            if (node->data.expr_stmt.expr) {
                char result[64];
                gen_expr(g, node->data.expr_stmt.expr, result);
            }
            break;
            
        default:
            break;
    }
}

/* Gera código para expressão */
static void gen_expr(CodeGenerator *g, ASTNode *node, char *result) {
    if (!node) {
        strcpy(result, "");
        return;
    }
    
    char line[MAX_LINE_LENGTH];
    
    switch(node->type) {
        case NODE_ASSIGN: {
            char var[64], expr[64];
            
            /* Gera código para variável */
            if (node->data.assign.var->data.var.index) {
                char index[64];
                gen_expr(g, node->data.assign.var->data.var.index, index);
                sprintf(var, "%s[%s]", node->data.assign.var->data.var.name, index);
            } else {
                strcpy(var, node->data.assign.var->data.var.name);
            }
            
            /* Gera código para expressão */
            gen_expr(g, node->data.assign.expr, expr);
            
            sprintf(line, "  %s = %s", var, expr);
            emit(g, line);
            strcpy(result, var);
            break;
        }
        
        case NODE_BINARY_OP: {
            char left[64], right[64], temp[64];
            
            gen_expr(g, node->data.binary_op.left, left);
            gen_expr(g, node->data.binary_op.right, right);
            
            new_temp(g, temp);
            sprintf(line, "  %s = %s %s %s", 
                    temp, left, get_op_string(node->data.binary_op.op), right);
            emit(g, line);
            strcpy(result, temp);
            break;
        }
        
        case NODE_VAR:
            if (node->data.var.index) {
                char index[64], temp[64];
                gen_expr(g, node->data.var.index, index);
                new_temp(g, temp);
                sprintf(line, "  %s = %s[%s]", temp, node->data.var.name, index);
                emit(g, line);
                strcpy(result, temp);
            } else {
                strcpy(result, node->data.var.name);
            }
            break;
            
        case NODE_CALL: {
            /* Gera código para argumentos */
            for (int i = 0; i < node->data.call.num_args; i++) {
                char arg_result[64];
                gen_expr(g, node->data.call.args[i], arg_result);
                sprintf(line, "  push %s", arg_result);
                emit(g, line);
            }
            
            /* Funções built-in */
            if (strcmp(node->data.call.name, "output") == 0) {
                if (node->data.call.num_args > 0) {
                    char arg[64];
                    gen_expr(g, node->data.call.args[0], arg);
                    sprintf(line, "  output %s", arg);
                    emit(g, line);
                }
                strcpy(result, "");
            } else if (strcmp(node->data.call.name, "input") == 0) {
                char temp[64];
                new_temp(g, temp);
                sprintf(line, "  %s = input", temp);
                emit(g, line);
                strcpy(result, temp);
            } else {
                char temp[64];
                new_temp(g, temp);
                sprintf(line, "  %s = call %s", temp, node->data.call.name);
                emit(g, line);
                strcpy(result, temp);
            }
            break;
        }
        
        case NODE_CONST:
            sprintf(result, "%d", node->data.const_val.value);
            break;
            
        default:
            strcpy(result, "");
            break;
    }
}