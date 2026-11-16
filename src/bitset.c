/*
 * bitset.c - Implementacao de conjuntos de bits (ver bitset.h).
 *
 * Modulo FORNECIDO: nao e necessario altera-lo.
 */
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include "bitset.h"
#include "util.h"

#define BITS ((int) (CHAR_BIT * sizeof(unsigned long)))

void bitset_iniciar(Bitset *b, int n)
{
    b->n = n;
    b->npalavras = (n + BITS - 1) / BITS;
    b->w = (unsigned long *) xcalloc((size_t) (b->npalavras ? b->npalavras : 1),
                                     sizeof(unsigned long));
}

void bitset_liberar(Bitset *b)
{
    if (b->w != NULL)
        free(b->w);
    b->w = NULL;
    b->n = 0;
    b->npalavras = 0;
}

void bitset_limpar(Bitset *b)
{
    if (b->w != NULL)
        memset(b->w, 0, (size_t) b->npalavras * sizeof(unsigned long));
}

void bitset_add(Bitset *b, int i)
{
    if (i < 0 || i >= b->n)
        fatal("bitset_add: indice %d fora do intervalo [0, %d)", i, b->n);
    b->w[i / BITS] |= 1UL << (i % BITS);
}

int bitset_tem(const Bitset *b, int i)
{
    if (i < 0 || i >= b->n)
        return 0;
    return (b->w[i / BITS] >> (i % BITS)) & 1UL ? 1 : 0;
}

int bitset_vazio(const Bitset *b)
{
    int k;
    for (k = 0; k < b->npalavras; k++)
        if (b->w[k] != 0)
            return 0;
    return 1;
}

int bitset_tamanho(const Bitset *b)
{
    int i, c = 0;
    for (i = 0; i < b->n; i++)
        c += bitset_tem(b, i);
    return c;
}

int bitset_unir(Bitset *dest, const Bitset *src)
{
    int k, mudou = 0;
    unsigned long antes;
    if (dest->n != src->n)
        fatal("bitset_unir: tamanhos diferentes (%d e %d)", dest->n, src->n);
    for (k = 0; k < dest->npalavras; k++) {
        antes = dest->w[k];
        dest->w[k] |= src->w[k];
        if (dest->w[k] != antes)
            mudou = 1;
    }
    return mudou;
}

void bitset_copiar(Bitset *dest, const Bitset *src)
{
    if (dest->n != src->n)
        fatal("bitset_copiar: tamanhos diferentes (%d e %d)", dest->n, src->n);
    memcpy(dest->w, src->w, (size_t) dest->npalavras * sizeof(unsigned long));
}

int bitset_intersecta(const Bitset *a, const Bitset *b)
{
    int k;
    if (a->n != b->n)
        fatal("bitset_intersecta: tamanhos diferentes (%d e %d)", a->n, b->n);
    for (k = 0; k < a->npalavras; k++)
        if ((a->w[k] & b->w[k]) != 0)
            return 1;
    return 0;
}

void bitset_intersecao(Bitset *res, const Bitset *a, const Bitset *b)
{
    int k;
    if (a->n != b->n || res->n != a->n)
        fatal("bitset_intersecao: tamanhos diferentes");
    for (k = 0; k < a->npalavras; k++)
        res->w[k] = a->w[k] & b->w[k];
}

int bitset_proximo(const Bitset *b, int i)
{
    if (i < 0)
        i = 0;
    for (; i < b->n; i++)
        if (bitset_tem(b, i))
            return i;
    return -1;
}
