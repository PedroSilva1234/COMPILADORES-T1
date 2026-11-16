/*
 * conjuntos.h - Calculo de anulavel, FIRST e FOLLOW (etapa 3).
 *
 * Os conjuntos sao guardados tanto nos nao terminais (g->nts[i]) quanto
 * em cada no Expr, porque o gerador de codigo e a verificacao LL(1)
 * precisam deles para cada subexpressao (alternativas, [..] e {..}).
 */
#ifndef CONJUNTOS_H
#define CONJUNTOS_H

#include <stdio.h>
#include "gramatica.h"

/* Aloca (vazios) todos os bitsets de nao terminais e expressoes,
 * com g->nterms posicoes. Chame antes das funcoes abaixo. */
void conj_iniciar(Gramatica *g);

void conj_anulavel(Gramatica *g);   /* preenche o campo anulavel */
void conj_first(Gramatica *g);      /* requer anulavel */
void conj_follow(Gramatica *g);     /* requer anulavel e first */

/* Tabela da opcao --sets. */
void conj_imprimir(FILE *f, const Gramatica *g);

#endif
