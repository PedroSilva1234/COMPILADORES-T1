# Registro de Uso de Inteligência Artificial (TP2)

Ferramenta utilizada: **Google Gemini, Claude code**

## Etapas em que a ferramenta foi utilizada

1. **Análise das expressões EBNF (parse_expr, parse_seq e parse_fator em src/ebnf_parser.c)**
   Auxílio: Explicação da lógica do analisador descendente recursivo para a notação EBNF (uma função por regra, decisão pelo token corrente, forma da árvore). Emitindo as regras de maneira correta iguais as dos arquivos teste, a ferramenta apontou e ajudou a corrigir erros de compilação (uso de esperar, variável inexistente, declarações incompatíveis com C90, nomes de tokens)

2. **Gerador de Expressões Regulares (`canto_esquerdo e verifc_recursao_esquerda, verif_improdutivo` em `src/verificacao.c):**
   Auxílio para a implementação e correções da verificação de improdutivos  e recursão a esquerda para a verificação as gramáticas passadas pela entrada, emitindo avisos e erros quando necessário.

3. **Cálculos dos conjuntos FOLLOW e FIRST(`expr_follow, conj_follow, expr_first, conj_first` em `src/conjuntos.c`):**
   Explicação da teoria aplicada à árvore de expressões e do uso e entendimento dos arquivos bitset.c e bitset.h que foram usados para a implementação dos cálculos dos dois conjuntos e do conjunto anulável, além de esclarecer duvidas de lógica da implementação dos calculos feitos de acordo com a teoria aprendida em sala.


4. **Condição LL(1)(` verificar_expr e ll1_verificar` em `src/ll1.c`)**
   Auxílio para na implementação para a verificação das linguagens LL(1). Foram apontadas a visita duplicada dos filhos em E_SEQ (que repetia erros e avisos, por exemplo o aviso do else pendente) e a diferença entre as mensagens e o texto esperado em testes/esperado/microc.gerador e no manual.

5. **Tradução das regras (`gen_expr` em `src/codegen.c`):**
   Auxílio no entendimento do código codegen.c e correção de bugs para a implementação do tradutor do nosso gerador, que percorre a Árvore de Sintaxe Abstrata(AST) para gerar um Analisador Sintático Descendente Recursivo

6. **Motor de Expressões Regulares (`gen_casamento_regex` em `src/codegen.c`):**
   Auxílio na estruturação em ANSI C (C89) do algoritmo de conversão de expressões regulares em Autômato Finito Não-Determinístico (AFN de Thompson) e subsequente determinização por construção de subconjuntos (AFD), emitindo as tabelas de estados e transições para reconhecer o maior prefixo dos tokens.

7. **Leitura de Arquivo de Tokens (`gen_token_de_arquivo` em `src/codegen.c`):**
   Auxílio na implementação da rotina emitida para realizar o parsing seguro das linhas no formato `Token: tipo = <n>, lexema = '<texto>', linha = <n>` utilizando manipulação de ponteiros (`strstr`, `strrchr` e `sscanf`) compatível com `-ansi -pedantic`.

8. **Mapeamento e Integração com o TP1 (`src/microc_mapa.c` e `microc/scanner.c`):**
   Auxílio na implementação do mini-scanner em `microc_carregar_tokens_h` para extrair identificadores da enumeração `TokenType` ignorando comentários C/C++, além da adaptação de `microc/scanner.c` e da função `liberar_tabela_hash()` para integração sem vazamentos de memória com o analisador léxico do TP1.

9. **Depuração de Avisos e Documentação:**
   Revisão de mensagens de diagnóstico em `src/ll1.c`, elaboração da gramática de teste adicional (`config.ebnf`).

Todo o código sugerido foi revisado, compreendido, integrado e testado pela dupla com (`make test`) e com a ferramenta `valgrind` para verificar vazamento de memória.