/*
 * util.h - Funcoes auxiliares: alocacao segura, leitura de arquivos e
 * emissao de diagnosticos (erros e avisos) com linha e coluna.
 *
 * Modulo FORNECIDO: nao e necessario altera-lo.
 */
#ifndef UTIL_H
#define UTIL_H

#include <stddef.h>

/* Alocacao que encerra o programa com mensagem se faltar memoria. */
void *xmalloc(size_t n);
void *xcalloc(size_t n, size_t tam);
void *xrealloc(void *p, size_t n);
char *xstrdup(const char *s);
char *xstrndup(const char *s, size_t n);

/* Le o arquivo inteiro para uma string terminada em '\0' (liberar com free).
 * Retorna NULL se o arquivo nao puder ser aberto. */
char *ler_arquivo(const char *caminho);

/* Erro irrecuperavel: imprime a mensagem e encerra com EXIT_FAILURE. */
void fatal(const char *fmt, ...);

/* Diagnosticos sobre o arquivo .ebnf. Formato:
 *   arquivo:linha:coluna: erro: mensagem
 * Use linha <= 0 para omitir a posicao e coluna <= 0 para omitir a coluna. */
void diag_definir_arquivo(const char *arquivo);
void diag_erro(int linha, int coluna, const char *fmt, ...);
void diag_aviso(int linha, int coluna, const char *fmt, ...);
int  diag_num_erros(void);
int  diag_num_avisos(void);

#endif
