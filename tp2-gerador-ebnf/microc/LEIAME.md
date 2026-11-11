# Pasta microc/

Aqui fica o scanner de Micro C do TP1, convertido em função para o analisador gerado com `--scanner microc` (seção 3.1 do enunciado).

Já fornecidos:

- `scanner.h`: interface usada pelo analisador gerado (`scanner_iniciar`, `proximo_token`, `scanner_finalizar`).
- `scanner.c`: adaptador entre `microc_yylex()` e `Token`. Procure os `TODO(aluno)`.

Copie do seu TP1:

- `microc.flex`, com `%option prefix="microc_yy" noyywrap` e `return END_OF_FILE;` na regra `<<EOF>>`;
- `tokens.h`, com `TokenType` e `Token` como no livro-texto (seção 2.3.1);
- os demais arquivos `.c`/`.h` de que o scanner depende, como a tabela de strings.

O Makefile compila todos os `.c` desta pasta. O `lex.yy.c` gerado é compilado sem a exigência de ausência de avisos.

Cuidado com tokens devolvidos pelo código ASCII (TP1, seção 5). Valores até o último item de `TokenType` (36 no `tokens.h` do livro) são sempre interpretados como `TokenType`. Por isso `!` (33) deve ser devolvido como `NOT`, e não como `'!'`.
