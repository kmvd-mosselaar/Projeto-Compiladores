/* main.c - Compilador C- Completo */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"
#include "symtab.h"
#include "semantic.h"
#include "codegen.h"

extern int yyparse();
extern FILE *yyin;
extern ASTNode *syntax_tree;
extern int line_num;

void print_usage(char *prog_name)
{
    fprintf(stderr, "Uso: %s <arquivo_entrada.cm>\n", prog_name);
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        print_usage(argv[0]);
    }

    /* Abrir arquivo de entrada */
    yyin = fopen(argv[1], "r");
    if (!yyin)
    {
        fprintf(stderr, "Erro: não foi possível abrir o arquivo '%s'\n", argv[1]);
        return 1;
    }

    printf("=================================================\n");
    printf("COMPILADOR C- - Pipeline Completo\n");
    printf("=================================================\n");
    printf("Arquivo de entrada: %s\n\n", argv[1]);

    /* ========================================
       FASE 1: ANÁLISE LÉXICA E SINTÁTICA
       ======================================== */
    printf(">>> FASE 1: Analise Lexica e Sintatica\n");
    printf("Iniciando analise...\n");

    int parse_result = yyparse();

    if (parse_result != 0)
    {
        fprintf(stderr, "\n[ERRO] Compilacao abortada devido a erros sintaticos.\n");
        fclose(yyin);
        return 1;
    }

    printf("[OK] Analise sintatica concluida com sucesso!\n\n");

    /* Verificar se a AST foi construída */
    if (!syntax_tree)
    {
        fprintf(stderr, "[ERRO] Arvore sintatica nao foi construida!\n");
        fclose(yyin);
        return 1;
    }

    /* ========================================
       FASE 2: CONSTRUÇÃO DA TABELA DE SÍMBOLOS
       ======================================== */
    printf(">>> FASE 2: Construcao da Tabela de Simbolos\n");

    SymbolTable *symtab = create_symbol_table();
    build_symbol_table(syntax_tree, symtab);

    printf("[OK] Tabela de simbolos construida!\n");
    printf("Total de simbolos: %d\n", symtab->num_symbols);
    printf("Total de escopos: %d\n\n", symtab->num_scopes);

    /* Imprimir a tabela de símbolos */
    print_complete_symbol_table(symtab, syntax_tree);

    /* ========================================
       FASE 3: ANÁLISE SEMÂNTICA
       ======================================== */
    printf("\n>>> FASE 3: Analise Semantica\n");

    SemanticAnalyzer *semantic = semantic_create();
    int semantic_ok = semantic_analyze(semantic, syntax_tree);

    if (!semantic_ok)
    {
        fprintf(stderr, "\n[ERRO] Analise semantica encontrou erros!\n");
        fprintf(stderr, "Compilacao abortada.\n\n");

        /* Limpar memória */
        semantic_destroy(semantic);
        free_symbol_table(symtab);
        free_ast(syntax_tree);
        fclose(yyin);
        return 1;
    }

    printf("[OK] Analise semantica concluida sem erros!\n\n");

    /* ========================================
       FASE 4: GERAÇÃO DE CÓDIGO INTERMEDIÁRIO
       ======================================== */
    printf(">>> FASE 4: Geracao de Codigo Intermediario\n");

    CodeGenerator *codegen = codegen_create();
    codegen_generate(codegen, syntax_tree);

    printf("[OK] Codigo intermediario gerado com sucesso!\n");

    /* Imprimir o código intermediário */
    codegen_print(codegen);

    /* ========================================
       IMPRESSÃO DA AST (OPCIONAL)
       ======================================== */
    if (syntax_tree)
    {
        print_ast(syntax_tree, 0);
    }
    else
    {
        printf("Nenhuma arvore gerada.\n");
    }

    /* ========================================
       FINALIZAÇÃO
       ======================================== */
    printf("\n=================================================\n");
    printf("COMPILACAO CONCLUIDA COM SUCESSO!\n");
    printf("=================================================\n");
    printf("\nResumo:\n");
    printf("  - Arquivo: %s\n", argv[1]);
    printf("  - Simbolos: %d\n", symtab->num_symbols);
    printf("  - Escopos: %d\n", symtab->num_scopes);
    printf("  - Codigo gerado: %d linhas\n", codegen->line_count);
    printf("\nTodas as fases concluidas sem erros!\n");
    printf("=================================================\n\n");

    /* Limpar memória */
    codegen_destroy(codegen);
    semantic_destroy(semantic);
    free_symbol_table(symtab);
    free_ast(syntax_tree);
    fclose(yyin);

    return 0;
}