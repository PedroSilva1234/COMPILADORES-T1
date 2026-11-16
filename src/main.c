/*
 * main.c - Ponto de entrada do gerador de analisadores sintaticos.
 *
 *   gerador gramatica.ebnf -o parser [--scanner microc] [--sets] [--tree]
 *
 * Executa as cinco etapas do enunciado, nesta ordem, parando na primeira
 * que encontrar erros:
 *   1. leitura da gramatica          (ebnf_parser.c)
 *   2. verificacao semantica         (verificacao.c)
 *   3. anulavel, FIRST e FOLLOW      (conjuntos.c)
 *      + recursao a esquerda         (verificacao.c, precisa de anulavel)
 *   4. verificacao LL(1)             (ll1.c)
 *   5. geracao de codigo             (codegen.c)
 *
 * Modulo FORNECIDO: nao e necessario altera-lo.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "conjuntos.h"
#include "ebnf_parser.h"
#include "gramatica.h"
#include "ll1.h"
#include "microc_mapa.h"
#include "util.h"
#include "verificacao.h"

static void uso(const char *prog)
{
    fprintf(stderr,
            "uso: %s gramatica.ebnf -o prefixo [--scanner microc] [--sets] [--tree] "
            "[--debug]\n"
            "  -o prefixo         gera prefixo.h e prefixo.c\n"
            "  --scanner microc   usa o scanner do TP1 (TOKENS EXTERN \"tokens.h\")\n"
            "  --sets             imprime anulavel, FIRST e FOLLOW\n"
            "  --tree             o analisador gerado aceita --tree (arvore de derivacao)\n"
            "  --debug            imprime a gramatica lida\n",
            prog);
}

/* Caminho de TOKENS EXTERN relativo ao diretorio do arquivo .ebnf. */
static void resolver_tokens_externos(Gramatica *g, const char *arquivo)
{
    const char *barra = strrchr(arquivo, '/');
    char *novo;
    size_t n;
    if (g->tokens_externos == NULL || g->tokens_externos[0] == '/' || barra == NULL)
        return;
    n = (size_t) (barra - arquivo) + 1;
    novo = (char *) xmalloc(n + strlen(g->tokens_externos) + 1);
    memcpy(novo, arquivo, n);
    strcpy(novo + n, g->tokens_externos);
    free(g->tokens_externos);
    g->tokens_externos = novo;
}

int main(int argc, char **argv)
{
    const char *arquivo = NULL, *prefixo = NULL;
    int i, microc = 0, sets = 0, arvore = 0, debug = 0, status = EXIT_FAILURE;
    char *fonte;
    Gramatica *g;
    OpcoesGeracao op;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
            prefixo = argv[++i];
        } else if (strcmp(argv[i], "--scanner") == 0 && i + 1 < argc) {
            if (strcmp(argv[++i], "microc") != 0) {
                fprintf(stderr, "gerador: scanner desconhecido '%s' (use microc)\n", argv[i]);
                return EXIT_FAILURE;
            }
            microc = 1;
        } else if (strcmp(argv[i], "--sets") == 0) {
            sets = 1;
        } else if (strcmp(argv[i], "--tree") == 0) {
            arvore = 1;
        } else if (strcmp(argv[i], "--debug") == 0) {
            debug = 1;
        } else if (argv[i][0] == '-') {
            uso(argv[0]);
            return EXIT_FAILURE;
        } else {
            arquivo = argv[i];
        }
    }
    if (arquivo == NULL || (prefixo == NULL && !sets && !debug)) {
        uso(argv[0]);
        return EXIT_FAILURE;
    }

    fonte = ler_arquivo(arquivo);
    if (fonte == NULL) {
        fprintf(stderr, "gerador: nao foi possivel abrir %s\n", arquivo);
        return EXIT_FAILURE;
    }
    diag_definir_arquivo(arquivo);
    g = gram_criar(arquivo);

    /* 1. leitura */
    if (!ebnf_analisar(g, fonte))
        goto fim;
    resolver_tokens_externos(g, arquivo);
    if (debug)
        gram_imprimir(stdout, g);
    if (microc && g->tokens_externos == NULL) {
        diag_erro(0, 0, "--scanner microc exige a secao TOKENS EXTERN \"tokens.h\" ;");
        goto fim;
    }
    if (!microc && g->tokens_externos != NULL) {
        diag_erro(0, 0, "TOKENS EXTERN exige a opcao --scanner microc");
        goto fim;
    }

    /* 2. verificacao semantica */
    if (verif_basica(g, microc) > 0)
        goto fim;

    /* 3. conjuntos (+ recursao a esquerda, que depende de anulavel) */
    conj_iniciar(g);
    conj_anulavel(g);
    if (verif_recursao_esquerda(g) > 0)
        goto fim;
    conj_first(g);
    conj_follow(g);
    if (sets) {
        conj_imprimir(stdout, g);
        fflush(stdout);
    }

    /* 4. LL(1) */
    if (ll1_verificar(g) > 0) {
        fprintf(stderr, "%s: a gramatica nao e LL(1); nenhum codigo foi gerado\n", arquivo);
        goto fim;
    }

    /* 5. geracao */
    if (prefixo != NULL) {
        op.microc = microc;
        op.arvore = arvore;
        op.prefixo = prefixo;
        op.origem = arquivo;
        if (codegen_gerar(g, &op) != 0)
            goto fim;
    }
    status = EXIT_SUCCESS;

fim:
    gram_liberar(g);
    microc_liberar();
    free(fonte);
    return status;
}
