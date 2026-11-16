/*
 * scanner.c - Adaptador entre o scanner do TP1 (microc.flex) e o
 * analisador sintatico gerado (secao 3.1 do enunciado).
 *
 * Esta versao funciona com qualquer scanner flex gerado com
 *     %option prefix="microc_yy"
 * que mantenha a variavel global linha_atual (TP1, secao 4.4). Sem
 * alteracoes, o lexema de cada token e o texto reconhecido (microc_yytext).
 *
 * TODO(aluno): adaptar os pontos marcados ao seu TP1, para que
 *   - ID e constantes usem o texto do Symbol (microc_yylval.symbol), ja
 *     com as sequencias de escape convertidas (TP1, secao 4.3);
 *   - UNDEF traga a mensagem de microc_yylval.error_msg
 *     ("String nao terminada", "EOF em comentario" etc.);
 *   - scanner_finalizar libere a tabela de strings.
 */
#include <stdio.h>
#include <string.h>
#include "scanner.h"

/* ---- simbolos do scanner gerado pelo flex (ajuste ao seu TP1) ---- */
extern int   microc_yylex(void);
extern FILE *microc_yyin;
extern char *microc_yytext;
extern int   linha_atual;
extern int   microc_yylex_destroy(void);

/* Texto a guardar em Token.lexema para o token que microc_yylex acabou
 * de devolver. */
static const char *texto_do_lexema(int tipo)
{
    /* TODO(aluno): para ID, INTEGERCONST, CHARCONST e STRINGCONST, devolva
     * o texto do Symbol em microc_yylval.symbol; para UNDEF, devolva
     * microc_yylval.error_msg. */
    (void) tipo;
    return microc_yytext;
}

/* O TP1 permite que tokens de um caractere sejam devolvidos pelo proprio
 * codigo ASCII (TP1, secao 5). O analisador gerado compara com os nomes
 * de TokenType, entao esses codigos sao convertidos aqui.
 * Cuidado: valores ate o ultimo item da enumeracao (PRINT = 36 no
 * tokens.h do livro) sao sempre TokenType. Por isso '!' (33, que e
 * RETURN), '"', '#' e '$' nao podem ser devolvidos como ASCII. */
static TokenType normalizar(int tipo)
{
    if (tipo <= (int) PRINT)
        return (TokenType) tipo;
    switch (tipo) {
    case '+': return PLUS;
    case '-': return MINUS;
    case '*': return MUL;
    case '/': return DIV;
    case '%': return MOD;
    case '<': return LT;
    case '>': return GT;
    case '=': return ASSIGN;
    case ';': return SEMICOLON;
    case ',': return COMMA;
    case '(': return LPAREN;
    case ')': return RPAREN;
    case '{': return LBRACE;
    case '}': return RBRACE;
    case '[': return LBRACKET;
    case ']': return RBRACKET;
    default:  return (TokenType) tipo;
    }
}

void scanner_iniciar(FILE *entrada)
{
    microc_yyin = entrada;
    linha_atual = 1;
}

Token proximo_token(void)
{
    Token t;
    int tipo;
    const char *texto;

    memset(&t, 0, sizeof t);
    tipo = microc_yylex();
    /* Observacao: o flex devolve 0 no fim da entrada, mas 0 e UNDEF no
     * tokens.h. O TP1 deve devolver END_OF_FILE na regra <<EOF>>. */
    t.tipo = normalizar(tipo);
    t.linha = linha_atual;
    texto = t.tipo == END_OF_FILE ? "" : texto_do_lexema(tipo);
    if (texto != NULL) {
        strncpy(t.lexema, texto, sizeof t.lexema - 1);
        t.lexema[sizeof t.lexema - 1] = '\0';
    }
    return t;
}

void scanner_finalizar(void)
{
    /* TODO(aluno): liberar tambem a tabela de strings do TP1. */
    microc_yylex_destroy();
}
