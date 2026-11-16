/*
 * codegen.h - Geracao do analisador sintatico descendente recursivo (etapa 5).
 *
 * A partir de <prefixo> gera <prefixo>.h e <prefixo>.c, em ANSI C:
 *   - um procedimento parse_<nao_terminal>() por nao terminal;
 *   - as primitivas avancar, match, erro_sintatico e a arvore (--tree);
 *   - no modo proprio, um analisador lexico gerado a partir de TOKENS;
 *     no modo Micro C, chamadas a proximo_token() do scanner do TP1;
 *   - uma funcao main que le o programa (ou um arquivo de tokens).
 */
#ifndef CODEGEN_H
#define CODEGEN_H

#include "gramatica.h"

typedef struct {
    int microc;            /* --scanner microc */
    int arvore;            /* --tree: gera suporte a impressao da arvore */
    const char *prefixo;   /* -o: caminho sem extensao */
    const char *origem;    /* nome do arquivo .ebnf (para comentarios) */
} OpcoesGeracao;

/* Retorna 0 em caso de sucesso. */
int codegen_gerar(Gramatica *g, const OpcoesGeracao *op);

#endif
