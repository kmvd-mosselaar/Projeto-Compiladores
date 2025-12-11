/* codegen.h - Gerador de Código Intermediário */

#ifndef CODEGEN_H
#define CODEGEN_H

#include "ast.h"

#define MAX_CODE_LINES 10000
#define MAX_LINE_LENGTH 256

/* Estrutura do Gerador de Código */
typedef struct {
    char code[MAX_CODE_LINES][MAX_LINE_LENGTH];
    int line_count;
    int temp_counter;
    int label_counter;
} CodeGenerator;

/* Inicialização e finalização */
CodeGenerator* codegen_create(void);
void codegen_destroy(CodeGenerator *gen);

/* Função principal de geração */
void codegen_generate(CodeGenerator *gen, ASTNode *root);

/* Impressão do código gerado */
void codegen_print(CodeGenerator *gen);

#endif /* CODEGEN_H */