#include "./lopasm_parser.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <sv.h>

#include "util.h"

typedef LopAsm_Parser Parser;
typedef LopAsm_Token Token;
typedef LopAsm_Label Label;

#define PARSER_TOKENS_CAP_INC 64
static void append_token(Parser *parser, Token token)
{
    if (parser->tokens_sz >= parser->tokens_cap) {
        parser->tokens = realloc(parser->tokens, parser->tokens_cap + PARSER_TOKENS_CAP_INC);
        parser->tokens_cap += PARSER_TOKENS_CAP_INC;
    }

    parser->tokens[parser->tokens_sz++] = token;
}

static bool parse_label_def(Parser *parser, Token token, Label *out)
{
    assert(token.type == LOPASM_TOKEN_TYPE_LABEL_DEF);

    if (out) {
        *out = (Label) {
            .loc = parser->ip,
            .name = token.as.label_def.name,
        };
    }

    return true;
}

void lopasm_parser_next_phase(LopAsm_Parser *parser)
{
    parser->phase++;
    switch  (parser->phase) {
        case LOPASM_PARSER_PHASE_ONE: {
            parser->ip = 0;
        } break;

        case LOPASM_PARSER_PHASE_TWO: {
            parser->ip = 0;
        } break;

        default: {
            fprintf(stderr, "ERROR: Illegal parser state");
            exit(1);
        }
    }
}

bool lopasm_parser_accept_token(LopAsm_Parser *parser, LopAsm_Token token)
{
    assert(parser->phase == LOPASM_PARSER_PHASE_ONE);

    switch (token.type) {
        case LOPASM_TOKEN_TYPE_LABEL_DEF: {
            if (parse_label_def(parser, token, &parser->labels[parser->labels_sz])) {
                parser->labels_sz++;
            } else return false;
        } break;

        case LOPASM_TOKEN_TYPE_INST: {
            parser->ip++; // we'll catch things like `push swap` at a later stage
            FALLTHROUGH;
        }

        default: {
            append_token(parser, token);
        }
    }

    return true;
}

static bool parse_identifier_value(const Parser *parser, Token token, LopsinValue *out)
{
    assert(token.type == LOPASM_TOKEN_TYPE_IDENTIFIER);


    for (size_t i = 0; i < parser->labels_sz; i++) {
        Label label = parser->labels[i];
        if (sv_eq(token.as.identifier.name, label.name)) {

            if (out) *out = (LopsinValue) {
                .type = LOPSIN_TYPE_I64,
                .as.i64 = label.loc,
            };

            return true;
        }
    }

    return false;
}

static bool parse_operand(const Parser *parser, Token token, LopsinValue *out)
{
    switch (token.type) {
        case LOPASM_TOKEN_TYPE_IDENTIFIER: {
            bool success = parse_identifier_value(parser, token, out);
            return success;
        } break;

        case LOPASM_TOKEN_TYPE_LIT_INT: {
            if (out) *out = (LopsinValue) {
                .type = LOPSIN_TYPE_I64,
                .as.i64 = token.as.lit_int.value,
            };
            return true;
        } break;

        default: {
            fprintf(stderr, "ERROR: Unexpected operand to `"SV_Fmt"`: `"SV_Fmt"`\n",
                    SV_Arg(parser->tokens[parser->ip - 2].text),
                    SV_Arg(token.text));
            exit(1);
        }
    }
    return false;
}

bool lopasm_parser_spit_inst(LopAsm_Parser *parser, LopsinInst *out)
{
    assert(parser->phase == LOPASM_PARSER_PHASE_TWO);

    if (parser->ip >= parser->tokens_sz) return false;

    LopsinInst result;

    {
        Token tok = parser->tokens[parser->ip++];

        switch (tok.type) {
            case LOPASM_TOKEN_TYPE_INST: {
                result.type = tok.as.inst.type;
                if (requires_operand(tok.as.inst.type)) {
                    if (parser->ip >= parser->tokens_sz) {
                        fprintf(stderr, 
                            "ERROR: Instruction of type `%s` requires an operand (found none)\n",
                            LOPSIN_INST_TYPE_NAMES[result.type]);

                        exit(1);
                    }

                    Token optok = parser->tokens[parser->ip++];
                    if (!parse_operand(parser, optok, &result.operand)) {
                        return false;
                    }
                }
            } break;

            case LOPASM_TOKEN_TYPE_LIT_INT: 
            case LOPASM_TOKEN_TYPE_IDENTIFIER: 
            case LOPASM_TOKEN_TYPE_LABEL_DEF:
            {
                fprintf(stderr, "ERROR: Unexpected token `"SV_Fmt"`\n",
                        SV_Arg(tok.text));
                exit(1);
            }

            default: {
                CRASH("Bad token type");
            }
        }
    }

    if (out) *out = result;
    return true;
}

void lopasm_parser_free(LopAsm_Parser *parser)
{
    free(parser->tokens);
    free(parser);
}

#define LOPASM_PARSER_INITIAL_TOKENS_CAP 1024
LopAsm_Parser *new_parser(void)
{
    Parser *parser = NOTNULL(malloc(sizeof(Parser)));
    *parser = (Parser) {
        .ip = 0,
        .labels = {0},
        .labels_sz = 0,
        .tokens = NOTNULL(calloc(sizeof(Token), LOPASM_PARSER_INITIAL_TOKENS_CAP)),
        .tokens_cap = LOPASM_PARSER_INITIAL_TOKENS_CAP,
        .tokens_sz = 0,
        .phase = LOPASM_PARSER_PHASE_ONE,
    };

    return parser;
}
