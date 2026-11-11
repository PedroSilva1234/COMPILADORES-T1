/*
 * scanner.h - O scanner de Micro C do TP1 convertido em funcao
 * (secao 3.1 do enunciado).
 *
 * O analisador gerado com --scanner microc inclui este cabecalho e chama
 * proximo_token() cada vez que precisa de um novo token. Token e
 * TokenType vem do tokens.h do TP1 (livro-texto, secao 2.3.1):
 *
 *   typedef struct { TokenType tipo; char lexema[100]; int linha; } Token;
 */
#ifndef SCANNER_H
#define SCANNER_H

#include <stdio.h>
#include "tokens.h"

/* Prepara o scanner para ler de entrada (ja aberto). */
void  scanner_iniciar(FILE *entrada);

/* Devolve o proximo token. No fim da entrada devolve END_OF_FILE; em
 * erro lexico devolve UNDEF com a mensagem de erro em lexema. */
Token proximo_token(void);

/* Libera os recursos do scanner (tabela de strings, buffers do flex). */
void  scanner_finalizar(void);

#endif
