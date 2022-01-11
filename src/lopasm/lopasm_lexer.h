/*
Created 11 January 2022
 */

#ifndef LOPASM_LEXER_H_
#define LOPASM_LEXER_H_

#include <sv.h>
#include <stdbool.h>
#include <stdio.h>

#include "../lopsinvm/lopsinvm.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

typedef struct {
    String_View file;
    size_t line;
    size_t col;
} LopAsm_TokLoc;

typedef enum {
    LOPASM_TOKEN_TYPE_INST = 0,
    LOPASM_TOKEN_TYPE_LIT_INT,
    LOPASM_TOKEN_TYPE_IDENTIFIER,

    COUNT_LOPASM_TOKEN_TYPES
} LopAsm_TokenType;

typedef struct {
    LopsinInstType type;
} LopAsm_InstToken;

typedef struct {
    int64_t value;
} LopAsm_LitIntToken;

typedef struct {
    String_View name;
} LopAsm_IdentifierToken;

typedef union {
    LopAsm_InstToken inst;
    LopAsm_LitIntToken lit_int;
    LopAsm_IdentifierToken identifier;
} LopAsm_TokenAs;


typedef struct {
    String_View text;
    LopAsm_TokenType type;
    LopAsm_TokenAs as;
} LopAsm_Token;

typedef struct {
    LopAsm_TokLoc loc;
    String_View source;
} LopAsm_Lexer;

bool lopasm_lexer_spit_token(LopAsm_Lexer *, LopAsm_Token *out);
void lopasm_print_token(FILE *stream, LopAsm_Token token);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOPASM_LEXER_H_ */
