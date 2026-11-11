/*
 * bitset.h - Conjuntos de terminais representados como vetores de bits.
 *
 * Cada terminal da gramatica tem um indice (0 = fim de arquivo "$"),
 * e um Bitset guarda quais indices pertencem ao conjunto. Use-o para
 * FIRST, FOLLOW e para os conjuntos de teste do codigo gerado.
 *
 * Modulo FORNECIDO: nao e necessario altera-lo.
 */
#ifndef BITSET_H
#define BITSET_H

typedef struct {
    int n;               /* capacidade em elementos (0 = nao iniciado) */
    int npalavras;
    unsigned long *w;
} Bitset;

void bitset_iniciar(Bitset *b, int n);    /* conjunto vazio com n posicoes */
void bitset_liberar(Bitset *b);           /* aceita bitset nao iniciado */
void bitset_limpar(Bitset *b);
void bitset_add(Bitset *b, int i);
int  bitset_tem(const Bitset *b, int i);
int  bitset_vazio(const Bitset *b);
int  bitset_tamanho(const Bitset *b);     /* numero de elementos */

/* dest = dest U src. Retorna 1 se dest mudou (util no ponto fixo). */
int  bitset_unir(Bitset *dest, const Bitset *src);
void bitset_copiar(Bitset *dest, const Bitset *src);

/* 1 se a intersecao de a e b nao for vazia. */
int  bitset_intersecta(const Bitset *a, const Bitset *b);

/* res = a intersecao b (res ja iniciado com a mesma capacidade). */
void bitset_intersecao(Bitset *res, const Bitset *a, const Bitset *b);

/* Iteracao: menor elemento >= i, ou -1 se nao houver.
 *   for (t = bitset_proximo(b, 0); t >= 0; t = bitset_proximo(b, t + 1)) */
int  bitset_proximo(const Bitset *b, int i);

#endif
