/*
 * gramatica.h - Representacao interna de uma gramatica EBNF.
 *
 * Uma gramatica e formada por:
 *   - uma tabela de terminais (indice 0 reservado para o fim de arquivo "$");
 *   - uma tabela de nao terminais, cada um com sua regra (arvore Expr);
 *   - expressoes regulares das diretivas SKIP;
 *   - o simbolo inicial (o primeiro nao terminal definido em RULES).
 *
 * O lado direito de cada regra e uma arvore de expressoes:
 *
 *   EBNF                Expr
 *   t (terminal)        E_TERM     (simbolo = indice em g->terms)
 *   B (nao terminal)    E_NAOTERM  (simbolo = indice em g->nts)
 *   a b c               E_SEQ      (filhos = a, b, c)
 *   a | b | c           E_ALT      (filhos = a, b, c)
 *   [ a ]               E_OPC      (filhos[0] = a)
 *   { a }               E_REP      (filhos[0] = a)
 *   ( a )               a          (o agrupamento nao gera no proprio)
 *   (sequencia vazia)   E_VAZIO
 *
 * Os campos anulavel, first e follow de Expr e NaoTerminal sao
 * preenchidos pelo modulo conjuntos.c.
 *
 * Modulo FORNECIDO: nao e necessario altera-lo, mas voce pode acrescentar
 * campos se precisar.
 */
#ifndef GRAMATICA_H
#define GRAMATICA_H

#include <stdio.h>
#include "bitset.h"

#define TERM_FIM 0   /* indice do terminal especial "$" (fim de arquivo) */

typedef enum {
    E_TERM, E_NAOTERM, E_SEQ, E_ALT, E_OPC, E_REP, E_VAZIO
} TipoExpr;

typedef struct Expr {
    TipoExpr tipo;
    int simbolo;               /* E_TERM e E_NAOTERM */
    struct Expr **filhos;      /* E_SEQ, E_ALT: n filhos; E_OPC, E_REP: 1 */
    int nfilhos, capfilhos;
    int linha, coluna;         /* posicao no arquivo .ebnf */

    /* Atributos calculados em conjuntos.c */
    int anulavel;              /* 1 se a expressao deriva a cadeia vazia */
    Bitset first;              /* FIRST da expressao */
    Bitset follow;             /* tokens que podem vir logo apos ela */
} Expr;

typedef struct {
    char *nome;        /* "NUM"; para literais, o texto sem aspas: "while", "+" */
    int literal;       /* 1 se veio de "..." ou '...' */
    char *regex;       /* expressao regular de TOKENS (NULL para literais) */
    char *exibicao;    /* nome para mensagens: NUM, 'while', '+' */
    char *nome_c;      /* nome no codigo gerado (definido em codegen.c) */
    int declarado;     /* 1 se declarado em TOKENS (literais: sempre 1) */
    int linha, coluna; /* primeira ocorrencia */
} Terminal;

typedef struct {
    char *nome;
    Expr *regra;       /* NULL enquanto nao definido */
    int linha, coluna; /* primeira ocorrencia (uso ou definicao) */
    int linha_def;     /* linha da definicao (0 se indefinido) */

    /* Atributos calculados em conjuntos.c */
    int anulavel;
    Bitset first, follow;
} NaoTerminal;

typedef struct {
    char *arquivo;
    Terminal *terms;     int nterms, capterms;
    NaoTerminal *nts;    int nnts, capnts;
    char **skips;        int nskips, capskips;
    int inicial;              /* indice do simbolo inicial (-1 se nenhum) */
    char *tokens_externos;    /* caminho de "TOKENS EXTERN"; NULL se nao usado */
} Gramatica;

/* --- Gramatica --- */
Gramatica *gram_criar(const char *arquivo);
void gram_liberar(Gramatica *g);

/* Busca o terminal; se nao existir, cria. Retorna o indice. */
int gram_terminal(Gramatica *g, const char *nome, int literal,
                  int linha, int coluna);
/* Busca o terminal; retorna -1 se nao existir. */
int gram_busca_terminal(const Gramatica *g, const char *nome, int literal);

/* Busca o nao terminal; se nao existir, cria (ainda sem regra). */
int gram_naoterminal(Gramatica *g, const char *nome, int linha, int coluna);
int gram_busca_naoterminal(const Gramatica *g, const char *nome);

void gram_add_skip(Gramatica *g, const char *regex);

/* --- Expressoes --- */
Expr *expr_criar(TipoExpr tipo, int linha, int coluna);
Expr *expr_simbolo(TipoExpr tipo, int simbolo, int linha, int coluna);
Expr *expr_unario(TipoExpr tipo, Expr *filho, int linha, int coluna);
void  expr_adicionar(Expr *pai, Expr *filho);
void  expr_liberar(Expr *e);

/* Percorre a arvore em pre-ordem chamando f(e, ctx) para cada no. */
void  expr_percorrer(Expr *e, void (*f)(Expr *e, void *ctx), void *ctx);

/* Imprime a expressao de volta em EBNF (usado em mensagens e comentarios). */
void  expr_imprimir(FILE *f, const Gramatica *g, const Expr *e);

/* Imprime a gramatica inteira em EBNF (opcao --debug). */
void  gram_imprimir(FILE *f, const Gramatica *g);

/* Imprime um conjunto de terminais como {A, B, 'c'} */
void  gram_imprimir_conjunto(FILE *f, const Gramatica *g, const Bitset *b);

#endif
