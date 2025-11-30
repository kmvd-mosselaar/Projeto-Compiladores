# Projeto-Compiladores

## 1. Introdução

Este documento descreve a implementação de um compilador para a linguagem C- (C Minus), uma linguagem de programação simplificada baseada em C, desenvolvida para fins didáticos. O projeto implementa as fases iniciais de compilação: análise léxica, análise sintática, construção da tabela de símbolos, árvore sintática abstrata e analisador semântico.

A linguagem C- foi especificada por Kenneth C. Louden no livro "Compiler Construction: Principles and Practice".

## 2. Objetivos do Projeto

O projeto tem como objetivos principais:

• Scanner (analisador léxico);
• Tabela de Símbolos;
• Parser (analisador sintático) e construção da AST;
• Analisador semântico;
• Gerador de código intermediário em três endereços (sem otimizações).

## 3. Especificação da Linguagem C-

A linguagem C- possui as seguintes características:

- Dois tipos de dados: `int` e `void`
- Suporte a arrays unidimensionais
- Funções com parâmetros
- Estruturas de controle: `if-else` e `while`
- Operadores aritméticos: `+`, `-`, `*`, `/`
- Operadores relacionais: `<`, `<=`, `>`, `>=`, `==`, `!=`
- Comentários delimitados por `/* */`

## 4. Arquitetura do Compilador

### 4.1. Componentes Principais

O compilador é estruturado em módulos independentes:
```
compilador/
├── scanner.l          - Especificação léxica (Flex)
├── parser.y           - Especificação sintática (Bison)
├── ast.h / ast.c      - Árvore Sintática Abstrata
├── symtab.h / symtab.c - Tabela de Símbolos
├── build_symtab.c     - Construtor da Tabela de Símbolos
├── common.h / common.c - Funções auxiliares
├── main.c             - Programa principal
└── Makefile           - Script de compilação
```

### 4.2. Fluxo de Processamento
```
Código Fonte (.cm)
        ↓
   [Scanner]  ← Análise Léxica (Flex)
        ↓
     Tokens
        ↓
    [Parser]  ← Análise Sintática (Bison)
        ↓
       AST
        ↓
[Tabela de Símbolos]
        ↓
    Saída: AST + Símbolos
```

## 5. Análise Léxica

### 5.1. Descrição

O analisador léxico é implementado utilizando Flex e tem as seguintes responsabilidades:

- Reconhecer tokens válidos da linguagem
- Ignorar espaços em branco e comentários
- Detectar caracteres inválidos
- Manter registro do número da linha para mensagens de erro

### 5.2. Tokens Reconhecidos

| Categoria | Tokens |
|-----------|--------|
| Palavras-chave | `if`, `else`, `while`, `return`, `int`, `void` |
| Identificadores | Sequências de letras |
| Números | Sequências de dígitos |
| Operadores | `+`, `-`, `*`, `/`, `<`, `<=`, `>`, `>=`, `==`, `!=`, `=` |
| Delimitadores | `;`, `,`, `(`, `)`, `[`, `]`, `{`, `}` |
| Comentários | `/* ... */` |

### 5.3. Tratamento de Erros

Erros léxicos são reportados no formato:
```
ERRO LEXICO: 'caractere' - LINHA: n
```

## 6. Análise Sintática

### 6.1. Descrição

O analisador sintático é implementado utilizando Bison e realiza:

- Validação da estrutura gramatical do programa
- Construção da Árvore Sintática Abstrata (AST)
- Detecção de erros sintáticos

### 6.2. Gramática

A gramática da linguagem C- é especificada em notação BNF no arquivo `parser.y`. Principais regras:
```
programa → declaracao-lista
declaracao-lista → declaracao-lista declaracao | declaracao
declaracao → var-declaracao | fun-declaracao
var-declaracao → tipo-especificador ID ; | tipo-especificador ID [ NUM ] ;
fun-declaracao → tipo-especificador ID ( params ) compound-stmt
```

### 6.3. Tratamento de Erros

Erros sintáticos são reportados no formato:
```
ERRO SINTATICO: token inesperado 'token' - LINHA: n
```

## 7. Árvore Sintática Abstrata (AST)

### 7.1. Estrutura

A AST é implementada como uma estrutura de dados em árvore onde cada nó representa uma construção da linguagem:

- Declarações de variáveis e funções
- Comandos (atribuição, if, while, return)
- Expressões (binárias, variáveis, constantes, chamadas de função)

### 7.2. Tipos de Nós
```c
typedef enum {
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_FUN_DECL,
    NODE_PARAM,
    NODE_COMPOUND_STMT,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_RETURN_STMT,
    NODE_ASSIGN,
    NODE_BINARY_OP,
    NODE_VAR,
    NODE_CALL,
    NODE_CONST,
    NODE_EXPR_STMT
} NodeType;
```

### 7.3. Visualização

A AST é apresentada em formato hierárquico visual utilizando caracteres ASCII para representar a estrutura de árvore:
```
                    PROGRAM
                       |
                   FUN:soma
                /------+------\
          PARAM:a   PARAM:b   BLOCK
                                |
                             RETURN
                                |
                               (+)
                             /----\
                            a      b
```

## 8. Tabela de Símbolos

### 8.1. Estrutura

A tabela de símbolos mantém informações sobre todos os identificadores do programa:
```c
typedef struct Symbol {
    char *name;              // Nome do identificador
    DataType type;           // Tipo (int/void)
    SymbolKind kind;         // Categoria (variável/função/parâmetro)
    int is_array;            // Flag para arrays
    int array_size;          // Tamanho do array
    char *scope;             // Nome do escopo
    int line_num;            // Linha de declaração
    struct Symbol *next;     // Próximo símbolo
} Symbol;
```

### 8.2. Gerenciamento de Escopos

O sistema implementa uma pilha de escopos para gerenciar visibilidade de identificadores:

- Escopo global: Contém funções e variáveis globais
- Escopos locais: Um para cada função, contendo parâmetros e variáveis locais

### 8.3. Formato de Saída
```
Nome                 | Tipo     | Categoria    | Escopo          | Info            | Linha
--------------------------------------------------------------------------------------------
main                 | int      | funcao       | global          | -                | 8
soma                 | int      | funcao       | global          | -                | 3
resultado            | int      | variavel     | main            | -                | 10
```

## 9. Compilação e Execução

### 9.1. Requisitos

- GCC (GNU Compiler Collection)
- Flex 2.6 ou superior
- Bison 3.0 ou superior
- Make

### 9.2. Instalação das Dependências

#### Linux (Ubuntu/Debian)
```bash
sudo apt-get update
sudo apt-get install build-essential flex bison
```

#### Alpine Linux
```bash
apk update
apk add build-base flex bison gcc g++ make
```

### 9.3. Compilação do Projeto
```bash
# Gerar código do Bison
bison -d parser.y

# Gerar código do Flex
flex scanner.l

# Compilar todos os arquivos
gcc -Wall -g -o cminusc lex.yy.c parser.tab.c ast.c common.c symtab.c build_symtab.c main.c
```

### 9.4. Teste
```bash
./cminusc <arquivo_teste.cm>
```

Onde `<arquivo.cm>` é o arquivo de código-fonte C-.

### 9.5. Saída do Compilador

O compilador produz três componentes na saída:

1. **Cabeçalho**: Informações sobre o arquivo sendo compilado
2. **Tabela de Símbolos**: Lista completa de identificadores
3. **AST**: Representação visual da estrutura do programa

## 10. Estrutura do Código

### 10.1. Modularização

O código é organizado em módulos independentes:

- **scanner.l**: Especificação léxica em Flex
- **parser.y**: Especificação sintática em Bison
- **ast.c/h**: Implementação da AST
- **symtab.c/h**: Implementação da tabela de símbolos
- **build_symtab.c**: Construção da tabela a partir da AST
- **common.c/h**: Estruturas e funções auxiliares
- **main.c**: Coordenação dos componentes

### 10.2. Convenções de Código

- Nomes de funções em snake_case
- Nomes de tipos em PascalCase
- Constantes em UPPER_CASE
- Indentação de 4 espaços

## 11. Referências

### 11.1. Bibliografia

LOUDEN, Kenneth C. **Compiler Construction: Principles and Practice**. Boston: PWS Publishing Company, 1997. 707 p.

AHO, Alfred V.; LAM, Monica S.; SETHI, Ravi; ULLMAN, Jeffrey D. **Compilers: Principles, Techniques, and Tools**. 2. ed. Boston: Addison-Wesley, 2006.

### 11.2. Ferramentas

- Flex: The Fast Lexical Analyzer. Disponível em: https://github.com/westes/flex
- GNU Bison: The YACC-compatible Parser Generator. Disponível em: https://www.gnu.org/software/bison/

## Informações do Projeto

**Disciplina**: Compiladores  
**Instituição**: UNIFESP - Universidade Federal de São Paulo  
**Campus**: São José dos Campos  
**Ano**: 2025
