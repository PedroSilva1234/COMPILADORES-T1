/*
 * ll1.h - Verificacao da condicao LL(1) (etapa 4).
 */
#ifndef LL1_H
#define LL1_H

#include "gramatica.h"

/* Verifica todas as regras e reporta cada conflito. Retorna o numero de
 * conflitos que impedem a geracao (erros). Requer anulavel, FIRST e FOLLOW. */
int ll1_verificar(const Gramatica *g);

#endif
