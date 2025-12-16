#ifndef AST_H
#define AST_H

/* Tipos de nós da AST */
typedef enum {
    /* Declarações */
    NODE_PROGRAM,
    NODE_VAR_DECL,
    NODE_FUN_DECL,
    NODE_PARAM,
    NODE_PARAM_LIST,
    
    /* Statements */
    NODE_COMPOUND_STMT,
    NODE_EXPR_STMT,
    NODE_IF_STMT,
    NODE_WHILE_STMT,
    NODE_RETURN_STMT,
    
    /* Expressões */
    NODE_ASSIGN,
    NODE_BINARY_OP,
    NODE_VAR,
    NODE_VAR_ARRAY,
    NODE_CALL,
    NODE_CONST,
    
    /* Lista de argumentos */
    NODE_ARG_LIST
} NodeType;

/* Tipos de operadores */
typedef enum {
    OP_ADD,     // +
    OP_SUB,     // -
    OP_MUL,     // *
    OP_DIV,     // /
    OP_LT,      // <
    OP_LE,      // <=
    OP_GT,      // >
    OP_GE,      // >=
    OP_EQ,      // ==
    OP_NE       // !=
} OpType;

/* Tipos de dados */
typedef enum {
    TYPE_VOID,
    TYPE_INT
} DataType;

/* Estrutura do nó da AST */
typedef struct ASTNode {
    NodeType type;
    int line_num;
    
    union {
        /* Declaração de variável */
        struct {
            DataType data_type;
            char *name;
            int is_array;
            int array_size;
        } var_decl;
        
        /* Declaração de função */
        struct {
            DataType return_type;
            char *name;
            struct ASTNode *params;
            struct ASTNode *body;
        } fun_decl;
        
        /* Parâmetro */
        struct {
            DataType data_type;
            char *name;
            int is_array;
        } param;
        
        /* Statement composto */
        struct {
            struct ASTNode **local_decls;
            int num_local_decls;
            struct ASTNode **statements;
            int num_statements;
        } compound;
        
        /* If statement */
        struct {
            struct ASTNode *condition;
            struct ASTNode *then_stmt;
            struct ASTNode *else_stmt;
        } if_stmt;
        
        /* While statement */
        struct {
            struct ASTNode *condition;
            struct ASTNode *body;
        } while_stmt;
        
        /* Return statement */
        struct {
            struct ASTNode *expr;
        } return_stmt;
        
        /* Atribuição */
        struct {
            struct ASTNode *var;
            struct ASTNode *expr;
        } assign;
        
        /* Operação binária */
        struct {
            OpType op;
            struct ASTNode *left;
            struct ASTNode *right;
        } binary_op;
        
        /* Variável */
        struct {
            char *name;
            struct ASTNode *index;  // NULL se não for array
        } var;
        
        /* Chamada de função */
        struct {
            char *name;
            struct ASTNode **args;
            int num_args;
        } call;
        
        /* Constante */
        struct {
            int value;
        } const_val;
        
        /* Expressão statement */
        struct {
            struct ASTNode *expr;
        } expr_stmt;
    } data;
    
    /* Ponteiros para irmãos (lista encadeada) */
    struct ASTNode *sibling;
} ASTNode;

/* Funções para criar nós da AST */
ASTNode* create_program_node(ASTNode *decl_list);
ASTNode* create_var_decl_node(DataType type, char *name, int is_array, int array_size, int line);
ASTNode* create_fun_decl_node(DataType return_type, char *name, ASTNode *params, ASTNode *body, int line);
ASTNode* create_param_node(DataType type, char *name, int is_array, int line);
ASTNode* create_compound_stmt_node(ASTNode **local_decls, int num_local_decls, 
                                    ASTNode **statements, int num_statements, int line);
ASTNode* create_if_stmt_node(ASTNode *condition, ASTNode *then_stmt, ASTNode *else_stmt, int line);
ASTNode* create_while_stmt_node(ASTNode *condition, ASTNode *body, int line);
ASTNode* create_return_stmt_node(ASTNode *expr, int line);
ASTNode* create_assign_node(ASTNode *var, ASTNode *expr, int line);
ASTNode* create_binary_op_node(OpType op, ASTNode *left, ASTNode *right, int line);
ASTNode* create_var_node(char *name, ASTNode *index, int line);
ASTNode* create_call_node(char *name, ASTNode **args, int num_args, int line);
ASTNode* create_const_node(int value, int line);
ASTNode* create_expr_stmt_node(ASTNode *expr, int line);

/* Funções auxiliares */
void print_ast(ASTNode *node, int indent);
void free_ast(ASTNode *node);
char* get_op_string(OpType op);
char* get_type_string(DataType type);

#endif
