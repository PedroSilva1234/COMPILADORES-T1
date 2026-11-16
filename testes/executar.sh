#!/bin/sh
# Testes automatizados do gerador. Execute a partir da raiz do projeto:
#     make test      (ou: sh testes/executar.sh)
#
# 1. Gramaticas: para cada testes/gramaticas/X.ebnf com um arquivo
#    testes/esperado/X.gerador, roda o gerador com --sets e compara a
#    saida (stdout + stderr + codigo de saida) com o esperado.
# 2. Programas: para cada pasta testes/programas/X, gera e compila o
#    analisador da gramatica X e roda cada entrada, comparando a saida
#    com o arquivo .esperado ao lado dela.
#    A pasta microc so roda se build/microc/parser_microc existir
#    (make microc, com os arquivos do TP1 em microc/).
#
# Para criar um caso novo, escreva a entrada, confira a saida do seu
# analisador a mao e salve-a como .esperado.

GERADOR=./gerador
SAIDA=build/testes
CC=${CC:-gcc}
CFLAGS=${CFLAGS:-"-ansi -pedantic -Wall -Wextra"}

total=0
falhas=0
mkdir -p "$SAIDA"

comparar() {
    # $1 = nome do caso, $2 = saida obtida, $3 = saida esperada
    total=$((total + 1))
    if diff -u "$3" "$2" > "$2.diff" 2>&1; then
        echo "ok     $1"
        rm -f "$2.diff"
    else
        echo "FALHA  $1   (diferencas em $2.diff)"
        falhas=$((falhas + 1))
    fi
}

executar() {
    # executa "$@" gravando stdout+stderr e o codigo de saida em $SAIDA_ATUAL
    "$@" > "$SAIDA_ATUAL" 2>&1
    echo "status $?" >> "$SAIDA_ATUAL"
}

echo "== gramaticas"
for g in testes/gramaticas/*.ebnf; do
    nome=$(basename "$g" .ebnf)
    esperado=testes/esperado/$nome.gerador
    [ -f "$esperado" ] || continue
    opcoes=""
    grep -q "TOKENS EXTERN" "$g" && opcoes="--scanner microc"
    SAIDA_ATUAL=$SAIDA/$nome.gerador
    executar $GERADOR "$g" -o "$SAIDA/$nome" --sets $opcoes
    comparar "gramatica $nome" "$SAIDA_ATUAL" "$esperado"
done

echo "== programas"
for pasta in testes/programas/*/; do
    nome=$(basename "$pasta")
    if [ "$nome" = microc ]; then
        analisador=build/microc/parser_microc
        if [ ! -x "$analisador" ]; then
            echo "pulado programas/microc (rode make microc com os arquivos do TP1)"
            continue
        fi
    else
        analisador=$SAIDA/$nome.bin
        if ! $GERADOR "testes/gramaticas/$nome.ebnf" -o "$SAIDA/$nome" > /dev/null 2>&1 \
           || ! $CC $CFLAGS -o "$analisador" "$SAIDA/$nome.c" > "$SAIDA/$nome.cc" 2>&1; then
            total=$((total + 1))
            falhas=$((falhas + 1))
            echo "FALHA  programas/$nome: nao foi possivel gerar ou compilar o analisador"
            continue
        fi
        if [ -s "$SAIDA/$nome.cc" ]; then
            echo "aviso  programas/$nome: o codigo gerado produziu avisos de compilacao:"
            sed 's/^/       /' "$SAIDA/$nome.cc"
        fi
    fi
    for entrada in "$pasta"*; do
        case "$entrada" in *.esperado) continue ;; esac
        esperado=${entrada%.*}.esperado
        [ -f "$esperado" ] || continue
        SAIDA_ATUAL=$SAIDA/$nome-$(basename "$entrada").saida
        executar "$analisador" "$entrada"
        comparar "programas/$nome/$(basename "$entrada")" "$SAIDA_ATUAL" "$esperado"
    done
done

echo
echo "$((total - falhas)) de $total testes passaram"
[ "$falhas" -eq 0 ]
