/*
 * verificacao.h - Verificacoes semanticas da gramatica (etapa 2).
 */
#ifndef VERIFICACAO_H
#define VERIFICACAO_H

#include "gramatica.h"

/* Nao terminais indefinidos, terminais nao declarados, nao terminais
 * inalcancaveis (aviso) e improdutivos. Retorna o numero de erros. */
int verif_basica(Gramatica *g, int modo_microc);

/* Recursao a esquerda direta e indireta. Requer o atributo anulavel
 * ja calculado (conjuntos.c). Retorna o numero de erros. */
int verif_recursao_esquerda(Gramatica *g);

#endif
