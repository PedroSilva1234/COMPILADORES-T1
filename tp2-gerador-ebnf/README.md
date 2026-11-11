aluno: <sua_matricula>
uso_ia: <sim|nao>

# Compiladores I — Trabalho Prático 2: Gerador de Analisadores Sintáticos Descendentes Recursivos

Código-base do gerador. Ele lê uma gramática em EBNF e gera, em ANSI C, um analisador sintático descendente recursivo para a linguagem descrita. Veja o enunciado para os requisitos completos.

As duas primeiras linhas deste arquivo seguem o formato do TP1: uma linha `aluno:` por integrante e a linha `uso_ia:` logo abaixo. Registre o uso de IA em `USO_IA.md`.

## Compilar e executar

```
make                      # compila o gerador (e o analisador de Micro C, se o TP1 estiver em microc/)
make test                 # executa os testes de testes/
make microc               # gera e compila build/microc/parser_microc
make clean
```

```
./gerador gramatica.ebnf -o saida/parser [--scanner microc] [--sets] [--tree] [--debug]
gcc -ansi -pedantic -Wall -Wextra -o parser saida/parser.c
./parser [--tree] programa.txt        # ou: ./parser --tokens tokens.txt
```

Todo o código deve compilar sem avisos com `gcc -ansi -pedantic -Wall -Wextra`. A única exceção é o `lex.yy.c` gerado pelo flex.

## Estado do código-base

O esqueleto compila, executa de ponta a ponta e gera um analisador que compila, mas que ainda não reconhece nada. Os pontos a implementar estão marcados com `TODO(aluno)`:

```
grep -rn "TODO(aluno)" src microc
```

| Etapa | Arquivo | Fornecido | A implementar |
| --- | --- | --- | --- |
| 1. Leitura da EBNF | `ebnf_lexer.c`, `ebnf_parser.c` | léxico da notação; seções TOKENS e RULES; recuperação de erros | `parse_expr`, `parse_seq`, `parse_fator` |
| 2. Verificação semântica | `verificacao.c` | indefinidos, terminais não declarados, duplicados | inalcançáveis, improdutivos, recursão à esquerda |
| 3. Conjuntos | `conjuntos.c` | alocação e impressão (`--sets`) | anulável, FIRST, FOLLOW |
| 4. LL(1) | `ll1.c` | formatação das mensagens | as três condições LL(1) |
| 5. Geração | `codegen.c` | estrutura do `.c`/`.h`, nomes dos tokens, conjuntos constantes, primitivas, léxico (exceto as regex), `main` | `gen_expr` (sequência, alternativa, `[ ]`, `{ }`), `gen_casamento_regex`, `gen_token_de_arquivo` |
| Micro C | `microc_mapa.c`, `microc/scanner.c` | tabela literal → `TokenType`, adaptador do scanner | `microc_carregar_tokens_h`; ajustes ao seu TP1 |

Módulos de apoio prontos: `util.c` (alocação e diagnósticos), `bitset.c` (conjuntos de terminais), `gramatica.c` (estrutura da gramática, impressão em EBNF) e `main.c` (ordem das etapas).

Comece por `gramatica.h`, que descreve a árvore `Expr` usada em todas as etapas.

## Integração com o scanner do TP1 (seção 3.1 do enunciado)

1. Copie `microc.flex`, `tokens.h` e os demais `.c`/`.h` do seu TP1, como a tabela de strings, para `microc/`.
2. No `microc.flex`, use `%option prefix="microc_yy" noyywrap` e devolva `END_OF_FILE` na regra `<<EOF>>`.
3. Ajuste `microc/scanner.c` ao seu TP1: lexema vindo da tabela de strings e mensagem de erro de `error_msg` no token UNDEF.
4. Rode `make microc` e `make test`.

A gramática `testes/gramaticas/microc.ebnf` é a do Apêndice B do livro-texto, fatorada para LL(1). O `else` pendente gera um aviso de conflito em `[ "else" comando ]`. O conflito é resolvido de forma gulosa: o `else` fica com o `if` mais próximo.

## Testes

`testes/executar.sh` compara a saída do gerador (`testes/esperado/*.gerador`) e a dos analisadores gerados (`testes/programas/<gramatica>/*.esperado`) com as saídas esperadas. Cada caso termina com a linha `status <código de saída>`. Acrescente seus próprios casos: o enunciado pede pelo menos 5 gramáticas e 3 programas aceitos e 3 rejeitados por gramática válida.

Os programas de Micro C só rodam depois de `make microc`. O caso `rejeitado_lexico.mc` espera a mensagem "String nao terminada" do seu TP1.
