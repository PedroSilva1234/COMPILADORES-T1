/*
 * ebnf_lexer.c - Analisador lexico da notacao EBNF (ver ebnf_lexer.h).
 *
 * Modulo FORNECIDO: nao e necessario altera-lo.
 */
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include "ebnf_lexer.h"
#include "util.h"

void mlex_iniciar(MetaLexer *lx, const char *fonte)
{
    lx->fonte = fonte;
    lx->pos = 0;
    lx->linha = 1;
    lx->coluna = 1;
}

static int atual(const MetaLexer *lx)
{
    return (unsigned char) lx->fonte[lx->pos];
}

static int seguinte(const MetaLexer *lx)
{
    if (lx->fonte[lx->pos] == '\0')
        return '\0';
    return (unsigned char) lx->fonte[lx->pos + 1];
}

static void avancar(MetaLexer *lx)
{
    if (lx->fonte[lx->pos] == '\0')
        return;
    if (lx->fonte[lx->pos] == '\n') {
        lx->linha++;
        lx->coluna = 1;
    } else {
        lx->coluna++;
    }
    lx->pos++;
}

static MetaToken criar(MetaTipo tipo, char *texto, int linha, int coluna)
{
    MetaToken t;
    t.tipo = tipo;
    t.texto = texto;
    t.linha = linha;
    t.coluna = coluna;
    return t;
}

/* Pula espacos e comentarios (* ... *). Retorna 0 se um comentario
 * nao terminar (o erro ja e reportado). */
static int pular_brancos(MetaLexer *lx)
{
    int l, c;
    for (;;) {
        while (isspace(atual(lx)))
            avancar(lx);
        if (atual(lx) == '(' && seguinte(lx) == '*') {
            l = lx->linha;
            c = lx->coluna;
            avancar(lx);
            avancar(lx);
            while (atual(lx) != '\0' && !(atual(lx) == '*' && seguinte(lx) == ')'))
                avancar(lx);
            if (atual(lx) == '\0') {
                diag_erro(l, c, "comentario nao terminado");
                return 0;
            }
            avancar(lx);
            avancar(lx);
            continue;
        }
        return 1;
    }
}

/* Literal entre aspas: aceita os escapes \\ \" \' \n \t. */
static MetaToken ler_literal(MetaLexer *lx, int l, int c)
{
    int aspa = atual(lx);
    size_t cap = 16, n = 0;
    char *buf = (char *) xmalloc(cap);
    int ch;

    avancar(lx);
    while (atual(lx) != aspa) {
        ch = atual(lx);
        if (ch == '\0' || ch == '\n') {
            diag_erro(l, c, "literal nao terminado");
            free(buf);
            return criar(MT_ERRO, NULL, l, c);
        }
        if (ch == '\\') {
            avancar(lx);
            ch = atual(lx);
            switch (ch) {
            case 'n': ch = '\n'; break;
            case 't': ch = '\t'; break;
            case '\\': case '"': case '\'': break;
            default:
                diag_erro(lx->linha, lx->coluna, "escape invalido '\\%c' em literal", ch);
                break;
            }
        }
        if (n + 1 >= cap) {
            cap *= 2;
            buf = (char *) xrealloc(buf, cap);
        }
        buf[n++] = (char) ch;
        avancar(lx);
    }
    avancar(lx);
    buf[n] = '\0';
    if (n == 0) {
        diag_erro(l, c, "literal vazio");
        free(buf);
        return criar(MT_ERRO, NULL, l, c);
    }
    return criar(MT_LITERAL, buf, l, c);
}

/* Expressao regular entre barras: o conteudo e mantido como esta,
 * inclusive escapes (\/ representa uma barra). */
static MetaToken ler_regex(MetaLexer *lx, int l, int c)
{
    size_t inicio;
    avancar(lx);
    inicio = lx->pos;
    while (atual(lx) != '/') {
        if (atual(lx) == '\0' || atual(lx) == '\n') {
            diag_erro(l, c, "expressao regular nao terminada");
            return criar(MT_ERRO, NULL, l, c);
        }
        if (atual(lx) == '\\' && seguinte(lx) != '\0' && seguinte(lx) != '\n')
            avancar(lx);
        avancar(lx);
    }
    avancar(lx);
    if (lx->pos - 1 == inicio) {
        diag_erro(l, c, "expressao regular vazia");
        return criar(MT_ERRO, NULL, l, c);
    }
    return criar(MT_REGEX, xstrndup(lx->fonte + inicio, lx->pos - 1 - inicio), l, c);
}

static MetaToken ler_identificador(MetaLexer *lx, int l, int c)
{
    size_t inicio = lx->pos, i;
    char *texto;
    int maiusculo = 1;

    while (isalnum(atual(lx)) || atual(lx) == '_')
        avancar(lx);
    texto = xstrndup(lx->fonte + inicio, lx->pos - inicio);

    if (strcmp(texto, "TOKENS") == 0) return criar(MT_TOKENS, texto, l, c);
    if (strcmp(texto, "RULES") == 0)  return criar(MT_RULES, texto, l, c);
    if (strcmp(texto, "SKIP") == 0)   return criar(MT_SKIP, texto, l, c);
    if (strcmp(texto, "EXTERN") == 0) return criar(MT_EXTERN, texto, l, c);

    if (islower((unsigned char) texto[0]))
        return criar(MT_NAOTERM, texto, l, c);
    for (i = 0; texto[i] != '\0'; i++)
        if (islower((unsigned char) texto[i]))
            maiusculo = 0;
    if (maiusculo)
        return criar(MT_NOME_TERM, texto, l, c);
    diag_erro(l, c, "identificador '%s' invalido: terminais usam so maiusculas "
              "e nao terminais comecam com minuscula", texto);
    free(texto);
    return criar(MT_ERRO, NULL, l, c);
}

MetaToken mlex_proximo(MetaLexer *lx)
{
    int l, c, ch;
    MetaTipo tipo;

    if (!pular_brancos(lx))
        return criar(MT_FIM, NULL, lx->linha, lx->coluna);
    l = lx->linha;
    c = lx->coluna;
    ch = atual(lx);

    if (ch == '\0')
        return criar(MT_FIM, NULL, l, c);
    if (ch == '"' || ch == '\'')
        return ler_literal(lx, l, c);
    if (ch == '/')
        return ler_regex(lx, l, c);
    if (isalpha(ch) || ch == '_')
        return ler_identificador(lx, l, c);

    switch (ch) {
    case '=': tipo = MT_IGUAL; break;
    case ';': tipo = MT_PV; break;
    case '|': tipo = MT_BARRA; break;
    case '[': tipo = MT_ABRE_COL; break;
    case ']': tipo = MT_FECHA_COL; break;
    case '{': tipo = MT_ABRE_CHAVE; break;
    case '}': tipo = MT_FECHA_CHAVE; break;
    case '(': tipo = MT_ABRE_PAR; break;
    case ')': tipo = MT_FECHA_PAR; break;
    default:
        diag_erro(l, c, "caractere invalido '%c'", ch);
        avancar(lx);
        return criar(MT_ERRO, NULL, l, c);
    }
    avancar(lx);
    return criar(tipo, NULL, l, c);
}

void mtok_liberar(MetaToken *t)
{
    free(t->texto);
    t->texto = NULL;
}

const char *mtipo_nome(MetaTipo t)
{
    switch (t) {
    case MT_FIM:         return "fim de arquivo";
    case MT_ERRO:        return "simbolo invalido";
    case MT_NOME_TERM:   return "nome de terminal";
    case MT_NAOTERM:     return "nao terminal";
    case MT_LITERAL:     return "literal";
    case MT_REGEX:       return "expressao regular";
    case MT_IGUAL:       return "'='";
    case MT_PV:          return "';'";
    case MT_BARRA:       return "'|'";
    case MT_ABRE_COL:    return "'['";
    case MT_FECHA_COL:   return "']'";
    case MT_ABRE_CHAVE:  return "'{'";
    case MT_FECHA_CHAVE: return "'}'";
    case MT_ABRE_PAR:    return "'('";
    case MT_FECHA_PAR:   return "')'";
    case MT_TOKENS:      return "TOKENS";
    case MT_RULES:       return "RULES";
    case MT_SKIP:        return "SKIP";
    case MT_EXTERN:      return "EXTERN";
    }
    return "?";
}
