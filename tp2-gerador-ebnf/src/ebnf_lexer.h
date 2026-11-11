/*
 * ebnf_lexer.h - Analisador lexico da notacao EBNF de entrada (meta-lexer).
 *
 * Reconhece os simbolos do arquivo .ebnf (secao 2 do enunciado):
 *
 *   NOME_TERM   identificador so com maiusculas, digitos e '_'   (NUM, ID)
 *   NAOTERM     identificador iniciado por minuscula             (expr)
 *   LITERAL     "texto" ou 'texto'                               ("while")
 *   REGEX       /expressao regular/                              (/[0-9]+/)
 *   =  ;  |  [  ]  {  }  (  )
 *   palavras reservadas: TOKENS RULES SKIP EXTERN
 *   comentarios (* ... *) sao ignorados
 *
 * Modulo FORNECIDO: nao e necessario altera-lo.
 */
#ifndef EBNF_LEXER_H
#define EBNF_LEXER_H

#include <stddef.h>

typedef enum {
    MT_FIM, MT_ERRO,
    MT_NOME_TERM, MT_NAOTERM, MT_LITERAL, MT_REGEX,
    MT_IGUAL, MT_PV, MT_BARRA,
    MT_ABRE_COL, MT_FECHA_COL, MT_ABRE_CHAVE, MT_FECHA_CHAVE,
    MT_ABRE_PAR, MT_FECHA_PAR,
    MT_TOKENS, MT_RULES, MT_SKIP, MT_EXTERN
} MetaTipo;

typedef struct {
    MetaTipo tipo;
    char *texto;          /* lexema (sem aspas/barras); alocado */
    int linha, coluna;
} MetaToken;

typedef struct {
    const char *fonte;
    size_t pos;
    int linha, coluna;
} MetaLexer;

void        mlex_iniciar(MetaLexer *lx, const char *fonte);
MetaToken   mlex_proximo(MetaLexer *lx);  /* erros lexicos ja sao reportados */
void        mtok_liberar(MetaToken *t);
const char *mtipo_nome(MetaTipo t);       /* para mensagens: "';'", "literal" */

#endif
