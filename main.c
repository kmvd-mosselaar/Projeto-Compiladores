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
    printf("Compilador C- - Análise Léxica e Sintática\n");
    printf("=================================================\n");
    printf("Arquivo de entrada: %s\n\n", argv[1]);

    /* Executar análise léxica e sintática */
    printf("Iniciando análise...\n");
    int parse_result = yyparse();

    if (parse_result != 0)
    {
        fprintf(stderr, "\nCompilação abortada devido a erros.\n");
        fclose(yyin);
        return 1;
    }

    printf("\n=================================================\n");
    printf("Análise sintática concluída com sucesso!\n");
    printf("=================================================\n\n");

    /* Construir tabela de símbolos (sintática) */
    printf("Construindo tabela de símbolos...\n");
    SymbolTable *symtab = create_symbol_table();
    build_symbol_table(syntax_tree, symtab);

    /* Imprimir a tabela de símbolos */
    printf("\n=================================================\n");
    printf("TABELA DE SÍMBOLOS\n");
    printf("=================================================\n");
    print_complete_symbol_table(symtab, syntax_tree);

    /* Imprimir a AST */
    printf("\n=================================================\n");
    printf("ÁRVORE SINTÁTICA ABSTRATA (AST)\n");
    printf("=================================================\n\n");

    if (syntax_tree)
    {
        print_ast(syntax_tree, 0);
    }
    else
    {
        printf("Nenhuma árvore gerada.\n");
    }

    /* ================= ANÁLISE SEMÂNTICA ================= */

    printf("\n=================================================\n");
    printf("ANÁLISE SEMÂNTICA\n");
    printf("=================================================\n");
    printf("Iniciando análise semântica...\n");

    SemanticAnalyzer *analyzer = semantic_create();
    int semantic_ok = semantic_analyze(analyzer, syntax_tree);

    if (!semantic_ok)
    {
        fprintf(stderr, "\nCompilação abortada devido a erros semânticos.\n");

        semantic_destroy(analyzer);
        free_ast(syntax_tree);
        free_symbol_table(symtab);
        fclose(yyin);
        return 1;
    }

    printf("\nAnálise semântica concluída com sucesso!\n");

    /* ================= CÓDIGO INTERMEDIÁRIO ================= */

    printf("\n=================================================\n");
    printf("GERAÇÃO DE CÓDIGO INTERMEDIÁRIO\n");
    printf("=================================================\n");
    printf("Gerando código intermediário...\n");

    CodeGenerator *codegen = codegen_create();
    codegen_generate(codegen, syntax_tree);

    printf("\n-------------------------------------------------\n");
    printf("CÓDIGO INTERMEDIÁRIO GERADO\n");
    printf("-------------------------------------------------\n");
    codegen_print(codegen);

    printf("\n=================================================\n");
    printf("Compilação concluída!\n");
    printf("=================================================\n");

    /* ================= LIMPEZA DE MEMÓRIA ================= */

    codegen_destroy(codegen);
    semantic_destroy(analyzer);

    if (syntax_tree)
    {
        free_ast(syntax_tree);
    }

    if (symtab)
    {
        free_symbol_table(symtab);
    }

    fclose(yyin);
    return 0;
}
