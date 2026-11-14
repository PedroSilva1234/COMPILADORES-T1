/*
 * Analisador sintatico descendente recursivo gerado a partir de testes/gramaticas/expr.ebnf.
 * Arquivo gerado automaticamente: nao edite.
 */
#ifndef PARSER_GERADO_H
#define PARSER_GERADO_H

typedef enum {
    T_FIM,   /* $ */
    T_NUM,   /* NUM */
    T_ID,   /* ID */
    T_MAIS,   /* '+' */
    T_MENOS,   /* '-' */
    T_VEZES,   /* '*' */
    T_BARRA,   /* '/' */
    T_ABRE_PAR,   /* '(' */
    T_FECHA_PAR   /* ')' */
} TipoToken;

typedef struct {
    int tipo;
    char lexema[100];
    int linha;
    int coluna;
} Token;

#define TOKEN_FIM T_FIM

const char *nome_token(int tipo);
int  pertence(int tipo, const int *conjunto);
void avancar(void);
void match(int esperado);
void erro_sintatico(const int *esperados);
void arvore_entra(const char *nao_terminal);
void arvore_sai(void);

void parse_expr(void);
void parse_term(void);
void parse_factor(void);

#endif
