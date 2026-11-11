/*
 * gramatica.c - Implementacao da representacao da gramatica (ver gramatica.h).
 *
 * Modulo FORNECIDO: nao e necessario altera-lo.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gramatica.h"
#include "util.h"

/* Monta o nome de exibicao de um literal: 'while', '+', "'" */
static char *exibicao_literal(const char *texto)
{
    size_t n = strlen(texto);
    char *s = (char *) xmalloc(n + 3);
    char aspa = strchr(texto, '\'') != NULL ? '"' : '\'';
    s[0] = aspa;
    memcpy(s + 1, texto, n);
    s[n + 1] = aspa;
    s[n + 2] = '\0';
    return s;
}

Gramatica *gram_criar(const char *arquivo)
{
    Gramatica *g = (Gramatica *) xcalloc(1, sizeof(Gramatica));
    g->arquivo = xstrdup(arquivo);
    g->inicial = -1;
    /* terminal 0: fim de arquivo */
    gram_terminal(g, "$", 0, 0, 0);
    g->terms[TERM_FIM].declarado = 1;
    free(g->terms[TERM_FIM].exibicao);
    g->terms[TERM_FIM].exibicao = xstrdup("$");
    return g;
}

void gram_liberar(Gramatica *g)
{
    int i;
    if (g == NULL)
        return;
    for (i = 0; i < g->nterms; i++) {
        free(g->terms[i].nome);
        free(g->terms[i].regex);
        free(g->terms[i].exibicao);
        free(g->terms[i].nome_c);
    }
    free(g->terms);
    for (i = 0; i < g->nnts; i++) {
        free(g->nts[i].nome);
        expr_liberar(g->nts[i].regra);
        bitset_liberar(&g->nts[i].first);
        bitset_liberar(&g->nts[i].follow);
    }
    free(g->nts);
    for (i = 0; i < g->nskips; i++)
        free(g->skips[i]);
    free(g->skips);
    free(g->tokens_externos);
    free(g->arquivo);
    free(g);
}

int gram_busca_terminal(const Gramatica *g, const char *nome, int literal)
{
    int i;
    for (i = 0; i < g->nterms; i++)
        if (g->terms[i].literal == literal && strcmp(g->terms[i].nome, nome) == 0)
            return i;
    return -1;
}

int gram_terminal(Gramatica *g, const char *nome, int literal,
                  int linha, int coluna)
{
    int i = gram_busca_terminal(g, nome, literal);
    Terminal *t;
    if (i >= 0)
        return i;
    if (g->nterms == g->capterms) {
        g->capterms = g->capterms ? 2 * g->capterms : 16;
        g->terms = (Terminal *) xrealloc(g->terms,
                                         (size_t) g->capterms * sizeof(Terminal));
    }
    t = &g->terms[g->nterms];
    memset(t, 0, sizeof(Terminal));
    t->nome = xstrdup(nome);
    t->literal = literal;
    t->declarado = literal;
    t->exibicao = literal ? exibicao_literal(nome) : xstrdup(nome);
    t->linha = linha;
    t->coluna = coluna;
    return g->nterms++;
}

int gram_busca_naoterminal(const Gramatica *g, const char *nome)
{
    int i;
    for (i = 0; i < g->nnts; i++)
        if (strcmp(g->nts[i].nome, nome) == 0)
            return i;
    return -1;
}

int gram_naoterminal(Gramatica *g, const char *nome, int linha, int coluna)
{
    int i = gram_busca_naoterminal(g, nome);
    NaoTerminal *a;
    if (i >= 0)
        return i;
    if (g->nnts == g->capnts) {
        g->capnts = g->capnts ? 2 * g->capnts : 16;
        g->nts = (NaoTerminal *) xrealloc(g->nts,
                                          (size_t) g->capnts * sizeof(NaoTerminal));
    }
    a = &g->nts[g->nnts];
    memset(a, 0, sizeof(NaoTerminal));
    a->nome = xstrdup(nome);
    a->linha = linha;
    a->coluna = coluna;
    return g->nnts++;
}

void gram_add_skip(Gramatica *g, const char *regex)
{
    if (g->nskips == g->capskips) {
        g->capskips = g->capskips ? 2 * g->capskips : 4;
        g->skips = (char **) xrealloc(g->skips, (size_t) g->capskips * sizeof(char *));
    }
    g->skips[g->nskips++] = xstrdup(regex);
}

Expr *expr_criar(TipoExpr tipo, int linha, int coluna)
{
    Expr *e = (Expr *) xcalloc(1, sizeof(Expr));
    e->tipo = tipo;
    e->simbolo = -1;
    e->linha = linha;
    e->coluna = coluna;
    return e;
}

Expr *expr_simbolo(TipoExpr tipo, int simbolo, int linha, int coluna)
{
    Expr *e = expr_criar(tipo, linha, coluna);
    e->simbolo = simbolo;
    return e;
}

Expr *expr_unario(TipoExpr tipo, Expr *filho, int linha, int coluna)
{
    Expr *e = expr_criar(tipo, linha, coluna);
    expr_adicionar(e, filho);
    return e;
}

void expr_adicionar(Expr *pai, Expr *filho)
{
    if (pai->nfilhos == pai->capfilhos) {
        pai->capfilhos = pai->capfilhos ? 2 * pai->capfilhos : 4;
        pai->filhos = (Expr **) xrealloc(pai->filhos,
                                         (size_t) pai->capfilhos * sizeof(Expr *));
    }
    pai->filhos[pai->nfilhos++] = filho;
}

void expr_liberar(Expr *e)
{
    int i;
    if (e == NULL)
        return;
    for (i = 0; i < e->nfilhos; i++)
        expr_liberar(e->filhos[i]);
    free(e->filhos);
    bitset_liberar(&e->first);
    bitset_liberar(&e->follow);
    free(e);
}

void expr_percorrer(Expr *e, void (*f)(Expr *e, void *ctx), void *ctx)
{
    int i;
    if (e == NULL)
        return;
    f(e, ctx);
    for (i = 0; i < e->nfilhos; i++)
        expr_percorrer(e->filhos[i], f, ctx);
}

/* Precedencia para decidir parenteses: ALT < SEQ < atomo */
static int precedencia(const Expr *e)
{
    switch (e->tipo) {
    case E_ALT: return 0;
    case E_SEQ: return 1;
    default:    return 2;
    }
}

static void imprimir_rec(FILE *f, const Gramatica *g, const Expr *e, int minimo)
{
    int i, parenteses = precedencia(e) < minimo;
    if (parenteses)
        fputs("( ", f);
    switch (e->tipo) {
    case E_TERM:
        fputs(g->terms[e->simbolo].exibicao, f);
        break;
    case E_NAOTERM:
        fputs(g->nts[e->simbolo].nome, f);
        break;
    case E_SEQ:
        for (i = 0; i < e->nfilhos; i++) {
            if (i > 0)
                fputc(' ', f);
            imprimir_rec(f, g, e->filhos[i], 2);
        }
        break;
    case E_ALT:
        for (i = 0; i < e->nfilhos; i++) {
            if (i > 0)
                fputs(" | ", f);
            imprimir_rec(f, g, e->filhos[i], 1);
        }
        break;
    case E_OPC:
        fputs("[ ", f);
        imprimir_rec(f, g, e->filhos[0], 0);
        fputs(" ]", f);
        break;
    case E_REP:
        fputs("{ ", f);
        imprimir_rec(f, g, e->filhos[0], 0);
        fputs(" }", f);
        break;
    case E_VAZIO:
        break;
    }
    if (parenteses)
        fputs(" )", f);
}

void expr_imprimir(FILE *f, const Gramatica *g, const Expr *e)
{
    if (e != NULL)
        imprimir_rec(f, g, e, 0);
}

void gram_imprimir(FILE *f, const Gramatica *g)
{
    int i;
    for (i = 0; i < g->nnts; i++) {
        if (g->nts[i].regra == NULL)
            continue;
        fprintf(f, "%s = ", g->nts[i].nome);
        expr_imprimir(f, g, g->nts[i].regra);
        fputs(" ;\n", f);
    }
}

void gram_imprimir_conjunto(FILE *f, const Gramatica *g, const Bitset *b)
{
    int t, primeiro = 1;
    fputc('{', f);
    for (t = bitset_proximo(b, 0); t >= 0; t = bitset_proximo(b, t + 1)) {
        fprintf(f, "%s%s", primeiro ? "" : ", ", g->terms[t].exibicao);
        primeiro = 0;
    }
    fputc('}', f);
}
