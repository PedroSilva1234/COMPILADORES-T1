/*
 * microc_mapa.c - Correspondencia com os tokens de Micro C (ver microc_mapa.h).
 *
 * A tabela de literais segue o tokens.h do livro-texto (secao 2.3.1):
 *
 *   UNDEF, ID, END_OF_FILE, INTEGERCONST, CHARCONST, STRINGCONST,
 *   PLUS, MINUS, MUL, DIV, MOD, EQ, NEQ, LT, GT, LEQ, GEQ, AND, OR, NOT,
 *   ASSIGN, SEMICOLON, COMMA, LPAREN, RPAREN, LBRACE, RBRACE,
 *   LBRACKET, RBRACKET, MAIN, IF, ELSE, FOR, RETURN, INT, CHAR, PRINT
 *
 * Se o seu tokens.h do TP1 usar outros nomes, ajuste a tabela.
 *
 * DONE: implementar microc_carregar_tokens_h.
 */
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microc_mapa.h"
#include "util.h"

static const char *const literais[][2] = {
    { "+",  "PLUS" },      { "-",  "MINUS" },     { "*",  "MUL" },
    { "/",  "DIV" },       { "%",  "MOD" },
    { "==", "EQ" },        { "!=", "NEQ" },       { "<",  "LT" },
    { ">",  "GT" },        { "<=", "LEQ" },       { ">=", "GEQ" },
    { "&&", "AND" },       { "||", "OR" },        { "!",  "NOT" },
    { "=",  "ASSIGN" },    { ";",  "SEMICOLON" }, { ",",  "COMMA" },
    { "(",  "LPAREN" },    { ")",  "RPAREN" },    { "{",  "LBRACE" },
    { "}",  "RBRACE" },    { "[",  "LBRACKET" },  { "]",  "RBRACKET" },
    { "main", "MAIN" },    { "if", "IF" },        { "else", "ELSE" },
    { "for", "FOR" },      { "return", "RETURN" },{ "int", "INT" },
    { "char", "CHAR" },    { "print", "PRINT" }
};

static char **nomes = NULL;
static int nnomes = 0, capnomes = 0;

const char *microc_nome_literal(const char *literal)
{
    size_t i;
    for (i = 0; i < sizeof(literais) / sizeof(literais[0]); i++)
        if (strcmp(literais[i][0], literal) == 0)
            return literais[i][1];
    return NULL;
}

static void adicionar_nome(const char *s, size_t n)
{
    if (nnomes == capnomes) {
        capnomes = capnomes ? 2 * capnomes : 64;
        nomes = (char **) xrealloc(nomes, (size_t) capnomes * sizeof(char *));
    }
    nomes[nnomes++] = xstrndup(s, n);
}

/* DONE: abrir o arquivo (ler_arquivo ajuda), localizar a
 * enumeracao "typedef enum { ... } TokenType;" e chamar adicionar_nome
 * para cada identificador da lista, ignorando comentarios e valores
 * explicitos ("X = 3"). Retorne 0 em caso de sucesso.
 * Enquanto nao implementada, a verificacao dos nomes e pulada. */
int microc_carregar_tokens_h(const char *caminho)
{
    char *fonte;
    char *p;
    int dentro_enum = 0;
    char *inicio;

    if (caminho == NULL) caminho = "tokens.h";

    /* A funcao ler_arquivo vem do util.h fornecido no esqueleto */
    fonte = ler_arquivo(caminho);
    if (fonte == NULL) {
        diag_erro(0, 0, "nao foi possivel ler o arquivo de tokens: %s", caminho);
        return -1;
    }

    /* Procura a palavra-chave "enum" para iniciar a varredura com seguranca */
    p = strstr(fonte, "enum");
    if (p == NULL) {
        diag_erro(0, 0, "nao foi possivel encontrar 'enum' em %s", caminho);
        free(fonte);
        return -1;
    }

    /* Varredura manual (mini-scanner) para extrair os tokens do header C */
    while (*p != '\0') {
        /* Pula espacos em branco */
        if (isspace((unsigned char)*p)) {
            p++;
            continue;
        }
        
        /* Pula comentarios de bloco */
        if (p[0] == '/' && p[1] == '*') {
            p += 2;
            while (*p != '\0' && !(p[0] == '*' && p[1] == '/')) p++;
            if (*p != '\0') p += 2;
            continue;
        }
        
        /* Pula comentarios de linha (// ...) */
        if (p[0] == '/' && p[1] == '/') {
            p += 2;
            while (*p != '\0' && *p != '\n') p++;
            continue;
        }

        /* Logica principal de extracao */
        if (!dentro_enum) {
            if (*p == '{') {
                dentro_enum = 1;
            }
            p++;
        } else {
            if (*p == '}') {
                break; /* Fim da enumeracao atingido */
            }
            
            /* Pula valores explicitos (como = 0 ou = 3) se existirem */
            if (*p == '=') {
                while (*p != '\0' && *p != ',' && *p != '}') p++;
                continue; 
            }
            
            /* Pula as virgulas separadoras */
            if (*p == ',') {
                p++;
                continue;
            }
            
            /* Se for uma letra, encontramos o nome de um token! */
            if (isalpha((unsigned char)*p) || *p == '_') {
                inicio = p;
                while (isalnum((unsigned char)*p) || *p == '_') p++;
                
                /* Chama a funcao interna para registrar o token */
                adicionar_nome(inicio, p - inicio);
            } else {
                p++;
            }
        }
    }
    
    free(fonte);
    return 0;
}

int microc_token_existe(const char *nome)
{
    int i;
    for (i = 0; i < nnomes; i++)
        if (strcmp(nomes[i], nome) == 0)
            return 1;
    return 0;
}

void microc_liberar(void)
{
    int i;
    for (i = 0; i < nnomes; i++)
        free(nomes[i]);
    free(nomes);
    nomes = NULL;
    nnomes = capnomes = 0;
}
