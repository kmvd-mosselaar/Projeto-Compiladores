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
├── semantic.h / semantic.c - Analisador Semântico
├── codegen.h / codegen.c - Gerador de Código Intermediário
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
[Análise Semântica]
        ↓
[Geração de Código]
        ↓
  Código Intermediário
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

## 9. Análise Semântica

### 9.1. Descrição

O analisador semântico realiza verificações de contexto sobre o programa que não podem ser expressas pela gramática livre de contexto. É implementado em `semantic.c/h` e opera sobre a AST construída pelo parser.

### 9.2. Verificações Realizadas

O analisador semântico verifica:

#### 9.2.1. Declarações
- Variáveis não podem ter tipo `void`
- Parâmetros não podem ter tipo `void`
- Identificadores não podem ser redeclarados no mesmo escopo
- A função `main` deve existir no programa

#### 9.2.2. Uso de Identificadores
- Identificadores devem ser declarados antes do uso
- Variáveis não podem ser usadas como funções
- Funções não podem ser usadas como variáveis

#### 9.2.3. Arrays
- Acesso a array deve incluir índice (exceto quando passado como argumento)
- Variáveis escalares não podem ser indexadas
- Arrays não podem ser usados em operações sem índice

#### 9.2.4. Funções
- Funções com tipo de retorno não-void devem ter pelo menos um comando `return`
- Funções `void` não podem retornar valor
- Funções não-void devem retornar valor

#### 9.2.5. Expressões
- Operações aritméticas e relacionais devem operar sobre valores válidos
- Atribuições devem respeitar compatibilidade de tipos

### 9.3. Funções Built-in

O compilador reconhece duas funções pré-definidas:

- `input()`: Lê um valor inteiro da entrada padrão
- `output(valor)`: Escreve um valor na saída padrão

### 9.4. Tratamento de Erros

Erros semânticos são reportados no formato:
```
ERRO SEMANTICO: descrição do erro - LINHA: n
```

Exemplos de erros detectados:
```
ERRO SEMANTICO: identificador 'x' nao declarado - LINHA: 5
ERRO SEMANTICO: funcao 'calcula' deve retornar um valor - LINHA: 3
ERRO SEMANTICO: array 'v' deve ser acessado com indice - LINHA: 12
ERRO SEMANTICO: 'soma' nao e um array - LINHA: 8
```

## 10. Geração de Código Intermediário

### 10.1. Descrição

O gerador de código intermediário produz código em representação de três endereços a partir da AST validada. A implementação está em `codegen.c/h`.

### 10.2. Formato do Código Intermediário

O código gerado utiliza a seguinte notação:

#### 10.2.1. Declarações
```
declare nome_variavel
declare_array nome_array[tamanho]
```

#### 10.2.2. Funções
```
function nome_funcao:
  param parametro1
  param parametro2
  [corpo da função]
end_function nome_funcao
```

#### 10.2.3. Atribuições e Operações
```
variavel = expressao
temporario = op1 operador op2
temporario = array[indice]
array[indice] = valor
```

#### 10.2.4. Estruturas de Controle

**Comandos if-else:**
```
  if_false condicao goto L1
  [then_stmt]
  goto L2
L1:
  [else_stmt]
L2:
```

**Comandos while:**
```
L1:
  if_false condicao goto L2
  [corpo do while]
  goto L1
L2:
```

#### 10.2.5. Chamadas de Função
```
push argumento1
push argumento2
temporario = call nome_funcao
```

#### 10.2.6. Comandos return
```
return valor
return
```

#### 10.2.7. Funções Built-in
```
temporario = input
output valor
```

### 10.3. Variáveis Temporárias e Labels

O gerador utiliza:
- Temporários: `t0`, `t1`, `t2`, ... para armazenar resultados intermediários
- Labels: `L0`, `L1`, `L2`, ... para controle de fluxo

### 10.4. Exemplo de Código Gerado

Código C-:
```c
int fatorial(int n) {
    int resultado;
    if (n <= 1) {
        resultado = 1;
    } else {
        resultado = n * fatorial(n - 1);
    }
    return resultado;
}
```

Código intermediário gerado:
```
function fatorial:
  param n
  declare resultado
  t0 = n <= 1
  if_false t0 goto L0
  resultado = 1
  goto L1
L0:
  t1 = n - 1
  push t1
  t2 = call fatorial
  t3 = n * t2
  resultado = t3
L1:
  return resultado
end_function fatorial
```

## 11. Compilação e Execução

### 11.1. Requisitos

- GCC (GNU Compiler Collection)
- Flex 2.6 ou superior
- Bison 3.0 ou superior
- Make

### 11.2. Instalação das Dependências

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

### 11.3. Compilação do Projeto
```bash
# Gerar código do Bison
bison -d parser.y

# Gerar código do Flex
flex scanner.l

# Compilar todos os arquivos
gcc -Wall -g -o cminusc lex.yy.c parser.tab.c ast.c common.c symtab.c build_symtab.c semantic.c codegen.c main.c
```

### 11.4. Teste
```bash
./cminusc <arquivo_teste.cm>
```

Onde `<arquivo.cm>` é o arquivo de código-fonte C-.

### 11.5. Saída do Compilador

O compilador produz os seguintes componentes na saída:

1. **Cabeçalho**: Informações sobre o arquivo sendo compilado
2. **Tabela de Símbolos**: Lista completa de identificadores com seus atributos
3. **AST**: Representação visual da estrutura do programa
4. **Análise Semântica**: Mensagens de erro semântico (se houver)
5. **Código Intermediário**: Código em representação de três endereços

### 11.6. Exemplo de Execução Completa

Arquivo `teste.cm`:
```c
int soma(int a, int b) {
    return a + b;
}

int main(void) {
    int x;
    int y;
    x = 5;
    y = 3;
    return soma(x, y);
}
```

Saída:
```
========================================
COMPILADOR C-
Arquivo: teste.cm
========================================

========== TABELA DE SIMBOLOS ==========
Nome                 | Tipo     | Categoria    | Escopo          | Info            | Linha
--------------------------------------------------------------------------------------------
soma                 | int      | funcao       | global          | -                | 1
a                    | int      | parametro    | soma            | -                | 1
b                    | int      | parametro    | soma            | -                | 1
main                 | int      | funcao       | global          | -                | 5
x                    | int      | variavel     | main            | -                | 6
y                    | int      | variavel     | main            | -                | 7
==========================================

========== ARVORE SINTATICA ABSTRATA ==========
[Visualização da AST]
==========================================

========== CODIGO INTERMEDIARIO ==========
# Codigo Intermediario C-

function soma:
  param a
  param b
  t0 = a + b
  return t0
end_function soma

function main:
  declare x
  declare y
  x = 5
  y = 3
  push x
  push y
  t1 = call soma
  return t1
end_function main
==========================================
```

## 12. Estrutura do Código

### 12.1. Modularização

O código é organizado em módulos independentes:

- **scanner.l**: Especificação léxica em Flex
- **parser.y**: Especificação sintática em Bison
- **ast.c/h**: Implementação da AST
- **symtab.c/h**: Implementação da tabela de símbolos
- **build_symtab.c**: Construção da tabela a partir da AST
- **semantic.c/h**: Análise semântica sobre a AST
- **codegen.c/h**: Geração de código intermediário
- **common.c/h**: Estruturas e funções auxiliares
- **main.c**: Coordenação dos componentes

### 12.2. Convenções de Código

- Nomes de funções em snake_case
- Nomes de tipos em PascalCase
- Constantes em UPPER_CASE
- Indentação de 4 espaços

## 13. Testes Realizados

### 13.1. Descrição dos Testes

O compilador foi testado com diversos programas que cobrem diferentes aspectos da linguagem C-. Os testes foram divididos em duas categorias principais: **testes de funcionalidades corretas** e **testes de detecção de erros**.

### 13.2. Testes de Funcionalidades

Foram realizados testes para verificar o correto funcionamento das seguintes funcionalidades:

- **Chamadas de função**: Programas com múltiplas funções e passagem de parâmetros
- **Arrays**: Declaração, indexação e passagem como argumento para funções
- **Estruturas de controle**: Comandos `if-else` e `while`, incluindo aninhamento
- **Operações aritméticas**: Expressões complexas envolvendo múltiplas operações
- **Operações relacionais**: Comparações em condicionais
- **Variáveis locais**: Declaração e uso de variáveis em diferentes escopos
- **Recursão e iteração**: Algoritmos como busca em array e cálculo de Fibonacci

### 13.3. Testes de Detecção de Erros

O compilador foi testado para garantir a correta detecção dos seguintes erros:

#### 13.3.1. Erros Léxicos
- Caracteres inválidos no código fonte

#### 13.3.2. Erros Sintáticos
- Falta de ponto-e-vírgula
- Estruturas gramaticais malformadas

#### 13.3.3. Erros Semânticos
- Uso de variáveis não declaradas
- Funções sem comando `return` quando obrigatório
- Declaração de variáveis com tipo `void`
- Uso de arrays sem índice em contextos inválidos
- Chamada de funções não declaradas
- Funções `void` retornando valores
- Redeclaração de identificadores no mesmo escopo

### 13.4. Resultados

Todos os testes foram executados com sucesso:

- Programas corretos geraram AST, tabela de símbolos e código intermediário sem erros
- Programas com erros foram corretamente rejeitados com mensagens apropriadas
- O sistema de escopos funcionou corretamente em todos os cenários testados
- A geração de código intermediário produziu representações corretas para todas as construções da linguagem

## 14. Referências

### 14.1. Bibliografia

LOUDEN, Kenneth C. **Compiler Construction: Principles and Practice**. Boston: PWS Publishing Company, 1997. 707 p.

AHO, Alfred V.; LAM, Monica S.; SETHI, Ravi; ULLMAN, Jeffrey D. **Compilers: Principles, Techniques, and Tools**. 2. ed. Boston: Addison-Wesley, 2006.

### 14.2. Ferramentas

- Flex: The Fast Lexical Analyzer. Disponível em: https://github.com/westes/flex
- GNU Bison: The YACC-compatible Parser Generator. Disponível em: https://www.gnu.org/software/bison/

## Informações do Projeto

**Disciplina**: Compiladores  
**Instituição**: UNIFESP - Universidade Federal de São Paulo  
**Campus**: São José dos Campos  
**Ano**: 2025