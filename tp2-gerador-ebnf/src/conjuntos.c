/*
 * conjuntos.c - Calculo de anulavel, FIRST e FOLLOW (etapa 3).
 *
 * Fornecido: conj_iniciar e conj_imprimir.
 *
 * TODO(aluno): conj_anulavel, conj_first e conj_follow. Os tres sao
 * calculados por iteracao ate ponto fixo: repita uma passada sobre todas
 * as regras enquanto algum conjunto (ou flag) mudar. bitset_unir devolve
 * 1 quando o conjunto de destino muda, o que ajuda a detectar o ponto fixo.
 */
#include <stdio.h>
#include <string.h>
#include "conjuntos.h"
#include "util.h"

/* ------------------------------------------------------------------ */
/* Fornecido                                                          */
/* ------------------------------------------------------------------ */

static void iniciar_expr(Expr *e, void *ctx)
{
    int n = *(int *) ctx;
    bitset_liberar(&e->first);
    bitset_liberar(&e->follow);
    bitset_iniciar(&e->first, n);
    bitset_iniciar(&e->follow, n);
    e->anulavel = 0;
}

void conj_iniciar(Gramatica *g)
{
    int i, n = g->nterms;
    for (i = 0; i < g->nnts; i++) {
        bitset_liberar(&g->nts[i].first);
        bitset_liberar(&g->nts[i].follow);
        bitset_iniciar(&g->nts[i].first, n);
        bitset_iniciar(&g->nts[i].follow, n);
        g->nts[i].anulavel = 0;
        expr_percorrer(g->nts[i].regra, iniciar_expr, &n);
    }
}

void conj_imprimir(FILE *f, const Gramatica *g)
{
    int i;
    size_t largura = strlen("NAO TERMINAL");
    for (i = 0; i < g->nnts; i++)
        if (strlen(g->nts[i].nome) > largura)
            largura = strlen(g->nts[i].nome);
    fprintf(f, "%-*s  ANULAVEL  FIRST / FOLLOW\n", (int) largura, "NAO TERMINAL");
    for (i = 0; i < g->nnts; i++) {
        fprintf(f, "%-*s  %-8s  FIRST  = ", (int) largura, g->nts[i].nome,
                g->nts[i].anulavel ? "sim" : "nao");
        gram_imprimir_conjunto(f, g, &g->nts[i].first);
        fprintf(f, "\n%-*s            FOLLOW = ", (int) largura, "");
        gram_imprimir_conjunto(f, g, &g->nts[i].follow);
        fputc('\n', f);
    }
}

/* ------------------------------------------------------------------ */
/* A implementar                                                      */
/* ------------------------------------------------------------------ */

static int expr_anulavel(const Gramatica *g, Expr *e)
{
    int i, v= 0;

    switch(e->tipo)
    {
        case E_TERM:
            v=0;
            break;
        case E_VAZIO:
            v=1;
            break;
        case E_NAOTERM:
            v = g->nts[e->simbolo].anulavel; /* Retorna o valor de anulavel do nao terminal */
            break;
        case E_SEQ:
            v =1;
            for(i = 0; i < e->nfilhos; i++)
                if(!expr_anulavel(g, e->filhos[i]))
                    v = 0; /* Se um filho não é anulável, a sequência falha */
            break;
        case E_ALT:
            v=0;
            for(i = 0; i < e->nfilhos; i++)
                if(expr_anulavel(g, e->filhos[i]))
                    v = 1; /* Se algum filho é anulável, a alternativa é anulável */
            break;

        default:
            expr_anulavel(g, e->filhos[0]); /* E_OPC, E_REP: retorna o valor do filho */
            v= 1; /* Caso padrão para segurança */
            break;
    }

    e->anulavel = v; /* Atribui o valor calculado */

    return v; /* Retorna o valor de anulável da expressão */
}

void conj_anulavel(Gramatica *g)
{
    int i, mudou;
    do
    {
        mudou = 0;
        for (i = 0; i <g->nnts; i++)
        {
            if(g->nts[i].regra != NULL)
            {
                int anterior = g->nts[i].anulavel;
                g->nts[i].anulavel = expr_anulavel(g, g->nts[i].regra);
                if(anterior != g->nts[i].anulavel)
                    mudou = 1;
            }

        }
    }while(mudou);

}

/* TODO(aluno): calcular e->first para cada no Expr e g->nts[i].first.
 *   E_TERM t: {t}     E_NAOTERM B: FIRST(B)     E_VAZIO: {}
 *   E_SEQ a1..an: FIRST(a1) U FIRST(a2) ... ate o primeiro ai nao anulavel
 *   E_ALT, E_OPC, E_REP: uniao dos FIRST dos filhos
 * Calcule o FIRST de TODOS os filhos de uma sequencia, mesmo os que
 * nao contribuem para o FIRST dela: o gerador de codigo usa-os. */

static void expr_first(Expr *e, Gramatica *g)
{
    int i;

    /* FIRST de TODOS os filhos, mesmo dos que nao contribuem para o do no */
    for (i = 0; i < e->nfilhos; i++)
        expr_first(e->filhos[i], g);

    switch (e->tipo)
    {
        case E_TERM:
            bitset_add(&e->first, e->simbolo);
            break;
        case E_NAOTERM:
            bitset_unir(&e->first, &g->nts[e->simbolo].first);
            break;
        case E_VAZIO:
            break;
        case E_SEQ:
            for (i = 0; i < e->nfilhos; i++)
            {
                bitset_unir(&e->first, &e->filhos[i]->first);
                if (!e->filhos[i]->anulavel)
                    break; /* para no primeiro filho nao anulavel */
            }
            break;
        case E_ALT:
        case E_OPC:
        case E_REP:
            for (i = 0; i < e->nfilhos; i++)
                bitset_unir(&e->first, &e->filhos[i]->first);
            break;
    }
}

void conj_first(Gramatica *g)
{
    int i, mudou;

    do
    {
        mudou = 0;
        for (i = 0; i < g->nnts; i++)
        {
            if (g->nts[i].regra != NULL)
            {
                expr_first(g->nts[i].regra, g);
                if (bitset_unir(&g->nts[i].first, &g->nts[i].regra->first))
                    mudou = 1;
            }
        }
    } while (mudou);
}

/* TODO(aluno): calcular e->follow (o que pode vir logo apos cada
 * construcao) e g->nts[i].follow. FOLLOW(inicial) contem $ (TERM_FIM).
 * Para cada regra A = alfa, faca follow(alfa) U= FOLLOW(A) e propague:
 *   E_SEQ a1..an:  follow(ai) U= FIRST(ai+1 ... an), mais follow(seq)
 *                  se ai+1 ... an for anulavel
 *   E_ALT, E_OPC:  follow(filho) U= follow(no)
 *   E_REP { a }:   follow(a) U= follow(no) U FIRST(a)
 *   E_NAOTERM B:   FOLLOW(B) U= follow(no)
 * Repita ate nada mudar. */

 void expr_follow(Expr *e, Gramatica *g, int *mudou)
 {
    int i, j,resto_anulavel;
    Expr *f;
    switch(e->tipo)
    {
        case E_NAOTERM:
            *mudou |= bitset_unir(&g->nts[e->simbolo].follow, &e->follow);
            break;
        case E_SEQ:
            for(i = 0; i < e->nfilhos; i++)
            {
                f = e->filhos[i];
                resto_anulavel = 1;
                for(j = i + 1; j < e->nfilhos && resto_anulavel; j++)
                {
                    *mudou |= bitset_unir(&e->filhos[i]->follow, &e->filhos[j]->first);
                    if(!e->filhos[j]->anulavel)
                        resto_anulavel = 0;
                }
                if(resto_anulavel)
                    *mudou |= bitset_unir(&e->filhos[i]->follow, &e->follow);
                expr_follow(f, g, mudou);
            }
            break;

        case E_ALT:
        case E_OPC:
            for(i = 0; i < e->nfilhos; i++)
            {
                *mudou |= bitset_unir(&e->filhos[i]->follow, &e->follow);
                expr_follow(e->filhos[i], g, mudou);
            }
            break;
        case E_REP:
            f = e->filhos[0];
            *mudou |= bitset_unir(&f->follow, &e->follow);
            *mudou |= bitset_unir(&f->follow, &f->first);
            expr_follow(f, g, mudou);
            break;
        default:
            break;
    }
}

void conj_follow(Gramatica *g)
{
    int i, mudou;
    bitset_add(&g->nts[g->inicial].follow, TERM_FIM); /* FOLLOW(inicial) contem $ */
    do 
    {
        mudou = 0;
        for (i = g->nnts - 1; i >= 0; i--)
        {
            if (g->nts[i].regra != NULL)
            {
            mudou |= bitset_unir(&g->nts[i].regra->follow, &g->nts[i].follow);
            expr_follow(g->nts[i].regra, g, &mudou);

            }
        }
    } while (mudou);

}
