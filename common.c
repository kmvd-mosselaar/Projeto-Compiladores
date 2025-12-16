#include <stdlib.h>
#include "common.h"

int line_num = 1;
int verbose_mode = 0;

void set_verbose(int mode)
{
    verbose_mode = mode;
}

void log_verbose(const char *phase, const char *msg)
{
    if (verbose_mode)
    {
        printf("[%s] %s\n", phase, msg);
    }
}

NodeList *create_list()
{
    NodeList *list = (NodeList *)malloc(sizeof(NodeList));
    list->capacity = 10;
    list->count = 0;
    list->nodes = (ASTNode **)malloc(sizeof(ASTNode *) * list->capacity);
    return list;
}

void add_to_list(NodeList *list, ASTNode *node)
{
    if (list->count >= list->capacity)
    {
        list->capacity *= 2;
        list->nodes = (ASTNode **)realloc(list->nodes, sizeof(ASTNode *) * list->capacity);
    }
    list->nodes[list->count++] = node;
}
