// Reconhecimento preditivo dirigido  por tabela
// entrada: uma cadeia w e uma tabela M
// saida: a derivacao mais a esquerda de w ou erro
// inicialmente o buffer contem w$ ($ simbolo de fim de cadeia)
// e a pilha contem o simbolo inicial G acima de $

/*
define i para que aponte para o primeiro simbolo de w
define X como o simbolo no topo da pilha
enquanto X != $ : se X for o primeiro caracter da cadeia w
*/

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <string.h>

using namespace std;

#pragma region ANALISADOR LEXICO

typedef enum
{
    ID,
    PLUS,
    MULT,
    AP,
    FP,
    FIMDAPILHA,
    ERRO
} Terminal;

typedef struct
{
    int indice_cadeia;
    Terminal token;
    char conteudo[250];
} Lexema;

const char *Terminal2Str(Terminal t)
{
    switch (t)
    {
    case ID:
        return "ID";
    case PLUS:
        return "PLUS(+)";
    case MULT:
        return "MULT(*)";
    case AP:
        return "AP(()";
    case FP:
        return "FP())";
    case FIMDAPILHA:
        return "$";
    case ERRO:
        return "ERRO";
    default:
        return "TOKEN INVALIDO";
    }
}

// Autômato simples: reconhece IDs [a-zA-Z_][a-zA-Z0-9_]* e + * ( )
int get_next_token(char cadeia[], int indice_cadeia, Lexema *lex)
{
    int i = indice_cadeia;

    // pular espaços
    while (cadeia[i] == ' ' || cadeia[i] == '\t' ||
           cadeia[i] == '\n' || cadeia[i] == '\r')
    {
        i++;
    }

    char c = cadeia[i];
    lex->indice_cadeia = i;

    if (c == '\0')
    {
        lex->token = FIMDAPILHA;
        strcpy_s(lex->conteudo, "$");
        return i;
    }

    // ID
    if (isalpha((unsigned char)c) || c == '_')
    {
        int j = 0;
        while (isalpha((unsigned char)cadeia[i]) ||
               isdigit((unsigned char)cadeia[i]) ||
               cadeia[i] == '_')
        {
            lex->conteudo[j++] = cadeia[i++];
        }
        lex->conteudo[j] = '\0';
        lex->token = ID;
        return i;
    }

    // símbolos especiais
    switch (c)
    {
    case '+':
        lex->token = PLUS;
        lex->conteudo[0] = c;
        lex->conteudo[1] = '\0';
        i++;
        return i;
    case '*':
        lex->token = MULT;
        lex->conteudo[0] = c;
        lex->conteudo[1] = '\0';
        i++;
        return i;
    case '(':
        lex->token = AP;
        lex->conteudo[0] = c;
        lex->conteudo[1] = '\0';
        i++;
        return i;
    case ')':
        lex->token = FP;
        lex->conteudo[0] = c;
        lex->conteudo[1] = '\0';
        i++;
        return i;
    default:
        lex->token = ERRO;
        lex->conteudo[0] = c;
        lex->conteudo[1] = '\0';
        return i; // não avança: erro no mesmo caractere
    }
}

#pragma endregion

#pragma region FERRAMENTAS DO PARSER

void avanca(char cadeia[], int *indice_cadeia, Lexema *lex)
{
    *indice_cadeia = get_next_token(cadeia, *indice_cadeia, lex);
}

Terminal espia(Lexema *lex)
{
    return lex->token;
}

bool expect(Terminal esperado, char cadeia[], int *indice_cadeia, Lexema *lex)
{
    if (lex->token == esperado)
    {
        avanca(cadeia, indice_cadeia, lex);
        return true;
    }
    fprintf(stderr, "ERRO: esperado <%s>, recebido <%s>\n",
            Terminal2Str(esperado), Terminal2Str(lex->token));
    return false;
}

#pragma endregion

#pragma region PROTOTIPOS DA GRAMATICA

// expr → term plus
// plus → + term plus | ε
// term → fact mult
// mult → * fact mult | ε
// fact → (expr) | id

bool expr_(char[], int *, Lexema *);
bool plus_(char[], int *, Lexema *);
bool term(char[], int *, Lexema *);
bool mult(char[], int *, Lexema *);
bool fact(char[], int *, Lexema *);

#pragma endregion

#pragma region IMPLEMENTACAO DO PARSER

bool expr_(char cadeia[], int *indice_cadeia, Lexema *lex)
{
    // FIRST(expr) = { ID, AP }
    if (!term(cadeia, indice_cadeia, lex))
    {
        fprintf(stderr, "ERRO em expr: esperado term, recebido <%s>\n",
                Terminal2Str(espia(lex)));
        return false;
    }
    if (!plus_(cadeia, indice_cadeia, lex))
    {
        fprintf(stderr, "ERRO em expr: erro ao processar plus\n");
        return false;
    }
    return true;
}

// plus → + term plus | ε
bool plus_(char cadeia[], int *indice_cadeia, Lexema *lex)
{
    switch (espia(lex))
    {
    case PLUS:
        if (!expect(PLUS, cadeia, indice_cadeia, lex))
            return false;
        if (!term(cadeia, indice_cadeia, lex))
        {
            fprintf(stderr, "ERRO em plus: esperado term apos +\n");
            return false;
        }
        return plus_(cadeia, indice_cadeia, lex);
        // FOLLOW(plus) = { FP, FIMDAPILHA }
    case FP:
    case FIMDAPILHA:
        // produção ε
        return true;
    default:
        fprintf(stderr, "ERRO em plus: esperado '+' ou ε, recebido <%s>\n",
                Terminal2Str(espia(lex)));
        return false;
    }
}

// term → fact mult
bool term(char cadeia[], int *indice_cadeia, Lexema *lex)
{
    if (!fact(cadeia, indice_cadeia, lex))
    {
        fprintf(stderr, "ERRO em term: esperado fact, recebido <%s>\n",
                Terminal2Str(espia(lex)));
        return false;
    }
    if (!mult(cadeia, indice_cadeia, lex))
    {
        fprintf(stderr, "ERRO em term: erro ao processar mult\n");
        return false;
    }
    return true;
}

// mult → * fact mult | ε
bool mult(char cadeia[], int *indice_cadeia, Lexema *lex)
{
    switch (espia(lex))
    {
    case MULT:
        if (!expect(MULT, cadeia, indice_cadeia, lex))
            return false;
        if (!fact(cadeia, indice_cadeia, lex))
        {
            fprintf(stderr, "ERRO em mult: esperado fact apos *\n");
            return false;
        }
        return mult(cadeia, indice_cadeia, lex);
        // FOLLOW(mult) = { PLUS, FP, FIMDAPILHA }
    case PLUS:
    case FP:
    case FIMDAPILHA:
        // produção ε
        return true;
    default:
        fprintf(stderr, "ERRO em mult: esperado '*' ou ε, recebido <%s>\n",
                Terminal2Str(espia(lex)));
        return false;
    }
}

// fact → (expr) | id
bool fact(char cadeia[], int *indice_cadeia, Lexema *lex)
{
    switch (espia(lex))
    {
    case AP: // '(' expr ')'
        if (!expect(AP, cadeia, indice_cadeia, lex))
            return false;
        if (!expr_(cadeia, indice_cadeia, lex))
        {
            fprintf(stderr, "ERRO em fact: erro em expr dentro de '()'\n");
            return false;
        }
        if (!expect(FP, cadeia, indice_cadeia, lex))
        {
            fprintf(stderr, "ERRO em fact: esperado ')'\n");
            return false;
        }
        return true;

    case ID:
        if (!expect(ID, cadeia, indice_cadeia, lex))
            return false;
        return true;

    default:
        fprintf(stderr, "ERRO em fact: esperado '(' ou ID, recebido <%s>\n",
                Terminal2Str(espia(lex)));
        return false;
    }
}

#pragma endregion

int main()
{
    char programa[1000];

    // alguns testes possíveis:
    // "a + b * c"
    // "(a + b) * c"
    // "a * b + c * (d + e)"
    // erro: "a + * b", "a ** b", "+ a"
    strcpy_s(programa, "a * b + c * (d + e)");

    Lexema lex;
    int i = 0;

    avanca(programa, &i, &lex); // lê primeiro token

    bool ok = expr_(programa, &i, &lex) && (espia(&lex) == FIMDAPILHA);

    if (ok)
        printf("\nACEITA\n");
    else
        printf("\nREJEITA\n");

    return 0;
}
