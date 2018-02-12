#ifndef SCHC_AST_H_
#define SCHC_AST_H_

#include "vector.h"

#include <stdio.h>
#include <sys/types.h>

typedef enum ast_rule_ {
    AST_NO_RULE = 0,
    AST_EXP_HAS_TYPE,
    AST_NEG,
    AST_FN_APPL,
    AST_OP_APPL,
    AST_LAMBDA,
    AST_LET,
    AST_IF,
    AST_CASE,
    AST_DO,
    AST_VAR,
    AST_CON,
    AST_LIT,
    AST_TUPLE,
    AST_CONS,
    AST_ARITH_SEQ,
    AST_TYPE_DECL,
    AST_DATA_DECL,
    AST_NEWTYPE_DECL,
    AST_CLASS_DECL,
    AST_INSTANCE_DECL,
    AST_DEFAULT_DECL,
    AST_FOREING_DECL,
    AST_HAS_TYPE_DECL,
    AST_FIXITY_DECL,
    AST_FN_DECL,
    AST_VAL_DECL,
    AST_MODULE,
    AST_BODY,
} ast_rule_t;

struct ast_;
typedef struct ast_ ast_t;

void ast_print(const ast_t *node, FILE *fp);
void ast_destroy(ast_t *node);

typedef struct ast_module_ {
    char *modid;
    vector_t/*ast_export_t*/ exports;
    ast_t *body;
} ast_module_t;

typedef struct ast_export_ {
    char *exportid;
} ast_export_t;

typedef struct ast_body_ {
    vector_t/*ast_t**/ impdecls;
    vector_t/*ast_t**/ topdecls;
} ast_body_t;

typedef struct ast_neg_ {
    ast_t *expr;
} ast_neg_t;

typedef struct ast_fn_appl_ {
    ast_t *fn;
    ast_t *arg;
} ast_fn_appl_t;

typedef struct ast_op_appl_ {
    char *op_name;
    ast_t *lhs;
    ast_t *rhs;
} ast_op_appl_t;

typedef struct ast_if_ {
    ast_t *cond;
    ast_t *then_branch;
    ast_t *else_branch;
} ast_if_t;

typedef struct ast_do_ {
    vector_t/*ast_t*/ steps;
} ast_do_t;

typedef struct ast_let_ {
    vector_t/*ast_t*/ bindings;
    ast_t *body;
} ast_let_t;

typedef struct ast_var_ {
    char *name;
} ast_var_t;

typedef struct ast_con_ {
    char *name;
} ast_con_t;

typedef struct ast_lit_ {
    union {
        int int_lit;
    };
} ast_lit_t;

typedef struct ast_fixity_decl_ {
    char associativity;
    int fixity;
    char *op;
} ast_fixity_decl_t;

typedef struct ast_fn_decl_ {
    char *name;
    vector_t/*char**/ vars;
    ast_t *body;
} ast_fn_decl_t;

typedef struct ast_val_decl_ {
    char *name;
    ast_t *body;
} ast_val_decl_t;

typedef struct ast_has_type_decl_ {
    char *symbol_name;
    ast_t *type_exp;
} ast_has_type_decl_t;

struct ast_ {
    ast_rule_t rule;
    union {
        ast_module_t module;
        ast_body_t body;
        ast_neg_t neg;
        ast_fn_appl_t fn_appl;
        ast_op_appl_t op_appl;
        ast_if_t if_exp;
        ast_do_t do_exp;
        ast_let_t let;
        ast_var_t var;
        ast_con_t con;
        ast_lit_t lit;
        ast_fixity_decl_t fixity_decl;
        ast_fn_decl_t fn_decl;
        ast_val_decl_t val_decl;
        ast_has_type_decl_t has_type_decl;
    };
};

#endif/*SCHC_AST_H_*/