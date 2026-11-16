/*
 * verificacao.c - Verificacoes semanticas da gramatica (etapa 2).
 *
 * Ja implementados (use como modelo):
 *   - verif_indefinidos: nao terminais usados mas nao definidos;
 *   - verif_terminais:   terminais nomeados sem declaracao.
 *
 * TODO(aluno):
 *   - verif_inalcancaveis      (aviso)
 *   - verif_improdutivos       (erro)
 *   - verif_recursao_esquerda  (erro; direta e indireta)
 *
 * A duplicidade de definicoes ja e detectada durante a leitura
 * (ebnf_parser.c).
 */
#include <stdlib.h>
#include <string.h>
#include "verificacao.h"
#include "microc_mapa.h"
#include "util.h"

/* ------------------------------------------------------------------ */
/* Fornecido                                                          */
/* ------------------------------------------------------------------ */

static int verif_indefinidos(Gramatica *g)
{
    int i, erros = 0;
    for (i = 0; i < g->nnts; i++) {
        if (g->nts[i].regra == NULL) {
            diag_erro(g->nts[i].linha, g->nts[i].coluna,
                      "nao terminal %s usado mas nao definido", g->nts[i].nome);
            erros++;
        }
    }
    return erros;
}

static int verif_terminais(Gramatica *g, int modo_microc)
{
    int i, erros = 0, carregado = 0;
    Terminal *t;

    if (modo_microc)
        carregado = microc_carregar_tokens_h(g->tokens_externos) == 0;

    for (i = 1; i < g->nterms; i++) {
        t = &g->terms[i];
        if (modo_microc) {
            if (t->literal && microc_nome_literal(t->nome) == NULL) {
                diag_erro(t->linha, t->coluna, "literal %s nao corresponde a nenhum "
                          "token de Micro C", t->exibicao);
                erros++;
            } else if (!t->literal && carregado && !microc_token_existe(t->nome)) {
                diag_erro(t->linha, t->coluna, "terminal %s nao existe na enumeracao "
                          "TokenType de %s", t->nome, g->tokens_externos);
                erros++;
            }
        } else if (!t->declarado) {
            diag_erro(t->linha, t->coluna, "terminal %s nao declarado em TOKENS", t->nome);
            erros++;
        }
    }
    return erros;
}

/* ------------------------------------------------------------------ */
/* A implementar                                                      */
/* ------------------------------------------------------------------ */

/* TODO(aluno): emitir um AVISO para cada nao terminal que nao pode ser
 * alcancado a partir do simbolo inicial (g->inicial). Sugestao: marque
 * o inicial e, ate um ponto fixo, marque todo nao terminal que aparece
 * na regra de um nao terminal ja marcado (expr_percorrer ajuda). */

typedef struct {
    char *alc; /* alc[i] = 1 se o nao terminal i e alcancavel */
    int mudou;
} ChecaAlc;

/*Função que marca os não terminais alcancáveis*/
static void  marca_alcancaveis(Expr *e, void *ctx)
{
    ChecaAlc *Ac = (ChecaAlc *) ctx;
    if(e->tipo == E_NAOTERM && !Ac->alc[e->simbolo]) {
        Ac->alc[e->simbolo] = 1;
        Ac->mudou = 1;
    }

}


 static int verif_inalcancaveis(Gramatica *g)
{
    ChecaAlc Ac;
    int i;
    Ac.alc = (char *) xcalloc((size_t) g->nnts, 1);
    Ac.alc[g->inicial] = 1;

    do {
        Ac.mudou = 0;
        for(i = 0; i < g->nnts; i++) {
            if(Ac.alc[i] && g->nts[i].regra != NULL) {
                expr_percorrer(g->nts[i].regra, marca_alcancaveis, &Ac);
            }
        }
    } while(Ac.mudou);

    for(i =0; i<g->nnts; i++)
    {
        if(!Ac.alc[i]) {
            diag_aviso(g->nts[i].linha, g->nts[i].coluna,
           "nao terminal %s sobra inalcancavel a partir de %s",
           g->nts[i].nome, g->nts[g->inicial].nome);
        }

    }
    free(Ac.alc);
    return 0;
}

/* TODO(aluno): emitir um ERRO para cada nao terminal definido que nao
 * deriva nenhuma cadeia de terminais. Ponto fixo: um nao terminal e
 * produtivo se sua regra for produtiva, onde terminais, E_VAZIO, [..] e
 * {..} sao produtivos, uma sequencia exige todos os filhos produtivos
 * e uma alternativa exige ao menos um. Retorne o numero de erros. */


static int produtivo(Expr *e, const char* prod)
{
    int i;
    switch(e->tipo)
    {
        case E_NAOTERM:
            return prod[e->simbolo];
        case E_SEQ:
            for(i = 0; i < e->nfilhos; i++)
                if(!produtivo(e->filhos[i], prod))
                    return 0; /* Se um filho é improdutivo, a sequência falha */
            return 1;
        case E_ALT:
            for(i =0; i<e->nfilhos; i++)
                if(produtivo(e->filhos[i], prod))
                    return 1;
            return 0;
        default: /* E_TERM, E_VAZIO, E_OPC, E_REP */
            return 1;
    }

}

 static int verif_improdutivos(Gramatica *g)
{
    char *prod = (char *) xcalloc((size_t) g->nnts, 1);
    int i, mudou,erros = 0;

    do {
        mudou = 0;
        for(i = 0; i < g->nnts; i++) {
            if(!prod[i] && g->nts[i].regra != NULL) {
                if(produtivo(g->nts[i].regra, prod)) {
                    prod[i] = 1;
                    mudou = 1;
                }
            }
        }
    } while(mudou);

    for(i = 0;i<g->nnts;i++)
    {
        if(!prod[i]) {
            diag_erro(g->nts[i].linha, g->nts[i].coluna,
           "nao terminal %s nao deriva nenhuma cadeia de terminais",
           g->nts[i].nome);
            erros++;
        }
    }

    free(prod);
    return erros;
}

/* TODO(aluno): detectar recursao a esquerda direta (a = a ...) e
 * indireta (a = b ... ; b = a ...), inclusive atraves de prefixos
 * anulaveis (a = [ x ] a ...). Use o campo anulavel de Expr, ja
 * calculado quando esta funcao e chamada. Reporte um erro por nao
 * terminal recursivo e retorne o numero de erros. */


 static void canto_esquerdo(const Expr *e, char *lc)
 {

    int i;
    switch(e->tipo){
    case E_NAOTERM:
        lc[e->simbolo] = 1;
        break;
    case E_SEQ:
        for(i = 0; i < e->nfilhos; i++)
        {
            canto_esquerdo(e->filhos[i], lc);
            if(!e->filhos[i]->anulavel)
                break;
        }
        break;
        case E_ALT:
            for(i = 0; i < e->nfilhos; i++)
                canto_esquerdo(e->filhos[i], lc);
            break;
        case E_OPC:
        case E_REP:
            canto_esquerdo(e->filhos[0], lc);
            break;
        default: /* E_TERM, E_VAZIO */
            break;

    }

 }

 int verif_recursao_esquerda(Gramatica *g)
{
    int i, j, k, erros = 0;
    size_t n = (size_t) g->nnts;

    char *lc = (char *) xcalloc(n * n, 1);
    char *direta = (char *) xcalloc(n, 1);

    /* 1. Preenche dependencias diretas iniciais */
    for (i = 0; i < g->nnts; i++) {
        if (g->nts[i].regra != NULL)
            canto_esquerdo(g->nts[i].regra, lc + (size_t)i * n);
    }

    /* 2. Verifica recursao direta ANTES do Warshall */
    for (i = 0; i < g->nnts; i++) {
        if (lc[(size_t)i * n + (size_t)i]) {
            direta[i] = 1;
        }
    }

    /* 3. Algoritmo de Warshall (Fecho Transitivo) */
    /* IMPORTANTE: k DEVE ser o laco mais externo! */
    for (k = 0; k < g->nnts; k++) {
        for (i = 0; i < g->nnts; i++) {
            if (lc[(size_t)i * n + (size_t)k]) {
                for (j = 0; j < g->nnts; j++) {
                    if (lc[(size_t)k * n + (size_t)j]) {
                        lc[(size_t)i * n + (size_t)j] = 1;
                    }
                }
            }
        }
    }

    /* 4. Emite erros para qualquer recursao (direta ou indireta) */
    for (i = 0; i < g->nnts; i++) {
        if (lc[(size_t)i * n + (size_t)i]) {
            diag_erro(g->nts[i].linha_def, 0,
                      "nao terminal %s tem recursao a esquerda %s",
                      g->nts[i].nome, direta[i] ? "direta" : "indireta");
            erros++;
        }
    }
    
    free(lc);
    free(direta);
    return erros;
}

int verif_basica(Gramatica *g, int modo_microc)
{
    int erros = 0;
    if (g->inicial < 0) {
        diag_erro(0, 0, "a gramatica nao tem nenhuma regra");
        return 1;
    }
    erros += verif_indefinidos(g);
    erros += verif_terminais(g, modo_microc);
    erros += verif_inalcancaveis(g);
    if (erros == 0)
        erros += verif_improdutivos(g);
    return erros;
}
