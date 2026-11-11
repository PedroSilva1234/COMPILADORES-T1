/*
 * Analisador sintatico descendente recursivo gerado a partir de testes/gramaticas/expr.ebnf.
 * Arquivo gerado automaticamente: nao edite.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "expr.h"

#define SUPORTE_ARVORE 0

/* ---- conjuntos de tokens ---- */


/* ---- estado do analisador ---- */

static Token tok;                  /* token corrente (lookahead) */
static int imprimir_arvore = 0;    /* opcao --tree */
static int nivel_arvore = 0;
static FILE *arquivo_tokens = NULL; /* opcao --tokens */

static Token token_de_arquivo(void);

/* ---- analisador lexico gerado a partir de TOKENS ---- */

static char *fonte = NULL;
static size_t pos = 0;
static int linha = 1, coluna = 1;

static char *ler_tudo(FILE *f)
{
    size_t cap = 4096, n = 0, lidos;
    char *buf = (char *) malloc(cap), *novo;
    if (buf == NULL) {
        fprintf(stderr, "memoria insuficiente\n");
        exit(EXIT_FAILURE);
    }
    while ((lidos = fread(buf + n, 1, cap - n - 1, f)) > 0) {
        n += lidos;
        if (cap - n - 1 == 0) {
            cap *= 2;
            novo = (char *) realloc(buf, cap);
            if (novo == NULL) {
                fprintf(stderr, "memoria insuficiente\n");
                exit(EXIT_FAILURE);
            }
            buf = novo;
        }
    }
    buf[n] = '\0';
    return buf;
}

static void consumir(size_t n)
{
    for (; n > 0 && fonte[pos] != '\0'; n--, pos++) {
        if (fonte[pos] == '\n') {
            linha++;
            coluna = 1;
        } else {
            coluna++;
        }
    }
}

/* SKIP: [ \t\r\n]+ */
static size_t casa_skip_0(const char *s)
{
    /* TODO(aluno): automato para [ \t\r\n]+ */
    (void) s;
    return 0;
}

/* NUM: [0-9]+ */
static size_t casa_T_NUM(const char *s)
{
    /* TODO(aluno): automato para [0-9]+ */
    (void) s;
    return 0;
}

/* ID: [a-zA-Z_][a-zA-Z0-9_]* */
static size_t casa_T_ID(const char *s)
{
    /* TODO(aluno): automato para [a-zA-Z_][a-zA-Z0-9_]* */
    (void) s;
    return 0;
}

static const char *const texto_literais[] = {
    "+",
    "-",
    "*",
    "/",
    "(",
    ")",
    NULL
};

static const int tipo_literais[] = {
    T_MAIS,
    T_MENOS,
    T_VEZES,
    T_BARRA,
    T_ABRE_PAR,
    T_FECHA_PAR,
    -1
};

/* Reconhece o maior prefixo da entrada; em caso de empate vence
 * o literal (palavras reservadas tem prioridade sobre ID). */
static Token proximo_token_gerado(void)
{
    Token t;
    size_t n, melhor;
    int i, tipo, pulou;

    do {
        pulou = 0;
        if ((n = casa_skip_0(fonte + pos)) > 0) {
            consumir(n);
            pulou = 1;
        }
    } while (pulou);

    memset(&t, 0, sizeof t);
    t.linha = linha;
    t.coluna = coluna;
    if (fonte[pos] == '\0') {
        t.tipo = T_FIM;
        return t;
    }
    melhor = 0;
    tipo = -1;
    for (i = 0; texto_literais[i] != NULL; i++) {
        n = strlen(texto_literais[i]);
        if (n > melhor && strncmp(fonte + pos, texto_literais[i], n) == 0) {
            melhor = n;
            tipo = tipo_literais[i];
        }
    }
    if ((n = casa_T_NUM(fonte + pos)) > melhor) {
        melhor = n;
        tipo = T_NUM;
    }
    if ((n = casa_T_ID(fonte + pos)) > melhor) {
        melhor = n;
        tipo = T_ID;
    }
    if (melhor == 0) {
        fprintf(stderr, "linha %d, coluna %d: erro lexico: caractere invalido '%c'\n",
                linha, coluna, fonte[pos]);
        exit(EXIT_FAILURE);
    }
    n = melhor < sizeof t.lexema ? melhor : sizeof t.lexema - 1;
    memcpy(t.lexema, fonte + pos, n);
    t.lexema[n] = '\0';
    t.tipo = tipo;
    consumir(melhor);
    return t;
}

const char *nome_token(int tipo)
{
    switch (tipo) {
    case T_FIM: return "fim de arquivo";
    case T_NUM: return "NUM";
    case T_ID: return "ID";
    case T_MAIS: return "'+'";
    case T_MENOS: return "'-'";
    case T_VEZES: return "'*'";
    case T_BARRA: return "'/'";
    case T_ABRE_PAR: return "'('";
    case T_FECHA_PAR: return "')'";
    default: return "token desconhecido";
    }
}

static void escrever_posicao(void)
{
    fprintf(stderr, "linha %d, coluna %d: ", tok.linha, tok.coluna);
}

/* Le o proximo token para tok. */
void avancar(void)
{
    if (arquivo_tokens != NULL)
        tok = token_de_arquivo();
    else
        tok = proximo_token_gerado();
}

/* ---- primitivas ---- */

int pertence(int tipo, const int *conjunto)
{
    for (; *conjunto != -1; conjunto++)
        if (*conjunto == tipo)
            return 1;
    return 0;
}

/* Reporta o erro com os tokens esperados (lista terminada em -1) e encerra. */
void erro_sintatico(const int *esperados)
{
    int i;
    escrever_posicao();
    if ((int) tok.tipo == TOKEN_FIM)
        fprintf(stderr, "encontrado fim de arquivo, ");
    else
        fprintf(stderr, "encontrado '%s', ", tok.lexema);
    if (esperados[0] != -1 && esperados[1] == -1) {
        fprintf(stderr, "esperado %s\n", nome_token(esperados[0]));
    } else {
        fprintf(stderr, "esperado um de {");
        for (i = 0; esperados[i] != -1; i++)
            fprintf(stderr, "%s%s", i > 0 ? ", " : "", nome_token(esperados[i]));
        fprintf(stderr, "}\n");
    }
    exit(EXIT_FAILURE);
}

static void recuo(void)
{
    int i;
    for (i = 0; i < nivel_arvore; i++)
        fputs("  ", stdout);
}

void arvore_entra(const char *nao_terminal)
{
    if (imprimir_arvore) {
        recuo();
        printf("%s\n", nao_terminal);
    }
    nivel_arvore++;
}

void arvore_sai(void)
{
    nivel_arvore--;
}

/* Consome o token corrente se for do tipo esperado; senao, erro. */
void match(int esperado)
{
    int lista[2];
    if ((int) tok.tipo != esperado) {
        lista[0] = esperado;
        lista[1] = -1;
        erro_sintatico(lista);
    }
    if (SUPORTE_ARVORE && imprimir_arvore) {
        recuo();
        if (nome_token(tok.tipo)[0] == '\'')
            printf("%s\n", nome_token(tok.tipo));
        else
            printf("%s '%s'\n", nome_token(tok.tipo), tok.lexema);
    }
    avancar();
}

static Token token_de_arquivo(void)
{
    Token t;
    memset(&t, 0, sizeof t);
    fprintf(stderr, "opcao --tokens ainda nao implementada\n");
    exit(EXIT_FAILURE);
    return t;
}

/* ---- procedimentos (um por nao terminal) ---- */

/* expr = term { ( '+' | '-' ) term } ; */
void parse_expr(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* term = factor { ( '*' | '/' ) factor } ; */
void parse_term(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* factor = NUM | ID | '(' expr ')' ; */
void parse_factor(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* ---- programa principal ---- */

int main(int argc, char **argv)
{
    int i, fim[2];
    const char *arquivo = NULL, *tokens = NULL;
    FILE *entrada = stdin;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tree") == 0) {
            imprimir_arvore = 1;
        } else if (strcmp(argv[i], "--tokens") == 0 && i + 1 < argc) {
            tokens = argv[++i];
        } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            fprintf(stderr, "uso: %s [--tree] [--tokens arquivo.tok | programa]\n", argv[0]);
            return 2;
        } else {
            arquivo = argv[i];
        }
    }
    if (imprimir_arvore && !SUPORTE_ARVORE)
        fprintf(stderr, "aviso: gere o analisador com --tree para imprimir a arvore\n");

    if (tokens != NULL) {
        arquivo_tokens = fopen(tokens, "r");
        if (arquivo_tokens == NULL) {
            fprintf(stderr, "nao foi possivel abrir %s\n", tokens);
            return 2;
        }
    } else {
        if (arquivo != NULL && (entrada = fopen(arquivo, "r")) == NULL) {
            fprintf(stderr, "nao foi possivel abrir %s\n", arquivo);
            return 2;
        }
        fonte = ler_tudo(entrada);
    }

    avancar();
    parse_expr();
    if (tok.tipo != TOKEN_FIM) {
        fim[0] = TOKEN_FIM;
        fim[1] = -1;
        erro_sintatico(fim);
    }
    printf("entrada aceita\n");

    if (arquivo_tokens != NULL) {
        fclose(arquivo_tokens);
    } else {
        free(fonte);
        if (entrada != stdin)
            fclose(entrada);
    }
    return 0;
}
