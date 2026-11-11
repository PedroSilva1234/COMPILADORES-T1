/*
 * util.c - Implementacao das funcoes auxiliares (ver util.h).
 *
 * Modulo FORNECIDO: nao e necessario altera-lo.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "util.h"

static const char *arquivo_atual = NULL;
static int num_erros = 0;
static int num_avisos = 0;

void *xmalloc(size_t n)
{
    void *p = malloc(n ? n : 1);
    if (p == NULL)
        fatal("memoria insuficiente");
    return p;
}

void *xcalloc(size_t n, size_t tam)
{
    void *p = calloc(n ? n : 1, tam ? tam : 1);
    if (p == NULL)
        fatal("memoria insuficiente");
    return p;
}

void *xrealloc(void *p, size_t n)
{
    void *q = realloc(p, n ? n : 1);
    if (q == NULL)
        fatal("memoria insuficiente");
    return q;
}

char *xstrdup(const char *s)
{
    return xstrndup(s, strlen(s));
}

char *xstrndup(const char *s, size_t n)
{
    char *p = (char *) xmalloc(n + 1);
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

char *ler_arquivo(const char *caminho)
{
    FILE *f;
    char *buf;
    size_t cap = 4096, tam = 0, lidos;

    f = fopen(caminho, "rb");
    if (f == NULL)
        return NULL;
    buf = (char *) xmalloc(cap);
    while ((lidos = fread(buf + tam, 1, cap - tam - 1, f)) > 0) {
        tam += lidos;
        if (cap - tam - 1 == 0) {
            cap *= 2;
            buf = (char *) xrealloc(buf, cap);
        }
    }
    buf[tam] = '\0';
    fclose(f);
    return buf;
}

void fatal(const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "gerador: erro fatal: ");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(EXIT_FAILURE);
}

void diag_definir_arquivo(const char *arquivo)
{
    arquivo_atual = arquivo;
}

static void diag_prefixo(int linha, int coluna, const char *tipo)
{
    if (arquivo_atual != NULL)
        fprintf(stderr, "%s:", arquivo_atual);
    if (linha > 0) {
        fprintf(stderr, "%d:", linha);
        if (coluna > 0)
            fprintf(stderr, "%d:", coluna);
    }
    fprintf(stderr, " %s: ", tipo);
}

void diag_erro(int linha, int coluna, const char *fmt, ...)
{
    va_list ap;
    diag_prefixo(linha, coluna, "erro");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    num_erros++;
}

void diag_aviso(int linha, int coluna, const char *fmt, ...)
{
    va_list ap;
    diag_prefixo(linha, coluna, "aviso");
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    num_avisos++;
}

int diag_num_erros(void)
{
    return num_erros;
}

int diag_num_avisos(void)
{
    return num_avisos;
}
