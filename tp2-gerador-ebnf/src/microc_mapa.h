/*
 * microc_mapa.h - Correspondencia entre a gramatica e os tokens de Micro C
 * (modo --scanner microc, secao 3.1 do enunciado).
 *
 * No modo Micro C os terminais nao sao gerados: sao os valores da
 * enumeracao TokenType de tokens.h (TP1 / livro-texto, secao 2.3.1).
 * Os literais da gramatica ("if", "+", ";") sao traduzidos para esses
 * nomes (IF, PLUS, SEMICOLON) e os terminais nomeados (ID, INTEGERCONST...)
 * sao usados diretamente.
 */
#ifndef MICROC_MAPA_H
#define MICROC_MAPA_H

/* Nome em TokenType para um literal da gramatica, ou NULL se o literal
 * nao for um token de Micro C. Ex.: "+" -> "PLUS", "while" -> NULL. */
const char *microc_nome_literal(const char *literal);

/* Le os nomes da enumeracao TokenType de um tokens.h. Retorna 0 se a
 * leitura foi feita, -1 caso contrario (a verificacao e pulada). */
int microc_carregar_tokens_h(const char *caminho);

/* 1 se o nome existe na enumeracao lida por microc_carregar_tokens_h. */
int microc_token_existe(const char *nome);

/* Libera a lista de nomes lida. */
void microc_liberar(void);

#endif
