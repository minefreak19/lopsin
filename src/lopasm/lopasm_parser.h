/*
Created 23 January 2022
 */

#ifndef LOPASM_PARSER_H_
#define LOPASM_PARSER_H_

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

#include <stdbool.h>

#include "./lopasm_lexer.h"

typedef struct {
    String_View name;
    size_t loc;
} LopAsm_Label;

typedef enum {
    LOPASM_PARSER_PHASE_NONE = 0,

    LOPASM_PARSER_PHASE_ONE,
    LOPASM_PARSER_PHASE_TWO,

    COUNT_LOPASM_PARSER_PHASES
} LopAsm_ParserPhase;

#define LOPASM_LABELS_CAP 1024
// TODO(#3): LopAsm_Parser's internal label list is not dynamically sized
typedef struct {
    LopAsm_Label labels[LOPASM_LABELS_CAP];
    size_t labels_sz;

    LopAsm_Token *tokens;
    size_t tokens_sz;
    size_t tokens_cap;

    // in phase one, this holds the instruction pointer
    // in phase two, this holds the token pointer (ie how many tokens have been consumed), because doing sh!t like `*parser->tokens++` is dangerous :)
    size_t ip;
    LopAsm_ParserPhase phase;
} LopAsm_Parser;

void lopasm_parser_next_phase(LopAsm_Parser *parser);
LopAsm_Parser *lopasm_parser_new(void);
bool lopasm_parser_spit_inst(LopAsm_Parser *parser, LopsinInst *out);
bool lopasm_parser_accept_token(LopAsm_Parser *parser, LopAsm_Token token);
void lopasm_parser_free(LopAsm_Parser *parser);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* LOPASM_PARSER_H_ */
