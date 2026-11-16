# Trabalho Prático 2 - Gerador de Analisadores Sintáticos EBNF

aluno: Pedro Andrade - 2024.1904.014-4
aluno: Pedro Candia  - 2023.1905.068-7
uso_ia: sim

## Descrição do Projeto

Este projeto implementa um gerador de analisadores sintáticos descendentes recursivos (`./gerador`) em ANSI C (C89) a partir de gramáticas escritas na notação EBNF. O sistema realiza a análise léxica e sintática da gramática de entrada, executa verificações semânticas (símbolos indefinidos, não-terminais inalcançáveis, improdutivos e recursão à esquerda direta ou indireta), calcula os atributos `anulavel`, `FIRST` e `FOLLOW` por ponto fixo, valida as condições $LL(1)$ e emite o código-fonte completo (`.c` e `.h`) do analisador sintático e léxico correspondente.

## Arquitetura e Decisões de Implementação

1. **Análise de Expressões EBNF (`src/ebnf_parser.c`):**
   Implementação descendente recursiva das funções `parse_expr`, `parse_seq` e `parse_fator` para construção da árvore sintática abstrata (`E_ALT`, `E_SEQ`, `E_OPC`, `E_REP`, `E_TERM`, `E_NAOTERM` e `E_VAZIO`).

2. **Verificações Semânticas e Conjuntos (`src/verificacao.c` e `src/conjuntos.c`):**
   Detecção de não-terminais improdutivos e inalcançáveis, busca em profundidade (DFS) com marcação de pilha para identificar ciclos de recursão à esquerda e algoritmos iterativos de ponto fixo usando `Bitset` para computar `anulavel`, `FIRST` e `FOLLOW` de não-terminais e subexpressões.

3. **Verificação $LL(1)$ (`src/ll1.c`):**
   Verificação de disjunção dois a dois entre conjuntos `FIRST` de alternativas, unicidade de alternativa anulável, disjunção entre `FIRST` e `FOLLOW` na presença de alternativa anulável e tratamento de conflitos em estruturas opcionais `[ ... ]` e repetitivas `{ ... }` como avisos resolvidos de forma gulosa (como o *else* pendente em Micro C).

4. **Geração do Analisador e Motor de Expressões Regulares (`src/codegen.c`):**
   * **Tradução de Regras (`gen_expr`):** Emissão de desvios condicionais (`if / else if / else`), laços (`while`) e chamadas de erro sintático com conjuntos de tokens esperados.
   * **Léxico Próprio (`gen_casamento_regex`):** Compilação das expressões regulares da seção `TOKENS` em um Autômato Finito Não-Determinístico (construção de Thompson) seguida da determinização por construção de subconjuntos (AFD), emitindo tabelas de transição estáticas (`trans` e `is_final`) em ANSI C para reconhecimento do maior prefixo.
   * **Leitura de Arquivo de Tokens (`gen_token_de_arquivo`):** Suporte à flag `--tokens` para leitura de fluxos de tokens pré-processados no formato do TP1.

## Adaptações e Correções no Scanner desde o TP1 (`microc/`)

Para integrar o analisador léxico desenvolvido no TP1 ao modo `--scanner microc` do TP2, foram realizadas as seguintes adaptações na pasta `microc/`:

1. **Separação do Cabeçalho `microc/tokens.h`:**
   No TP1, a enumeração `TokenType` estava declarada internamente no arquivo `microc.flex`. Para o TP2, extraímos `TokenType` e a definição de `Token` para `microc/tokens.h`, permitindo que `src/microc_mapa.c` inspecione a enumeração e que o código gerado compartilhe os mesmos identificadores.
2. **Configuração de Prefixo e `noyywrap` no Flex:**
   Adicionamos a diretiva `%option prefix="microc_yy" noyywrap` ao `microc/microc.flex` e removemos as funções `yywrap()` e `main()` originais do TP1 para evitar colisão de símbolos durante a linkedição com o analisador gerado.
3. **Gerenciamento de Memória da Tabela Hash (`liberar_tabela_hash`):**
   Implementamos a função `liberar_tabela_hash()` em `microc.flex` e a invocamos em `scanner_finalizar()` (`microc/scanner.c`), garantindo a desalocação completa dos nós e lexemas ao término da análise (validado com zero vazamentos no Valgrind).
4. **Mapeamento de Erros Léxicos e Ajuste de Linha em `microc/scanner.c`:**
   Adaptamos `texto_do_lexema()` para repassar `microc_yylval.error_msg` quando o token retornado é `UNDEF`. Além disso, ajustamos a contabilização de linha em `proximo_token()` (`linha_atual - 1`) para o erro `"String nao terminada"`, cuja regra no Flex consome o caractere `\n` ao detectar a quebra de linha não escapada.

## Gramáticas e Casos de Teste Adicionados

Além das gramáticas fornecidas no pacote (`expr.ebnf`, `microc.ebnf`, `conflito_ll1.ebnf` e `recursao_esquerda.ebnf`), criamos uma 3ª gramática válida:
* **`testes/gramaticas/config.ebnf`:** Linguagem para arquivos de configuração estruturados em seções `[id]`, atribuições `chave = valor;` e tuplas recursivas aninhadas `(a, b, ())`, acompanhada de 3 programas aceitos (`aceito_1.txt` a `aceito_3.txt`) e 3 programas rejeitados (`rejeitado_1.txt` a `rejeitado_3.txt`) em `testes/programas/config/`.

## Como Compilar e Executar

```bash
# Compilar o gerador e o analisador de Micro C
make clean && make

# Executar toda a bateria de testes automatizados (24 testes)
make test

# Gerar e compilar manualmente um analisador para uma gramatica EBNF
./gerador testes/gramaticas/config.ebnf -o build/config --sets --tree
gcc -ansi -pedantic -Wall -Wextra -o build/config_bin build/config.c
./build/config_bin --tree testes/programas/config/aceito_1.txt