#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "firth.h"
#include "assembler.h"

//======================================================
// Tokenization
//======================================================

firth_tokens *firth_tokenize(char *firth_src) {
    char *src = calloc(strlen(firth_src) + 1, sizeof(char));
    strcat(src, firth_src);
    char *str = strtok(src, " \n");
    firth_tokens *tokens = calloc(1, sizeof(firth_tokens));
    tokens->original_src = src;
    while (str != NULL) {
        firth_token *token = calloc(1, sizeof(firth_token));
        token->value = str;
        if (tokens->start == NULL) {
            tokens->start = token;
        }
        if (tokens->current != NULL) {
            tokens->current->next = token;
        }
        tokens->current = token;
        str = strtok(NULL, " \n");
    }
    tokens->current = tokens->start;
    return tokens;
}

int firth_match_token(char *str, firth_tokens *tokens) {
    return tokens->current && strcmp(str, tokens->current->value) == 0;
}

int firth_has_more_tokens(firth_tokens *tokens) {
    return tokens->current != NULL;
}

int firth_token_ends_with(firth_token * token, char *suffix)
{
    if(strlen(token->value) >= strlen(suffix))
    {
        if(!strcmp(token->value + strlen(token->value) - strlen(suffix), suffix))
        {
            return 1;
        }
    }
    return 0;
}

firth_token *firth_take_token(firth_tokens *tokens) {
    struct firth_token *current = tokens->current;
    if (current != NULL) {
        tokens->current = current->next;
    }
    return current;
}

void firth_report_error(char *msg, firth_tokens *tokens, firth_compilation_result *result) {
    result->error = msg;
    tokens->current = NULL;
}

//======================================================
// Parsing
//======================================================

// declarations
firth_parse_element *firth_parse_elt(firth_tokens *tokens, firth_compilation_result *result);

// implementations

void firth_delete_exprs(firth_parse_elements *elts);

firth_parse_element *firth_make_elt(firth_token *token, firth_parse_element_type type){
    firth_parse_element *elt = calloc(1, sizeof(firth_parse_element));
    elt->token = token;
    elt->type = type;
    return elt;
}

void firth_add_element(firth_parse_elements *elements, firth_parse_element *elt) {
    if (elements->first == NULL) {
        elements->first = elt;
        elements->last = elt;
    } else {
        elements->last->next_sibling = elt;
        elements->last = elt;
    }
}

firth_parse_element *firth_parse_num(firth_tokens *tokens, firth_compilation_result *result) {
    if (asm_is_num(tokens->current->value)) {
        return firth_make_elt(firth_take_token(tokens), NUMBER);
    }
    return NULL;
}

firth_parse_element *firth_parse_def(firth_tokens *tokens, firth_compilation_result *result) {
    if (firth_match_token("def", tokens)) {
        firth_parse_element *fun = firth_make_elt(firth_take_token(tokens), DEF);

        firth_token *function_name = firth_take_token(tokens);
        if (!firth_token_ends_with(function_name, "()")) {
            firth_report_error("function must end with parens", tokens, result);
            return fun;
        }
        fun->name = function_name;

        fun->left_children = calloc(1, sizeof(firth_parse_elements));

        while (firth_has_more_tokens(tokens) &&
               !firth_match_token("end", tokens)) {
            firth_add_element(fun->left_children, firth_parse_elt(tokens, result));
        }
        if (firth_match_token("end", tokens)) {
            firth_take_token(tokens);
        } else {
            firth_report_error("Expected end for fun? statement", tokens, result);
        }
        return fun;
    }
    return NULL;
}

firth_parse_element *firth_parse_call(firth_tokens *tokens, firth_compilation_result *result) {
    if (tokens->current && firth_token_ends_with(tokens->current, "()")) {
        firth_parse_element *call = firth_make_elt(firth_take_token(tokens), CALL);
        return call;
    }
    return NULL;
}

firth_parse_element *firth_parse_op(firth_tokens *tokens, firth_compilation_result *result) {
    if (firth_match_token("+", tokens) ||
        firth_match_token("-", tokens) ||
        firth_match_token("*",tokens) ||
        firth_match_token("/",tokens) ||
        firth_match_token("max",tokens) ||
        firth_match_token("min",tokens) ||
        firth_match_token("get", tokens) ||
        firth_match_token("dup", tokens) ||
        firth_match_token("pop", tokens) ||
        firth_match_token("swap", tokens) ||
        firth_match_token("return", tokens) ||
        firth_match_token(".", tokens)) {
        return firth_make_elt(firth_take_token(tokens), OP);
    }
    return NULL;
}

firth_parse_element *firth_parse_zero(firth_tokens *tokens, firth_compilation_result *result) {
    if (firth_match_token("zero?", tokens)) {
        firth_parse_element *zero = firth_make_elt(firth_take_token(tokens), ZERO_TEST);

        zero->left_children = calloc(1, sizeof(firth_parse_elements));
        zero->right_children = calloc(1, sizeof(firth_parse_elements));

        while (firth_has_more_tokens(tokens) &&
               !firth_match_token("else", tokens) &&
               !firth_match_token("end", tokens)) {
            firth_add_element(zero->left_children, firth_parse_elt(tokens, result));
        }
        if (firth_match_token("else", tokens)) {
            firth_take_token(tokens);
            while (firth_has_more_tokens(tokens) &&
                   !firth_match_token("end", tokens)) {
                firth_add_element(zero->right_children, firth_parse_elt(tokens, result));
            }
        }
        if (firth_match_token("end", tokens)) {
            firth_take_token(tokens);
        } else {
            firth_report_error("Expected end for zero? statement", tokens, result);
        }
        return zero;
    }
    return NULL;
}

firth_parse_element *firth_parse_elt(firth_tokens *tokens, firth_compilation_result *result) {

    firth_parse_element *op = firth_parse_op(tokens, result);
    if (op) {
        return op;
    }

    firth_parse_element *num = firth_parse_num(tokens, result);
    if (num) {
        return num;
    }

    firth_parse_element *zero_test = firth_parse_zero(tokens, result);
    if (zero_test) {
        return zero_test;
    }

    firth_parse_element *def = firth_parse_def(tokens, result);
    if (def) {
        return def;
    }

    firth_parse_element *call = firth_parse_call(tokens, result);
    if (call) {
        return call;
    }

    firth_parse_element *error_elt = firth_make_elt(firth_take_token(tokens), ERROR);
    firth_report_error("Unknown token", tokens, result);
    return error_elt;
}

//======================================================
// Code Generation
//======================================================
int firth_elt_token_equals(const firth_parse_element * elt, const char *s2) {
    return strcmp(elt->token->value, s2) == 0;
}

void firth_code_gen_elt(firth_parse_element * elt, firth_compilation_result *result) {
    if (elt->type == OP) {
        if (firth_elt_token_equals(elt, ".")) {
            strcat(result->lmsm_assembly, "SDUP\nSPOP\nOUT\n");
        } else if (firth_elt_token_equals(elt, "+")) {
            strcat(result->lmsm_assembly, "SADD\n");
        } else if (firth_elt_token_equals(elt, "-")) {
            strcat(result->lmsm_assembly, "SSUB\n");
        } else if (firth_elt_token_equals(elt, "*")) {
            strcat(result->lmsm_assembly, "SMUL\n");  //Added MUL, DIV, MAX, MIN
        } else if (firth_elt_token_equals(elt, "/")) {
            strcat(result->lmsm_assembly, "SDIV\n");
        } else if (firth_elt_token_equals(elt, "max")) {
            strcat(result->lmsm_assembly, "SMAX\n");
        } else if (firth_elt_token_equals(elt, "min")) {
            strcat(result->lmsm_assembly, "SMIN\n");
        } else if (firth_elt_token_equals(elt, "get")) {
            strcat(result->lmsm_assembly, "INP\nSPUSH\n");
        } else if (firth_elt_token_equals(elt, "pop")) {
            strcat(result->lmsm_assembly, "SPOP\n");
        } else if (firth_elt_token_equals(elt, "dup")) {
            strcat(result->lmsm_assembly, "SDUP\n");
        } else if (firth_elt_token_equals(elt, "swap")) {
            strcat(result->lmsm_assembly, "SSWAP\n");
        } else if (firth_elt_token_equals(elt, "return")) {
            strcat(result->lmsm_assembly, "RET\n");
        }
    } else if (elt->type == NUMBER) {
        strcat(result->lmsm_assembly, "LDI ");
        strcat(result->lmsm_assembly, elt->token->value);
        strcat(result->lmsm_assembly, "\n");
        strcat(result->lmsm_assembly, "SPUSH\n");
    } else if (elt->type == ZERO_TEST) {
        char if_zero_label[20];
        sprintf(if_zero_label, "if_zero_%d", result->label_num++);

        char end_zero_label[20];
        sprintf(end_zero_label, "end_zero_%d", result->label_num++);

        // branch if top of stack zero
        strcat(result->lmsm_assembly, "SPOP\nBRZ ");
        if (elt->left_children->first) {
            strcat(result->lmsm_assembly, if_zero_label);
        } else {
            strcat(result->lmsm_assembly, end_zero_label);
        }
        strcat(result->lmsm_assembly, "\n");

        // generate else
        if (elt->right_children->first) {
            struct firth_parse_element *child = elt->right_children->first;
            while (child != NULL) {
                firth_code_gen_elt(child, result);
                child = child->next_sibling;
            }
        }

        // jump to end of zero condition
        strcat(result->lmsm_assembly, "BRA ");
        strcat(result->lmsm_assembly, end_zero_label);
        strcat(result->lmsm_assembly, "\n");

        // generate if zero condition
        if (elt->left_children->first) {
            strcat(result->lmsm_assembly, if_zero_label);
            strcat(result->lmsm_assembly, " ");
            struct firth_parse_element *child = elt->left_children->first;
            while (child != NULL) {
                firth_code_gen_elt(child, result);
                child = child->next_sibling;
            }
        }

        // label end of zero conditional
        strcat(result->lmsm_assembly, end_zero_label);
        strcat(result->lmsm_assembly, " ");
    } else if (elt->type == CALL) {
        strcat(result->lmsm_assembly, "CALL ");
        strcat(result->lmsm_assembly, elt->token->value);
        strcat(result->lmsm_assembly, "\n");
    } else if (elt->type == DEF) {
        // function label
        strcat(result->lmsm_assembly, elt->name->value);
        strcat(result->lmsm_assembly, " ");
        // function body
        if (elt->left_children->first) {
            struct firth_parse_element *child = elt->left_children->first;
            while (child != NULL) {
                firth_code_gen_elt(child, result);
                child = child->next_sibling;
            }
        }
        // always append a RET
        strcat(result->lmsm_assembly, "RET\n");
    }

}

void firth_code_gen_top_level(firth_compilation_result *result) {
    struct firth_parse_element *elt = result->root_elements->first;
    while (elt != NULL) {
        if (elt->type != DEF) {
            firth_code_gen_elt(elt, result);
        }
        elt = elt->next_sibling;
    }
    strcat(result->lmsm_assembly, "HLT\n");
}

void firth_code_gen_functions(firth_compilation_result *result) {
    struct firth_parse_element *elt = result->root_elements->first;
    while (elt != NULL) {
        if (elt->type == DEF) {
            firth_code_gen_elt(elt, result);
        }
        elt = elt->next_sibling;
    }
}

void firth_code_gen(firth_compilation_result *result) {
    if (result->error == NULL) {
        firth_code_gen_top_level(result);
        firth_code_gen_functions(result);
    }
}

//======================================================
// Entry Point
//======================================================
void firth_delete_tokens(firth_tokens *tokens) {
    firth_token *token = tokens->start;
    while (token != NULL) {
        firth_token *to_delete = token;
        token = to_delete->next;
        free(to_delete);
    }
    free(tokens->original_src);
}

void firth_delete_compilation_result(firth_compilation_result * result){
    firth_delete_exprs(result->root_elements);
    firth_delete_tokens(result->tokens);
    free(result);
}

void firth_delete_expr(firth_parse_element *elt) {
    if (elt->left_children) {
        firth_delete_exprs(elt->left_children);
    }
    if (elt->right_children) {
        firth_delete_exprs(elt->right_children);
    }
    free(elt);
}

void firth_delete_exprs(firth_parse_elements *elts) {
    if (elts->first) {
        firth_parse_element *current = elts->first;
        while (current != NULL) {
            firth_parse_element *to_delete = current;
            current = to_delete->next_sibling;
            firth_delete_expr(to_delete);
        }
    }
}

firth_compilation_result *firth_compile(char *firth_src) {

    firth_compilation_result *result = calloc(1, sizeof(firth_compilation_result));

    void *root_elements = calloc(1, sizeof(firth_parse_elements));
    result->root_elements = root_elements;

    firth_tokens *tokens = firth_tokenize(firth_src);
    result->tokens = tokens;

    while (firth_has_more_tokens(tokens)) {
        firth_add_element(root_elements, firth_parse_elt(tokens, result));
    }

    firth_code_gen(result);

    return result;
}


