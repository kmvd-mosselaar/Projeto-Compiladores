#ifndef COMMON_H
#define COMMON_H

#include "ast.h"

/* Lista dinâmica auxiliar para construção de listas */
typedef struct {
    ASTNode **nodes;
    int count;
    int capacity;
} NodeList;

/* Funções para manipulação de NodeList */
NodeList* create_list();
void add_to_list(NodeList *list, ASTNode *node);

#endif
