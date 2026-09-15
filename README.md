Pedro Paulo de Oliveira Andrade: 202419040144.

Pedro e Silva Candia: 202319050687

uso_ia: sim

# Trabalho de Compiladores I - Parte 1

# Trabalho Prático 1: Análise Léxica para Micro C

Este projeto implementa o analisador léxico para a linguagem Micro C utilizando a ferramenta Flex e o livro do professor Brivaldo. O analisador é capaz de reconhecer palavras reservadas, identificadores, números inteiros, caractere e strings, além de processar operadores matemáticos e símbolos gramaticais no geral.

Durante o desenvolvimento, adotamos algumas estratégias específicas para lidar com os requisitos da linguagem:
1. **Tabela de Identificadores (Hash Table):**
   Para armazenar os lexemas de maneira eficiente e evitar repetições na memória, implementamos uma Tabela Hash com tratamento de colisões por encadeamento (lista ligada). Utilizamos o tamanho 211 (um número primo) para otimizar a dispersão dos dados e a função `djb2` adaptada para o cálculo dos índices.

2. **Ambiguidade do Sinal de Menos (Lookahead):**
   Para remover a ambiguidade do sinal de menos '-' e não perder o determinismo do analisador, implementamos uma variável ultimo_token no escopo global. Através da função `retorna_token()`, o estado é sempre atualizado com o último token válido retornado. Ao encontrar o padrão `"-"[0-9]+`, o analisador verifica se o termo antecessor exige um operador matemático como `ID`, `INTEGERCONST`, `RPAREN` ou `RBRACKET`, e se sim, o Flex utiliza a função `yyless(1)` para devolver os dígitos à fita e retorna `MINUS`, caso contrário, consome o número todo como `INTEGERCONST`.

3. **Tratamento de Strings e Caracteres:**
   Para sequencias como`\n`, `\t`, `\\`, `\"`, `\0`, implementamos um algoritmo de conversão com dois ponteiros, `leitor` e `escritor`. As strings e os caracteres são processados substituindo as sequências pelos seus respectivos valores ASCII no próprio array `yytext`, truncando as aspas originais com o caractere nulo (`\0`) antes de enviar o dado limpo para a função `guarda_lexema()`.

Para compilar e executar basta seguir as mesmas instruções que já tinham sido definidas no arquivo original:

flex microc.flex

gcc lex.yy.c -o lexer

./lexer test.mc
