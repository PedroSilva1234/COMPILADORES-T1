/*
 * Analisador sintatico descendente recursivo gerado a partir de testes/gramaticas/microc.ebnf.
 * Arquivo gerado automaticamente: nao edite.
 */
#ifndef PARSER_GERADO_H
#define PARSER_GERADO_H

#include "scanner.h"   /* scanner do TP1: Token, TokenType, proximo_token */

#define TOKEN_FIM END_OF_FILE

const char *nome_token(int tipo);
int  pertence(int tipo, const int *conjunto);
void avancar(void);
void match(int esperado);
void erro_sintatico(const int *esperados);
void arvore_entra(const char *nao_terminal);
void arvore_sai(void);

void parse_programa(void);
void parse_bloco(void);
void parse_declaracao(void);
void parse_comando(void);
void parse_tipo(void);
void parse_comando_if(void);
void parse_comando_for(void);
void parse_atribuicao(void);
void parse_comando_return(void);
void parse_comando_print(void);
void parse_expressao(void);
void parse_atribuicao_interna(void);
void parse_conteudo_print(void);
void parse_expressao_logica(void);
void parse_expressao_relacional(void);
void parse_expressao_aritmetica(void);
void parse_operador_relacional(void);
void parse_termo(void);
void parse_fator(void);

#endif
