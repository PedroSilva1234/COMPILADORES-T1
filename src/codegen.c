/*
 * codegen.c - Geracao do analisador sintatico (etapa 5).
 *
 * Estrutura do arquivo .c gerado:
 *
 *   1. includes e conjuntos constantes (conj_0, conj_1, ...)
 *   2. estado e primitivas: avancar, match, erro_sintatico, arvore
 *   3. modo proprio: analisador lexico gerado a partir de TOKENS
 *      modo Micro C: tokens obtidos de proximo_token() (scanner do TP1)
 *   4. leitura de um arquivo de tokens (opcao --tokens do analisador)
 *   5. um procedimento parse_X() por nao terminal
 *   6. main
 *
 * Fornecido: toda a estrutura acima, os nomes dos tokens, os conjuntos,
 * as primitivas, o lexico (exceto o reconhecimento das expressoes
 * regulares) e a traducao de terminais e nao terminais.
 *
 * DONE:
 *   - gen_expr: traducao de sequencia, alternativa, [ ] e { }
 *     (esquema da secao 4 do enunciado);
 *   - gen_casamento_regex: converter uma expressao regular de TOKENS em
 *     codigo C (automato finito) que reconhece o maior prefixo;
 *   - gen_token_de_arquivo: leitura do arquivo de tokens do TP1.
 */
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codegen.h"
#include "microc_mapa.h"
#include "util.h"

/* ------------------------------------------------------------------ */
/* Emissao de texto                                                   */
/* ------------------------------------------------------------------ */

typedef struct {
    FILE *f;
    int nivel;          /* nivel de indentacao (4 espacos por nivel) */
} Emissor;

/* Emite uma linha indentada; fmt segue printf. */
static void emit(Emissor *em, const char *fmt, ...)
{
    int i;
    va_list ap;
    for (i = 0; i < em->nivel; i++)
        fputs("    ", em->f);
    va_start(ap, fmt);
    vfprintf(em->f, fmt, ap);
    va_end(ap);
    fputc('\n', em->f);
}

/* Emite linhas fixas, sem interpretar '%'. A lista termina em NULL. */
static void emit_linhas(FILE *f, const char *const *linhas)
{
    for (; *linhas != NULL; linhas++) {
        fputs(*linhas, f);
        fputc('\n', f);
    }
}

/* Texto dentro de comentario C: evita fechar o comentario por engano. */
static void emitir_comentario(FILE *f, const char *s)
{
    for (; *s != '\0'; s++) {
        if (s[0] == '*' && s[1] == '/') {
            fputs("* /", f);
            s++;
        } else if (*s == '\n') {
            fputc(' ', f);
        } else {
            fputc(*s, f);
        }
    }
}

/* String C entre aspas, com escapes. */
static void emitir_string_c(FILE *f, const char *s)
{
    fputc('"', f);
    for (; *s != '\0'; s++) {
        unsigned char c = (unsigned char) *s;
        if (c == '"' || c == '\\')
            fprintf(f, "\\%c", c);
        else if (c == '\n')
            fputs("\\n", f);
        else if (c == '\t')
            fputs("\\t", f);
        else if (c < 32 || c > 126)
            fprintf(f, "\\%03o", c);
        else
            fputc(c, f);
    }
    fputc('"', f);
}

/* ------------------------------------------------------------------ */
/* Nomes dos tokens no codigo gerado                                  */
/* ------------------------------------------------------------------ */

static const char *const nomes_simbolos[][2] = {
    { "+", "MAIS" }, { "-", "MENOS" }, { "*", "VEZES" }, { "/", "BARRA" },
    { "%", "PORCENTO" }, { "(", "ABRE_PAR" }, { ")", "FECHA_PAR" },
    { "[", "ABRE_COL" }, { "]", "FECHA_COL" }, { "{", "ABRE_CHAVE" },
    { "}", "FECHA_CHAVE" }, { ";", "PV" }, { ",", "VIRGULA" },
    { ".", "PONTO" }, { ":", "DOIS_PONTOS" }, { "=", "IGUAL" },
    { "==", "IGUAL_IGUAL" }, { "!=", "DIFERENTE" }, { "<", "MENOR" },
    { ">", "MAIOR" }, { "<=", "MENOR_IGUAL" }, { ">=", "MAIOR_IGUAL" },
    { "&&", "E_LOGICO" }, { "||", "OU_LOGICO" }, { "!", "NAO" },
    { ":=", "ATRIBUICAO" }, { "->", "SETA" }
};

static int eh_palavra(const char *s)
{
    if (!isalpha((unsigned char) *s) && *s != '_')
        return 0;
    for (; *s != '\0'; s++)
        if (!isalnum((unsigned char) *s) && *s != '_')
            return 0;
    return 1;
}

static int nome_em_uso(const Gramatica *g, int ate, const char *nome)
{
    int i;
    for (i = 0; i < ate; i++)
        if (strcmp(g->terms[i].nome_c, nome) == 0)
            return 1;
    return 0;
}

/* Define g->terms[i].nome_c para todos os terminais. */
static void nomear_terminais(Gramatica *g, int microc)
{
    int i;
    size_t k;
    char buf[256];
    Terminal *t;

    for (i = 0; i < g->nterms; i++) {
        t = &g->terms[i];
        free(t->nome_c);
        if (i == TERM_FIM) {
            t->nome_c = xstrdup(microc ? "END_OF_FILE" : "T_FIM");
            continue;
        }
        if (microc) {
            t->nome_c = xstrdup(t->literal ? microc_nome_literal(t->nome) : t->nome);
            continue;
        }
        buf[0] = '\0';
        if (!t->literal) {
            sprintf(buf, "T_%.200s", t->nome);
        } else if (eh_palavra(t->nome) && strlen(t->nome) < 200) {
            sprintf(buf, "T_%s", t->nome);
            for (k = 2; buf[k] != '\0'; k++)
                buf[k] = (char) toupper((unsigned char) buf[k]);
        } else {
            for (k = 0; k < sizeof(nomes_simbolos) / sizeof(nomes_simbolos[0]); k++)
                if (strcmp(nomes_simbolos[k][0], t->nome) == 0)
                    sprintf(buf, "T_%s", nomes_simbolos[k][1]);
            if (buf[0] == '\0')
                sprintf(buf, "T_LIT_%d", i);
        }
        if (nome_em_uso(g, i, buf))
            sprintf(buf + strlen(buf), "_%d", i);
        t->nome_c = xstrdup(buf);
    }
}

/* ------------------------------------------------------------------ */
/* Conjuntos constantes                                               */
/* ------------------------------------------------------------------ */

#define MAX_COMPARACOES 3   /* ate 3 tokens: tok.tipo == A || ...; acima: pertence */

static Bitset *conjuntos = NULL;
static int nconj = 0, capconj = 0;

static int bitset_igual(const Bitset *a, const Bitset *b)
{
    int i;
    for (i = 0; i < a->n; i++)
        if (bitset_tem(a, i) != bitset_tem(b, i))
            return 0;
    return 1;
}

/* Registra um conjunto e devolve seu numero N (emitido como conj_N).
 * Conjuntos iguais compartilham a mesma constante. */
static int registrar_conjunto(const Bitset *b)
{
    int i;
    for (i = 0; i < nconj; i++)
        if (bitset_igual(&conjuntos[i], b))
            return i;
    if (nconj == capconj) {
        capconj = capconj ? 2 * capconj : 16;
        conjuntos = (Bitset *) xrealloc(conjuntos, (size_t) capconj * sizeof(Bitset));
    }
    bitset_iniciar(&conjuntos[nconj], b->n);
    bitset_copiar(&conjuntos[nconj], b);
    return nconj++;
}

static void liberar_conjuntos(void)
{
    int i;
    for (i = 0; i < nconj; i++)
        bitset_liberar(&conjuntos[i]);
    free(conjuntos);
    conjuntos = NULL;
    nconj = capconj = 0;
}

static void emitir_conjuntos(FILE *f, const Gramatica *g)
{
    int i, t;
    for (i = 0; i < nconj; i++) {
        fprintf(f, "static const int conj_%d[] = { ", i);
        for (t = bitset_proximo(&conjuntos[i], 0); t >= 0;
             t = bitset_proximo(&conjuntos[i], t + 1))
            fprintf(f, "%s, ", g->terms[t].nome_c);
        fputs("-1 };\n", f);
    }
}

/* Escreve a condicao "o token corrente pertence a b":
 *   tok.tipo == T_A                     (um token)
 *   tok.tipo == T_A || tok.tipo == T_B  (ate MAX_COMPARACOES)
 *   pertence(tok.tipo, conj_N)          (mais tokens) */
static void emitir_teste(FILE *f, const Gramatica *g, const Bitset *b)
{
    int t, n = bitset_tamanho(b), primeiro = 1;
    if (n == 0) {
        fputs("0", f);
    } else if (n <= MAX_COMPARACOES) {
        for (t = bitset_proximo(b, 0); t >= 0; t = bitset_proximo(b, t + 1)) {
            fprintf(f, "%stok.tipo == %s", primeiro ? "" : " || ", g->terms[t].nome_c);
            primeiro = 0;
        }
    } else {
        fprintf(f, "pertence(tok.tipo, conj_%d)", registrar_conjunto(b));
    }
}

/* Linha "<antes><teste><depois>", ex.: emit_teste(em, "while (", g, b, ") {") */
static void emit_teste(Emissor *em, const char *antes, const Gramatica *g,
                       const Bitset *b, const char *depois)
{
    int i;
    for (i = 0; i < em->nivel; i++)
        fputs("    ", em->f);
    fputs(antes, em->f);
    emitir_teste(em->f, g, b);
    fputs(depois, em->f);
    fputc('\n', em->f);
}

/* Linha "erro_sintatico(conj_N);" com N = conjunto dos tokens esperados. */
static void emit_erro(Emissor *em, const Bitset *esperados)
{
    emit(em, "erro_sintatico(conj_%d);", registrar_conjunto(esperados));
}

/* ------------------------------------------------------------------ */
/* Traducao das expressoes (esquema da secao 4 do enunciado)          */
/* ------------------------------------------------------------------ */

static void gen_expr(Emissor *em, const Gramatica *g, const Expr *e)
{
    int i, usados;
    const Expr *nula;

    switch (e->tipo) {
    case E_TERM:
        emit(em, "match(%s);", g->terms[e->simbolo].nome_c);
        break;
    case E_NAOTERM:
        emit(em, "parse_%s();", g->nts[e->simbolo].nome);
        break;
    
    case E_SEQ: 
        for(i=0; i<e->nfilhos; i++) {
            gen_expr(em, g, e->filhos[i]);
        }
        break;
    
    case E_OPC:
        emit_teste(em, "if (", g, &e->filhos[0]->first, ") {");
        em->nivel++;
        gen_expr(em, g, e->filhos[0]);
        em->nivel--;
        emit(em, "}");
        break;

    case E_REP: 
            emit_teste(em, "while (", g, &e->filhos[0]->first, ") {");
            em->nivel++;
            gen_expr(em, g, e->filhos[0]);
            em->nivel--;
            emit(em, "}");
            break;
    case E_VAZIO:
        break;
    case E_ALT:
        nula = NULL;
        usados = 0;
        for(i = 0; i < e->nfilhos; i++) {
            if(e->filhos[i]->anulavel) {
                nula = e->filhos[i];
                continue;
            }
            emit_teste(em, usados==0 ? "if (" : "else if (", g, &e->filhos[i]->first, ") {");
            usados++;
            em->nivel++;
            gen_expr(em, g, e->filhos[i]);
            em->nivel--;
            emit(em, "}");
        }

        if (!(nula != NULL && nula->tipo == E_VAZIO)) {
            emit(em, "else {");
            em->nivel++;
            if (nula != NULL)
                gen_expr(em, g, nula);
            else
                emit_erro(em, &e->first);
            em->nivel--;
            emit(em, "}");
        }
        break;

    /*
     * DONE: traduzir as demais construcoes (secao 4):
     *
     *   E_SEQ   a1 a2 ... an   ->  gen_expr de cada filho, em ordem
     *   E_ALT   a1 | ... | an  ->  if (FIRST(a1)) {...} else if ... else erro
     *   E_OPC   [ a ]          ->  if (FIRST(a)) { ... }
     *   E_REP   { a }          ->  while (FIRST(a)) { ... }
     *   E_VAZIO                ->  nada
     *
     * Ferramentas disponiveis neste arquivo:
     *   emit(em, fmt, ...)                 uma linha indentada
     *   em->nivel++ / em->nivel--          indentacao dos blocos
     *   emit_teste(em, "if (", g, &x->first, ") {")
     *   emit_erro(em, &e->first)           erro_sintatico com os esperados
     * Os conjuntos FIRST de cada no foram calculados em conjuntos.c.
     * Uma alternativa anulavel nao tem teste: ela fica no else final,
     * no lugar do erro.
     */
    default:
        break;
    }
}

/* ------------------------------------------------------------------ */
/* Expressoes regulares de TOKENS (modo proprio)                      */
/* ------------------------------------------------------------------ */

/* Emite a funcao
 *     static size_t <nome>(const char *s)
 * que devolve o comprimento do MAIOR prefixo de s reconhecido pela
 * expressao regular, ou 0 se nenhum prefixo for reconhecido.
 * Retorna 0 se a expressao for valida (senao reporte com diag_erro). */
#define MAX_NFA_STATES 1024
#define MAX_DFA_STATES 1024

typedef struct {
    int eps;
    unsigned int chars[8];
    int out1, out2;
} NFA_State;

static NFA_State nfa[MAX_NFA_STATES];
static int nfa_size = 0;

static int new_state(void)
{
    int s = nfa_size++;
    nfa[s].eps = 1;
    memset(nfa[s].chars, 0, sizeof(nfa[s].chars));
    nfa[s].out1 = -1;
    nfa[s].out2 = -1;
    return s;
}

static void set_char(NFA_State *s, unsigned char c)
{
    s->eps = 0;
    s->chars[c >> 5] |= (1u << (c & 31));
}

static void set_range(NFA_State *s, unsigned char a, unsigned char b)
{
    unsigned int i;
    s->eps = 0;
    for (i = a; i <= b; i++)
        s->chars[i >> 5] |= (1u << (i & 31));
}

static unsigned char ler_escape(const char **p)
{
    unsigned char c;
    (*p)++;
    c = (unsigned char) **p;
    if (c == '\0')
        return '\\';
    (*p)++;
    if (c == 'n') return '\n';
    if (c == 't') return '\t';
    if (c == 'r') return '\r';
    return c;
}

static int parse_or(const char **p, int *start, int *end);

static int parse_atom(const char **p, int *start, int *end)
{
    int s, e, in_s, in_e, invert, i;
    unsigned char c1, c2;

    if (**p == '\0' || **p == '|' || **p == '*' || **p == '+' || **p == '?' || **p == ')')
        return 0;

    s = new_state();
    e = new_state();
    *start = s;
    *end = e;
    nfa[s].out1 = e;

    if (**p == '(') {
        (*p)++;
        if (!parse_or(p, &in_s, &in_e))
            return 0;
        if (**p != ')')
            return 0;
        (*p)++;
        nfa[s].eps = 1;
        nfa[s].out1 = in_s;
        nfa[in_e].eps = 1;
        nfa[in_e].out1 = e;
        return 1;
    }

    if (**p == '[') {
        (*p)++;
        nfa[s].eps = 0;
        invert = 0;
        if (**p == '^') {
            invert = 1;
            (*p)++;
        }
        if (**p == ']' || **p == '\0')
            return 0;
        while (**p != '\0' && **p != ']') {
            c1 = (**p == '\\') ? ler_escape(p) : (unsigned char) *(*p)++;
            if (**p == '-' && (*p)[1] != '\0' && (*p)[1] != ']') {
                (*p)++;
                c2 = (**p == '\\') ? ler_escape(p) : (unsigned char) *(*p)++;
                if (c1 > c2)
                    return 0;
                set_range(&nfa[s], c1, c2);
            } else {
                set_char(&nfa[s], c1);
            }
        }
        if (**p != ']')
            return 0;
        (*p)++;
        if (invert) {
            for (i = 0; i < 8; i++)
                nfa[s].chars[i] = ~nfa[s].chars[i];
            nfa[s].chars[0] &= ~1u;
        }
        return 1;
    }

    if (**p == '.') {
        (*p)++;
        nfa[s].eps = 0;
        for (i = 1; i < 256; i++) {
            if (i != '\n')
                set_char(&nfa[s], (unsigned char) i);
        }
        return 1;
    }

    if (**p == '\\') {
        c1 = ler_escape(p);
        set_char(&nfa[s], c1);
        return 1;
    }

    set_char(&nfa[s], (unsigned char) **p);
    (*p)++;
    return 1;
}

static int parse_suffix(const char **p, int *start, int *end)
{
    int s, e, ns, ne;
    if (!parse_atom(p, &s, &e))
        return 0;
    while (**p == '*' || **p == '+' || **p == '?') {
        ns = new_state();
        ne = new_state();
        if (**p == '*') {
            nfa[ns].eps = 1; nfa[ns].out1 = s; nfa[ns].out2 = ne;
            nfa[e].eps = 1;  nfa[e].out1 = s;  nfa[e].out2 = ne;
        } else if (**p == '+') {
            nfa[ns].eps = 1; nfa[ns].out1 = s;
            nfa[e].eps = 1;  nfa[e].out1 = s;  nfa[e].out2 = ne;
        } else {
            nfa[ns].eps = 1; nfa[ns].out1 = s; nfa[ns].out2 = ne;
            nfa[e].eps = 1;  nfa[e].out1 = ne;
        }
        s = ns;
        e = ne;
        (*p)++;
    }
    *start = s;
    *end = e;
    return 1;
}

static int parse_concat(const char **p, int *start, int *end)
{
    int s = -1, e = -1, ns, ne;
    while (**p != '\0' && **p != '|' && **p != ')') {
        if (!parse_suffix(p, &ns, &ne))
            return 0;
        if (s == -1) {
            s = ns;
            e = ne;
        } else {
            nfa[e].eps = 1;
            nfa[e].out1 = ns;
            e = ne;
        }
    }
    if (s == -1) {
        s = new_state();
        e = new_state();
        nfa[s].eps = 1;
        nfa[s].out1 = e;
    }
    *start = s;
    *end = e;
    return 1;
}

static int parse_or(const char **p, int *start, int *end)
{
    int s, e, ns, ne, n_s;
    if (!parse_concat(p, &ns, &ne))
        return 0;
    s = new_state();
    e = new_state();
    nfa[s].eps = 1;
    nfa[s].out1 = ns;
    nfa[ne].eps = 1;
    nfa[ne].out1 = e;

    while (**p == '|') {
        (*p)++;
        if (!parse_concat(p, &ns, &ne))
            return 0;
        n_s = new_state();
        nfa[n_s].eps = 1;
        nfa[n_s].out1 = s;
        nfa[n_s].out2 = ns;
        s = n_s;
        nfa[ne].eps = 1;
        nfa[ne].out1 = e;
    }
    *start = s;
    *end = e;
    return 1;
}

typedef struct {
    int states[MAX_NFA_STATES];
    int n;
} StateSet;

static void add_state(StateSet *ss, int s)
{
    int i;
    for (i = 0; i < ss->n; i++)
        if (ss->states[i] == s)
            return;
    ss->states[ss->n++] = s;
}

static void closure(StateSet *ss)
{
    int changed = 1, old_n, i, s, old;
    while (changed) {
        old_n = ss->n;
        changed = 0;
        for (i = 0; i < old_n; i++) {
            s = ss->states[i];
            if (nfa[s].eps) {
                if (nfa[s].out1 != -1) {
                    old = ss->n;
                    add_state(ss, nfa[s].out1);
                    if (ss->n > old) changed = 1;
                }
                if (nfa[s].out2 != -1) {
                    old = ss->n;
                    add_state(ss, nfa[s].out2);
                    if (ss->n > old) changed = 1;
                }
            }
        }
    }
}

typedef struct {
    StateSet ss;
    int trans[256];
    int is_final;
} DFA_State;

static DFA_State dfa[MAX_DFA_STATES];
static int dfa_size = 0;

static int get_dfa_state(StateSet *ss)
{
    int i, j, k, id, c, match, found;
    for (i = 0; i < dfa_size; i++) {
        if (dfa[i].ss.n == ss->n) {
            match = 1;
            for (j = 0; j < ss->n; j++) {
                found = 0;
                for (k = 0; k < dfa[i].ss.n; k++) {
                    if (dfa[i].ss.states[k] == ss->states[j]) {
                        found = 1;
                        break;
                    }
                }
                if (!found) {
                    match = 0;
                    break;
                }
            }
            if (match)
                return i;
        }
    }
    id = dfa_size++;
    dfa[id].ss = *ss;
    dfa[id].is_final = 0;
    for (c = 0; c < 256; c++)
        dfa[id].trans[c] = -1;
    return id;
}

static void build_dfa(int start, int end)
{
    int i, j, c, s;
    StateSet init, next;

    dfa_size = 0;
    init.n = 0;
    add_state(&init, start);
    closure(&init);
    get_dfa_state(&init);

    for (i = 0; i < dfa_size; i++) {
        for (j = 0; j < dfa[i].ss.n; j++) {
            if (dfa[i].ss.states[j] == end)
                dfa[i].is_final = 1;
        }
        for (c = 0; c < 256; c++) {
            next.n = 0;
            for (j = 0; j < dfa[i].ss.n; j++) {
                s = dfa[i].ss.states[j];
                if (!nfa[s].eps) {
                    if (nfa[s].chars[c >> 5] & (1u << (c & 31))) {
                        if (nfa[s].out1 != -1)
                            add_state(&next, nfa[s].out1);
                    }
                }
            }
            if (next.n > 0) {
                closure(&next);
                dfa[i].trans[c] = get_dfa_state(&next);
            }
        }
    }
}

static int gen_casamento_regex(FILE *f, const char *nome, const char *regex)
{
    const char *p = regex;
    int start, end, i, c;

    nfa_size = 0;
    if (regex == NULL || *regex == '\0' || !parse_or(&p, &start, &end) || *p != '\0') {
        diag_erro(0, 0, "expressao regular invalida: %s", regex ? regex : "");
        return -1;
    }
    build_dfa(start, end);

    fprintf(f, "static size_t %s(const char *s)\n{\n", nome);
    fprintf(f, "    static const int trans[%d][256] = {\n", dfa_size);
    for (i = 0; i < dfa_size; i++) {
        fprintf(f, "        {");
        for (c = 0; c < 256; c++)
            fprintf(f, "%d%s", dfa[i].trans[c], c == 255 ? "" : ",");
        fprintf(f, "}%s\n", i == dfa_size - 1 ? "" : ",");
    }
    fprintf(f, "    };\n");

    fprintf(f, "    static const int is_final[%d] = {", dfa_size);
    for (i = 0; i < dfa_size; i++)
        fprintf(f, "%d%s", dfa[i].is_final, i == dfa_size - 1 ? "" : ",");
    fprintf(f, "};\n");

    fprintf(f, "    int estado = 0;\n");
    fprintf(f, "    size_t indice = 0, ultimo_fim = 0;\n");
    fprintf(f, "    unsigned char ch;\n");
    fprintf(f, "    while ((ch = (unsigned char)s[indice]) != '\\0') {\n");
    fprintf(f, "        if (is_final[estado]) ultimo_fim = indice;\n");
    fprintf(f, "        estado = trans[estado][ch];\n");
    fprintf(f, "        if (estado == -1) break;\n");
    fprintf(f, "        indice++;\n");
    fprintf(f, "    }\n");
    fprintf(f, "    if (estado != -1 && is_final[estado]) ultimo_fim = indice;\n");
    fprintf(f, "    return ultimo_fim;\n");
    fprintf(f, "}\n\n");

    return 0;
}

/* ------------------------------------------------------------------ */
/* Leitura do arquivo de tokens (opcao --tokens do analisador gerado) */
/* ------------------------------------------------------------------ */

/* Emite "static Token token_de_arquivo(void)", que le o proximo token
 * do arquivo aberto em arquivo_tokens, no formato gravado pelo lexer do
 * TP1 (livro-texto, secao 3.3.2):
 *
 *   Token: tipo = <numero>, lexema = '<texto>', linha = <numero>
 *
 * Ao chegar ao fim do arquivo devolve um token TOKEN_FIM. */
static void gen_token_de_arquivo(FILE *f, int microc)
{
    static const char *const linhas[] = {
        "static Token token_de_arquivo(void)",
        "{",
        "    static int num_linha_arq = 0;",
        "    char buf[1024];",
        "    Token t;",
        "    memset(&t, 0, sizeof t);",
        "",
        "    while (fgets(buf, sizeof buf, arquivo_tokens) != NULL) {",
        "        char *p = buf;",
        "        char *lex_ini, *lex_fim, *linha_ptr;",
        "        int len, lido_tipo;",
        "        num_linha_arq++;",
        "",
        "        /* Ignora linhas em branco */",
        "        while (*p == ' ' || *p == '\\t' || *p == '\\r' || *p == '\\n') p++;",
        "        if (*p == '\\0') continue;",
        "",
        "        /* Extrai o tipo */",
        "        if (sscanf(p, \"Token: tipo = %d\", &lido_tipo) != 1) goto malformada;",
        "        t.tipo = lido_tipo;",
        "",
        "        /* Localiza o inicio do lexema */",
        "        lex_ini = strstr(p, \"lexema = '\");",
        "        if (lex_ini == NULL) goto malformada;",
        "        lex_ini += 10; /* pula \"lexema = '\" */",
        "",
        "        /* Localiza a ultima aspa simples antes da virgula da linha */",
        "        lex_fim = strrchr(lex_ini, '\\'');",
        "        if (lex_fim == NULL) goto malformada;",
        "",
        "        /* Extrai a linha */",
        "        linha_ptr = strstr(lex_fim, \", linha = \");",
        "        if (linha_ptr == NULL || sscanf(linha_ptr, \", linha = %d\", &t.linha) != 1) goto malformada;",
        "",
        "        /* Copia o lexema limitando ao tamanho do buffer da struct Token */",
        "        len = lex_fim - lex_ini;",
        "        if (len >= (int)sizeof(t.lexema)) len = sizeof(t.lexema) - 1;",
        "        memcpy(t.lexema, lex_ini, len);",
        "        t.lexema[len] = '\\0';",
        "        return t;",
        "",
        "    malformada:",
        "        fprintf(stderr, \"linha %d do arquivo de tokens malformada\\n\", num_linha_arq);",
        "        exit(EXIT_FAILURE);",
        "    }",
        "",
        "    /* Se o fgets retornar NULL, chegamos ao fim do arquivo */",
        "    t.tipo = TOKEN_FIM;",
        "    strcpy(t.lexema, \"EOF\");",
        "    return t;",
        "}",
        "",
        NULL
    };
    (void) microc;
    emit_linhas(f, linhas);
}

/* ------------------------------------------------------------------ */
/* Partes fixas do codigo gerado                                      */
/* ------------------------------------------------------------------ */

static const char *const rt_estado[] = {
    "/* ---- estado do analisador ---- */",
    "",
    "static Token tok;                  /* token corrente (lookahead) */",
    "static int imprimir_arvore = 0;    /* opcao --tree */",
    "static int nivel_arvore = 0;",
    "static FILE *arquivo_tokens = NULL; /* opcao --tokens */",
    "",
    "static Token token_de_arquivo(void);",
    "",
    NULL
};

static const char *const rt_primitivas[] = {
    "/* ---- primitivas ---- */",
    "",
    "int pertence(int tipo, const int *conjunto)",
    "{",
    "    for (; *conjunto != -1; conjunto++)",
    "        if (*conjunto == tipo)",
    "            return 1;",
    "    return 0;",
    "}",
    "",
    "/* Reporta o erro com os tokens esperados (lista terminada em -1) e encerra. */",
    "void erro_sintatico(const int *esperados)",
    "{",
    "    int i;",
    "    escrever_posicao();",
    "    if ((int) tok.tipo == TOKEN_FIM)",
    "        fprintf(stderr, \"encontrado fim de arquivo, \");",
    "    else",
    "        fprintf(stderr, \"encontrado '%s', \", tok.lexema);",
    "    if (esperados[0] != -1 && esperados[1] == -1) {",
    "        fprintf(stderr, \"esperado %s\\n\", nome_token(esperados[0]));",
    "    } else {",
    "        fprintf(stderr, \"esperado um de {\");",
    "        for (i = 0; esperados[i] != -1; i++)",
    "            fprintf(stderr, \"%s%s\", i > 0 ? \", \" : \"\", nome_token(esperados[i]));",
    "        fprintf(stderr, \"}\\n\");",
    "    }",
    "    exit(EXIT_FAILURE);",
    "}",
    "",
    "static void recuo(void)",
    "{",
    "    int i;",
    "    for (i = 0; i < nivel_arvore; i++)",
    "        fputs(\"  \", stdout);",
    "}",
    "",
    "void arvore_entra(const char *nao_terminal)",
    "{",
    "    if (imprimir_arvore) {",
    "        recuo();",
    "        printf(\"%s\\n\", nao_terminal);",
    "    }",
    "    nivel_arvore++;",
    "}",
    "",
    "void arvore_sai(void)",
    "{",
    "    nivel_arvore--;",
    "}",
    "",
    "/* Consome o token corrente se for do tipo esperado; senao, erro. */",
    "void match(int esperado)",
    "{",
    "    int lista[2];",
    "    if ((int) tok.tipo != esperado) {",
    "        lista[0] = esperado;",
    "        lista[1] = -1;",
    "        erro_sintatico(lista);",
    "    }",
    "    if (SUPORTE_ARVORE && imprimir_arvore) {",
    "        recuo();",
    "        if (nome_token(tok.tipo)[0] == '\\'')",
    "            printf(\"%s\\n\", nome_token(tok.tipo));",
    "        else",
    "            printf(\"%s '%s'\\n\", nome_token(tok.tipo), tok.lexema);",
    "    }",
    "    avancar();",
    "}",
    "",
    NULL
};

/* Lexico proprio: partes fixas antes e depois das funcoes de regex. */
static const char *const rt_lexico_estado[] = {
    "/* ---- analisador lexico gerado a partir de TOKENS ---- */",
    "",
    "static char *fonte = NULL;",
    "static size_t pos = 0;",
    "static int linha = 1, coluna = 1;",
    "",
    "static char *ler_tudo(FILE *f)",
    "{",
    "    size_t cap = 4096, n = 0, lidos;",
    "    char *buf = (char *) malloc(cap), *novo;",
    "    if (buf == NULL) {",
    "        fprintf(stderr, \"memoria insuficiente\\n\");",
    "        exit(EXIT_FAILURE);",
    "    }",
    "    while ((lidos = fread(buf + n, 1, cap - n - 1, f)) > 0) {",
    "        n += lidos;",
    "        if (cap - n - 1 == 0) {",
    "            cap *= 2;",
    "            novo = (char *) realloc(buf, cap);",
    "            if (novo == NULL) {",
    "                fprintf(stderr, \"memoria insuficiente\\n\");",
    "                exit(EXIT_FAILURE);",
    "            }",
    "            buf = novo;",
    "        }",
    "    }",
    "    buf[n] = '\\0';",
    "    return buf;",
    "}",
    "",
    "static void consumir(size_t n)",
    "{",
    "    for (; n > 0 && fonte[pos] != '\\0'; n--, pos++) {",
    "        if (fonte[pos] == '\\n') {",
    "            linha++;",
    "            coluna = 1;",
    "        } else {",
    "            coluna++;",
    "        }",
    "    }",
    "}",
    "",
    NULL
};

static void escrever_cabecalho_c(FILE *f, const OpcoesGeracao *op)
{
    fputs("/*\n * Analisador sintatico descendente recursivo gerado a partir de ", f);
    emitir_comentario(f, op->origem);
    fputs(".\n * Arquivo gerado automaticamente: nao edite.\n */\n", f);
}

/* ------------------------------------------------------------------ */
/* Arquivo .h                                                         */
/* ------------------------------------------------------------------ */

static int gerar_h(const Gramatica *g, const OpcoesGeracao *op)
{
    char *caminho = (char *) xmalloc(strlen(op->prefixo) + 3);
    FILE *f;
    int i;

    sprintf(caminho, "%s.h", op->prefixo);
    f = fopen(caminho, "w");
    if (f == NULL) {
        diag_erro(0, 0, "nao foi possivel criar %s", caminho);
        free(caminho);
        return -1;
    }
    escrever_cabecalho_c(f, op);
    fputs("#ifndef PARSER_GERADO_H\n#define PARSER_GERADO_H\n\n", f);
    if (op->microc) {
        fputs("#include \"scanner.h\"   /* scanner do TP1: Token, TokenType, proximo_token */\n\n", f);
        fputs("#define TOKEN_FIM END_OF_FILE\n\n", f);
    } else {
        fputs("typedef enum {\n", f);
        for (i = 0; i < g->nterms; i++) {
            fprintf(f, "    %s%s   /* ", g->terms[i].nome_c, i + 1 < g->nterms ? "," : "");
            emitir_comentario(f, g->terms[i].exibicao);
            fputs(" */\n", f);
        }
        fputs("} TipoToken;\n\n", f);
        fputs("typedef struct {\n    int tipo;\n    char lexema[100];\n"
              "    int linha;\n    int coluna;\n} Token;\n\n", f);
        fputs("#define TOKEN_FIM T_FIM\n\n", f);
    }
    fputs("const char *nome_token(int tipo);\n", f);
    fputs("int  pertence(int tipo, const int *conjunto);\n", f);
    fputs("void avancar(void);\n", f);
    fputs("void match(int esperado);\n", f);
    fputs("void erro_sintatico(const int *esperados);\n", f);
    fputs("void arvore_entra(const char *nao_terminal);\n", f);
    fputs("void arvore_sai(void);\n\n", f);
    for (i = 0; i < g->nnts; i++)
        if (g->nts[i].regra != NULL)
            fprintf(f, "void parse_%s(void);\n", g->nts[i].nome);
    fputs("\n#endif\n", f);
    fclose(f);
    free(caminho);
    return 0;
}

/* ------------------------------------------------------------------ */
/* Arquivo .c                                                         */
/* ------------------------------------------------------------------ */

static void gerar_nome_token(FILE *f, const Gramatica *g)
{
    int i;
    fputs("const char *nome_token(int tipo)\n{\n    switch (tipo) {\n", f);
    for (i = 0; i < g->nterms; i++) {
        fprintf(f, "    case %s: return ", g->terms[i].nome_c);
        emitir_string_c(f, i == TERM_FIM ? "fim de arquivo" : g->terms[i].exibicao);
        fputs(";\n", f);
    }
    fputs("    default: return \"token desconhecido\";\n    }\n}\n\n", f);
}

static void gerar_posicao_e_avancar(FILE *f, int microc)
{
    fputs("static void escrever_posicao(void)\n{\n", f);
    if (microc)
        fputs("    fprintf(stderr, \"linha %d: \", tok.linha);\n", f);
    else
        fputs("    fprintf(stderr, \"linha %d, coluna %d: \", tok.linha, tok.coluna);\n", f);
    fputs("}\n\n", f);

    fputs("/* Le o proximo token para tok. */\nvoid avancar(void)\n{\n", f);
    fputs("    if (arquivo_tokens != NULL)\n        tok = token_de_arquivo();\n    else\n", f);
    if (microc) {
        fputs("        tok = proximo_token();\n", f);
        fputs("    if (tok.tipo == UNDEF) {\n", f);
        fputs("        fprintf(stderr, \"linha %d: erro lexico: %s\\n\", tok.linha, tok.lexema);\n", f);
        fputs("        exit(EXIT_FAILURE);\n    }\n", f);
    } else {
        fputs("        tok = proximo_token_gerado();\n", f);
    }
    fputs("}\n\n", f);
}

static int gerar_lexico(FILE *f, const Gramatica *g)
{
    int i, nlit = 0, erros = 0;
    char nome[300];

    emit_linhas(f, rt_lexico_estado);

    for (i = 0; i < g->nskips; i++) {
        sprintf(nome, "casa_skip_%d", i);
        fputs("/* SKIP: ", f);
        emitir_comentario(f, g->skips[i]);
        fputs(" */\n", f);
        if (gen_casamento_regex(f, nome, g->skips[i]) != 0)
            erros++;
    }
    for (i = 1; i < g->nterms; i++) {
        if (g->terms[i].literal) {
            nlit++;
            continue;
        }
        sprintf(nome, "casa_%.250s", g->terms[i].nome_c);
        fprintf(f, "/* %s: ", g->terms[i].nome);
        emitir_comentario(f, g->terms[i].regex ? g->terms[i].regex : "");
        fputs(" */\n", f);
        if (gen_casamento_regex(f, nome, g->terms[i].regex ? g->terms[i].regex : "")
            != 0)
            erros++;
    }

    /* literais: comparacao direta, com prioridade em caso de empate */
    fputs("static const char *const texto_literais[] = {\n", f);
    for (i = 1; i < g->nterms; i++)
        if (g->terms[i].literal) {
            fputs("    ", f);
            emitir_string_c(f, g->terms[i].nome);
            fputs(",\n", f);
        }
    fputs("    NULL\n};\n\nstatic const int tipo_literais[] = {\n", f);
    for (i = 1; i < g->nterms; i++)
        if (g->terms[i].literal)
            fprintf(f, "    %s,\n", g->terms[i].nome_c);
    fputs("    -1\n};\n\n", f);

    fputs("/* Reconhece o maior prefixo da entrada; em caso de empate vence\n"
          " * o literal (palavras reservadas tem prioridade sobre ID). */\n", f);
    fputs("static Token proximo_token_gerado(void)\n{\n", f);
    fputs("    Token t;\n    size_t n, melhor;\n    int i, tipo, pulou;\n\n", f);
    fputs("    do {\n        pulou = 0;\n", f);
    for (i = 0; i < g->nskips; i++)
        fprintf(f, "        if ((n = casa_skip_%d(fonte + pos)) > 0) {\n"
                   "            consumir(n);\n            pulou = 1;\n        }\n", i);
    fputs("    } while (pulou);\n\n", f);
    fputs("    memset(&t, 0, sizeof t);\n", f);
    fputs("    t.linha = linha;\n    t.coluna = coluna;\n", f);
    fputs("    if (fonte[pos] == '\\0') {\n        t.tipo = T_FIM;\n        return t;\n    }\n", f);
    fputs("    melhor = 0;\n    tipo = -1;\n", f);
    fputs("    for (i = 0; texto_literais[i] != NULL; i++) {\n", f);
    fputs("        n = strlen(texto_literais[i]);\n", f);
    fputs("        if (n > melhor && strncmp(fonte + pos, texto_literais[i], n) == 0) {\n", f);
    fputs("            melhor = n;\n            tipo = tipo_literais[i];\n        }\n    }\n", f);
    for (i = 1; i < g->nterms; i++)
        if (!g->terms[i].literal)
            fprintf(f, "    if ((n = casa_%s(fonte + pos)) > melhor) {\n"
                       "        melhor = n;\n        tipo = %s;\n    }\n",
                    g->terms[i].nome_c, g->terms[i].nome_c);
    fputs("    if (melhor == 0) {\n", f);
    fputs("        fprintf(stderr, \"linha %d, coluna %d: erro lexico: caractere "
          "invalido '%c'\\n\",\n                linha, coluna, fonte[pos]);\n", f);
    fputs("        exit(EXIT_FAILURE);\n    }\n", f);
    fputs("    n = melhor < sizeof t.lexema ? melhor : sizeof t.lexema - 1;\n", f);
    fputs("    memcpy(t.lexema, fonte + pos, n);\n    t.lexema[n] = '\\0';\n", f);
    fputs("    t.tipo = tipo;\n    consumir(melhor);\n    return t;\n}\n\n", f);
    (void) nlit;
    return erros;
}

static void gerar_procedimentos(FILE *f, const Gramatica *g, const OpcoesGeracao *op)
{
    int i;
    Emissor em;
    em.f = f;
    em.nivel = 0;

    fputs("/* ---- procedimentos (um por nao terminal) ---- */\n\n", f);
    for (i = 0; i < g->nnts; i++) {
        if (g->nts[i].regra == NULL)
            continue;
        fprintf(f, "/* %s = ", g->nts[i].nome);
        {
            /* a regra em EBNF, sem "* /" */
            FILE *tmp = tmpfile();
            int c;
            if (tmp != NULL) {
                char buf[2] = { 0, 0 };
                expr_imprimir(tmp, g, g->nts[i].regra);
                rewind(tmp);
                while ((c = fgetc(tmp)) != EOF) {
                    buf[0] = (char) c;
                    emitir_comentario(f, buf);
                }
                fclose(tmp);
            }
        }
        fputs(" ; */\n", f);
        fprintf(f, "void parse_%s(void)\n{\n", g->nts[i].nome);
        em.nivel = 1;
        if (op->arvore)
            emit(&em, "arvore_entra(\"%s\");", g->nts[i].nome);
        gen_expr(&em, g, g->nts[i].regra);
        if (op->arvore)
            emit(&em, "arvore_sai();");
        fputs("}\n\n", f);
    }
}

static void gerar_main(FILE *f, const Gramatica *g, const OpcoesGeracao *op)
{
    fputs("/* ---- programa principal ---- */\n\n", f);
    fputs("int main(int argc, char **argv)\n{\n", f);
    fputs("    int i, fim[2];\n    const char *arquivo = NULL, *tokens = NULL;\n", f);
    fputs("    FILE *entrada = stdin;\n\n", f);
    fputs("    for (i = 1; i < argc; i++) {\n", f);
    fputs("        if (strcmp(argv[i], \"--tree\") == 0) {\n", f);
    fputs("            imprimir_arvore = 1;\n", f);
    fputs("        } else if (strcmp(argv[i], \"--tokens\") == 0 && i + 1 < argc) {\n", f);
    fputs("            tokens = argv[++i];\n", f);
    fputs("        } else if (argv[i][0] == '-' && argv[i][1] != '\\0') {\n", f);
    fputs("            fprintf(stderr, \"uso: %s [--tree] [--tokens arquivo.tok | "
          "programa]\\n\", argv[0]);\n", f);
    fputs("            return 2;\n        } else {\n            arquivo = argv[i];\n"
          "        }\n    }\n", f);
    fputs("    if (imprimir_arvore && !SUPORTE_ARVORE)\n", f);
    fputs("        fprintf(stderr, \"aviso: gere o analisador com --tree para "
          "imprimir a arvore\\n\");\n\n", f);
    fputs("    if (tokens != NULL) {\n", f);
    fputs("        arquivo_tokens = fopen(tokens, \"r\");\n", f);
    fputs("        if (arquivo_tokens == NULL) {\n", f);
    fputs("            fprintf(stderr, \"nao foi possivel abrir %s\\n\", tokens);\n", f);
    fputs("            return 2;\n        }\n    } else {\n", f);
    fputs("        if (arquivo != NULL && (entrada = fopen(arquivo, \"r\")) == NULL) {\n", f);
    fputs("            fprintf(stderr, \"nao foi possivel abrir %s\\n\", arquivo);\n", f);
    fputs("            return 2;\n        }\n", f);
    if (op->microc)
        fputs("        scanner_iniciar(entrada);\n", f);
    else
        fputs("        fonte = ler_tudo(entrada);\n", f);
    fputs("    }\n\n", f);
    fputs("    avancar();\n", f);
    fprintf(f, "    parse_%s();\n", g->nts[g->inicial].nome);
    fputs("    if (tok.tipo != TOKEN_FIM) {\n", f);
    fputs("        fim[0] = TOKEN_FIM;\n        fim[1] = -1;\n", f);
    fputs("        erro_sintatico(fim);\n    }\n", f);
    fputs("    printf(\"entrada aceita\\n\");\n\n", f);
    fputs("    if (arquivo_tokens != NULL) {\n        fclose(arquivo_tokens);\n", f);
    fputs("    } else {\n", f);
    if (op->microc)
        fputs("        scanner_finalizar();\n", f);
    else
        fputs("        free(fonte);\n", f);
    fputs("        if (entrada != stdin)\n            fclose(entrada);\n    }\n", f);
    fputs("    return 0;\n}\n", f);
}

static const char *nome_base(const char *caminho)
{
    const char *b = strrchr(caminho, '/');
    return b != NULL ? b + 1 : caminho;
}

static int gerar_c(const Gramatica *g, const OpcoesGeracao *op)
{
    char *caminho = (char *) xmalloc(strlen(op->prefixo) + 3);
    FILE *f, *corpo;
    int c, erros = 0;

    /* Os procedimentos sao gerados primeiro num arquivo temporario porque
     * registram os conjuntos (conj_N), que precisam vir antes no .c. */
    corpo = tmpfile();
    if (corpo == NULL) {
        diag_erro(0, 0, "nao foi possivel criar arquivo temporario");
        free(caminho);
        return -1;
    }
    gerar_procedimentos(corpo, g, op);

    sprintf(caminho, "%s.c", op->prefixo);
    f = fopen(caminho, "w");
    if (f == NULL) {
        diag_erro(0, 0, "nao foi possivel criar %s", caminho);
        fclose(corpo);
        free(caminho);
        return -1;
    }
    escrever_cabecalho_c(f, op);
    fputs("#include <stdio.h>\n#include <stdlib.h>\n#include <string.h>\n", f);
    fprintf(f, "#include \"%s.h\"\n\n", nome_base(op->prefixo));
    fprintf(f, "#define SUPORTE_ARVORE %d\n\n", op->arvore ? 1 : 0);

    fputs("/* ---- conjuntos de tokens ---- */\n\n", f);
    emitir_conjuntos(f, g);
    fputc('\n', f);

    emit_linhas(f, rt_estado);
    if (!op->microc)
        erros += gerar_lexico(f, g);
    gerar_nome_token(f, g);
    gerar_posicao_e_avancar(f, op->microc);
    emit_linhas(f, rt_primitivas);
    gen_token_de_arquivo(f, op->microc);

    rewind(corpo);
    while ((c = fgetc(corpo)) != EOF)
        fputc(c, f);
    fclose(corpo);

    gerar_main(f, g, op);
    fclose(f);
    free(caminho);
    return erros == 0 ? 0 : -1;
}

int codegen_gerar(Gramatica *g, const OpcoesGeracao *op)
{
    int r;
    nomear_terminais(g, op->microc);
    r = gerar_h(g, op);
    if (r == 0)
        r = gerar_c(g, op);
    liberar_conjuntos();
    return r;
}
