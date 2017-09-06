#ifndef SCHC_AST_H_
#define SCHC_AST_H_

#include "vector.h"

#include <stdio.h>
#include <sys/types.h>

typedef enum ast_rule_ {
    AST_NO_RULE = 0,
    AST_EXP_HAS_TYPE,
    AST_FN_APPL,
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
    AST_MODULE_DECL,
    AST_TYPE_DECL,
    AST_DATA_DECL,
    AST_NEWTYPE_DECL,
    AST_CLASS_DECL,
    AST_INSTANCE_DECL,
    AST_DEFAULT_DECL,
    AST_FOREING_DECL,
    AST_HAS_TYPE_DECL,
    AST_HAS_FIXITY_DECL,
    AST_FN_DECL,
    AST_VAL_DECL,
    AST_MODULE,
} ast_rule_t;

struct ast_;
typedef struct ast_ ast_t;

void ast_print(const ast_t *ast, FILE *fp);

typedef struct ast_fn_appl_ {
    ast_t *fn;
    vector_t/*ast_t**/ args;
} ast_fn_appl_t;

typedef struct ast_if_ {
    ast_t *cond;
    ast_t *then_branch;
    ast_t *else_branch;
} ast_if_t;

typedef struct ast_binding_ {
    char *symbol;
    ast_t *value;
} ast_binding_t;

typedef struct ast_let_ {
    vector_t/*ast_binding_t*/ bindings;
    ast_t *body;
} ast_let_t;

typedef struct ast_var_ {
    char *name;
} ast_var_t;

typedef struct ast_lit_ {
    union {
        int int_lit;
    };
} ast_lit_t;

typedef struct ast_fn_decl_ {
    char *name;
    vector_t/*char**/ vars;
    ast_t *body;
} ast_fn_decl_t;

typedef struct ast_var_decl_ {
    char *name;
    ast_t *body;
} ast_val_decl_t;

typedef struct ast_module_ {
    char *name;
    vector_t/*ast_t**/ decls;
} ast_module_t;

struct ast_ {
    ast_rule_t rule;
    union {
        ast_fn_appl_t fn_appl;
        ast_if_t if_exp;
        ast_let_t let;
        ast_var_t var;
        ast_lit_t lit;
        ast_fn_decl_t fn_decl;
        ast_val_decl_t val_decl;
        ast_module_t module;
    };
};

#endif/*SCHC_AST_H_*/