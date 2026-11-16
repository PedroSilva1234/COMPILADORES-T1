/*
 * ebnf_parser.h - Analisador sintatico da notacao EBNF de entrada.
 *
 * Le o texto de um arquivo .ebnf e preenche a Gramatica: terminais,
 * diretivas SKIP, TOKENS EXTERN e as regras de RULES.
 */
#ifndef EBNF_PARSER_H
#define EBNF_PARSER_H

#include "gramatica.h"

/* Retorna 1 se a leitura terminou sem erros (diag_num_erros() == 0). */
int ebnf_analisar(Gramatica *g, const char *fonte);

#endif
