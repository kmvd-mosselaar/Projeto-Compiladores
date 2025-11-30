#include <stdlib.h>
#include "common.h"

NodeList* create_list() {
    NodeList *list = (NodeList*)malloc(sizeof(NodeList));
    list->capacity = 10;
    list->count = 0;
    list->nodes = (ASTNode**)malloc(sizeof(ASTNode*) * list->capacity);
    return list;
}

void add_to_list(NodeList *list, ASTNode *node) {
    if (list->count >= list->capacity) {
        list->capacity *= 2;
        list->nodes = (ASTNode**)realloc(list->nodes, sizeof(ASTNode*) * list->capacity);
    }
    list->nodes[list->count++] = node;
}
