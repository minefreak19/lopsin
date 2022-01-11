#define SVDEF static inline
#include "lopasm.h" // also includes <sv.h>

#define SV_IMPLEMENTATION
#include <sv.h>

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#include "util.h"

typedef LopAsm_Token Token;
typedef LopAsm_InstToken InstToken;
typedef LopAsm_Lexer Lexer;

static inline bool notisspace(char c)
{
    return !isspace(c);
}

// please the pedentic compiler!
static inline bool is_space(char c) 
{
    return isspace(c);
}

static inline String_View sv_chop_by_whitespace(String_View *sv)
{
    return sv_chop_left_while(sv, notisspace);
}

static bool sv_try_chop_i64(String_View *sv, int64_t *out)
{
    char *contents = NOTNULL(malloc((sv->count + 1) * sizeof(char)));
    
    memcpy(contents, sv->data, sv->count);
    contents[sv->count] = '\0';

    char *endptr = NULL;
    int64_t result = strtoll(contents, &endptr, 0);

    size_t len = endptr - contents;
    free(contents);

    if (len == 0) 
        return false;
    else {
        sv->count -= len;
        sv->data  += len;
        if (out) *out = result;
    }

    return true;
}

static bool lex_tok_as_inst(Token *tok)
{
    for (LopsinInstType i = 0; i < COUNT_LOPSIN_INST_TYPES; i++) {
        if (sv_eq(tok->text, sv_from_cstr(LOPSIN_INST_TYPE_NAMES[i]))) {
            tok->type = LOPASM_TOKEN_TYPE_INST;
            tok->as.inst = (InstToken) {
                .type = i,
            };
            return true;
        }
    }

    return false;
}

static bool lex_tok_as_i64(Token *tok)
{
    String_View tok_text = tok->text;
    if (!sv_try_chop_i64(&tok_text, &tok->as.lit_int.value)) {
        return false;
    }

    if (tok_text.count > 0) {
        return false;
    }

    tok->type = LOPASM_TOKEN_TYPE_LIT_INT;
    return true;
}

static bool lex_tok_as_identifier(Token *tok)
{
    String_View toktext = tok->text;
    String_View ident_name = sv_trim(sv_chop_by_delim(&toktext, ':'));

    if (ident_name.count == 0) {
        return false;
    }

    tok->as.identifier.name = ident_name;
    tok->type = LOPASM_TOKEN_TYPE_IDENTIFIER;

    return true;
}

bool lopasm_lexer_spit_token(Lexer *lexer, Token *out)
{
    lexer->source = sv_trim_left(lexer->source);
    if (lexer->source.count == 0) return false;

    Token result;

    result.text = sv_chop_by_whitespace(&lexer->source);

    if (result.text.count == 0) {
        return lopasm_lexer_spit_token(lexer, out);
    }

    if (lex_tok_as_inst(&result)) {
    } else if (lex_tok_as_i64(&result)) {
    } else if (lex_tok_as_identifier(&result)) {
    } else {
        assert(false && "unreachable");
    }

    if (out) *out = result;
    return true;
}

void lopasm_print_token(FILE *stream, Token token)
#ifdef _DEBUG
{
    fprintf(stream, 
        "Token (%02d):\t`"SV_Fmt"`\n",
            token.type, 
            SV_Arg(token.text));
}
#else
{
    (void) stream;
    (void) token;
    return;
}
#endif
