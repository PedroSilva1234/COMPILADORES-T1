/*
 * ll1.c - Verificacao da condicao LL(1) (etapa 4).
 *
 * Condicoes (secao 3 do enunciado):
 *   1. Em a1 | a2 | ... | an, os FIRST das alternativas sao disjuntos dois
 *      a dois e no maximo uma alternativa e anulavel.
 *   2. Se uma alternativa ai e anulavel, FOLLOW(alternativa) e disjunto
 *      dos FIRST das demais.
 *   3. Em [ a ] e { a }, a nao e anulavel e FIRST(a) e disjunto do
 *      FOLLOW da construcao.
 *
 * Violacoes de 1, 2 e da primeira parte de 3 sao ERROS. A violacao de
 * FIRST(a) inter FOLLOW nao vazio em [ a ] ou { a } e reportada como
 * AVISO e resolvida de forma gulosa: o analisador gerado entra na
 * construcao sempre que o token estiver em FIRST(a). E o caso classico
 * do "else pendente":  if = "if" "(" expr ")" cmd [ "else" cmd ] ;
 * em que o else fica com o if mais proximo, como em C.
 *
 * Fornecido: reportar (formata as mensagens).
 * TODO(aluno): verificar_expr e ll1_verificar.
 */
#include <stdarg.h>
#include <stdio.h>
#include "ll1.h"
#include "util.h"

/* Reporta um conflito na regra do nao terminal nt, na posicao de e.
 * comum (pode ser NULL) e o conjunto de tokens em conflito. */
static void reportar(const Gramatica *g, int nt, const Expr *e, int eh_aviso,
                     const Bitset *comum, const char *fmt, ...)
{
    va_list ap;
    fprintf(stderr, "%s:%d:%d: %s: conflito LL(1) em %s: ", g->arquivo, e->linha,
            e->coluna, eh_aviso ? "aviso" : "erro", g->nts[nt].nome);
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    if (comum != NULL) {
        fprintf(stderr, " ");
        gram_imprimir_conjunto(stderr, g, comum);
    }
    fputc('\n', stderr);
}

static void verificar_expr(const Gramatica *g, int nt, const Expr *e, int *erros, Bitset *comum)
{
    int i,j;
    if(e==NULL)
        return;
    /* Visita todos os filhos recursivamente ANTES de testar o no atual */
    for (i = 0; i < e->nfilhos; i++) {
        verificar_expr(g, nt, e->filhos[i], erros, comum);
    }
    switch(e->tipo)
    {
        case E_SEQ:
            for(i = 0; i < e->nfilhos; i++)
                verificar_expr(g, nt, e->filhos[i], erros, comum);
            break;
        case E_ALT:
        /* Compara todas as alternativas duas a duas */
            for(i = 0; i < e->nfilhos; i++)
            {
                for(j = i+1; j < e->nfilhos; j++)
                {
                    /* Condicao 1a: FIRSTs disjuntos */
                    bitset_intersecao(comum, &e->filhos[i]->first, &e->filhos[j]->first);
                    
                    
                    if(!bitset_vazio(comum))
                    {
                        reportar(g, nt, e, 0, comum,
                                  "alternativas %d e %d comecam com", i+1, j+1);
                        (*erros)++;
                    }

                    /* Condicao 1b: No maximo uma alternativa anulavel */
                    if(e->filhos[i]->anulavel && e->filhos[j]->anulavel)
                    {
                        reportar(g, nt, e, 0, NULL,
                                  "alternativas %d e %d sao anulaveis", i+1, j+1);
                        (*erros)++;
                    }

                    /*Condicao 2: Se uma eh anulavel */
                    if(e->filhos[i]->anulavel)
                    {
                        bitset_intersecao(comum, &e->follow, &e->filhos[j]->first);
                        if(!bitset_vazio(comum))
                        {
                            reportar(g, nt, e, 0, comum,
                                      "alternativa %d e anulavel, mas a alternativa %d colide com o FOLLOW da construcao:",
                                      i+1, j+1);
                            (*erros)++;
                        }
                    }

                    if(e->filhos[j]->anulavel)
                    {
                        bitset_intersecao(comum, &e->follow, &e->filhos[i]->first);
                        if(!bitset_vazio(comum))
                        {
                            reportar(g, nt, e, 0, comum,
                                      "alternativa %d e anulavel, mas a alternativa %d colide com o FOLLOW da construcao:",
                                      j+1, i+1);
                            (*erros)++;
                        }
                    }

                }
            }
            break;
            
        case E_OPC:
        case E_REP:
        /* Condicao 3a: O filho interno nao pode ser anulavel (ERRO) */
            if(e->filhos[0]->anulavel)
            {
                reportar(g, nt, e, 0, NULL,
                          "conteudo de %s e anulavel",
                          e->tipo == E_OPC ? "[ ... ]" : "{ ... }");
                (*erros)++;
            }
            /* Condicao 3b: FIRST do filho disjunto do FOLLOW da construcao (AVISO) */
            bitset_intersecao(comum, &e->follow, &e->filhos[0]->first);
            if(!bitset_vazio(comum))
            {
                reportar(g, nt, e, 1, comum,
                          "conteudo de %s comeca com token que tambem esta em seu FOLLOW",
                          e->tipo == E_OPC ? "[ ... ]" : "{ ... }");
            }
            break;
        default: /* E_TERM, E_NAOTERM, E_VAZIO */
            break;
    }
}

/* TODO(aluno): percorrer as regras e, para cada no E_ALT, E_OPC e E_REP,
 * verificar as condicoes do cabecalho. Use bitset_intersecao/bitset_vazio
 * e reporte com reportar(...), por exemplo:
 *
 *   reportar(g, nt, e, 0, &comum, "alternativas %d e %d comecam com", i+1, j+1);
 *
 * que imprime:
 *   expr.ebnf:7:10: erro: conflito LL(1) em stmt: alternativas 1 e 2 comecam com {ID}
 *
 * Retorne o numero de ERROS (avisos nao contam). */
int ll1_verificar(const Gramatica *g)
{
    int i,erros = 0;
    Bitset comum;
    bitset_iniciar(&comum, g->nterms);
    for(i = 0; i < g->nnts; i++)
    {
        if(g->nts[i].regra != NULL)
        {
            verificar_expr(g, i, g->nts[i].regra, &erros, &comum);
        }
    }
    bitset_liberar(&comum);
    return erros;

}
