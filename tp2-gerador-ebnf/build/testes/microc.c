/*
 * Analisador sintatico descendente recursivo gerado a partir de testes/gramaticas/microc.ebnf.
 * Arquivo gerado automaticamente: nao edite.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "microc.h"

#define SUPORTE_ARVORE 0

/* ---- conjuntos de tokens ---- */


/* ---- estado do analisador ---- */

static Token tok;                  /* token corrente (lookahead) */
static int imprimir_arvore = 0;    /* opcao --tree */
static int nivel_arvore = 0;
static FILE *arquivo_tokens = NULL; /* opcao --tokens */

static Token token_de_arquivo(void);

const char *nome_token(int tipo)
{
    switch (tipo) {
    case END_OF_FILE: return "fim de arquivo";
    case INT: return "'int'";
    case MAIN: return "'main'";
    case LPAREN: return "'('";
    case RPAREN: return "')'";
    case LBRACE: return "'{'";
    case RBRACE: return "'}'";
    case ID: return "ID";
    case LBRACKET: return "'['";
    case INTEGERCONST: return "INTEGERCONST";
    case RBRACKET: return "']'";
    case SEMICOLON: return "';'";
    case CHAR: return "'char'";
    case IF: return "'if'";
    case ELSE: return "'else'";
    case FOR: return "'for'";
    case RETURN: return "'return'";
    case PRINT: return "'print'";
    case ASSIGN: return "'='";
    case STRINGCONST: return "STRINGCONST";
    case AND: return "'&&'";
    case OR: return "'||'";
    case EQ: return "'=='";
    case NEQ: return "'!='";
    case LT: return "'<'";
    case GT: return "'>'";
    case LEQ: return "'<='";
    case GEQ: return "'>='";
    case PLUS: return "'+'";
    case MINUS: return "'-'";
    case MUL: return "'*'";
    case DIV: return "'/'";
    case MOD: return "'%'";
    case CHARCONST: return "CHARCONST";
    case NOT: return "'!'";
    default: return "token desconhecido";
    }
}

static void escrever_posicao(void)
{
    fprintf(stderr, "linha %d: ", tok.linha);
}

/* Le o proximo token para tok. */
void avancar(void)
{
    if (arquivo_tokens != NULL)
        tok = token_de_arquivo();
    else
        tok = proximo_token();
    if (tok.tipo == UNDEF) {
        fprintf(stderr, "linha %d: erro lexico: %s\n", tok.linha, tok.lexema);
        exit(EXIT_FAILURE);
    }
}

/* ---- primitivas ---- */

int pertence(int tipo, const int *conjunto)
{
    for (; *conjunto != -1; conjunto++)
        if (*conjunto == tipo)
            return 1;
    return 0;
}

/* Reporta o erro com os tokens esperados (lista terminada em -1) e encerra. */
void erro_sintatico(const int *esperados)
{
    int i;
    escrever_posicao();
    if ((int) tok.tipo == TOKEN_FIM)
        fprintf(stderr, "encontrado fim de arquivo, ");
    else
        fprintf(stderr, "encontrado '%s', ", tok.lexema);
    if (esperados[0] != -1 && esperados[1] == -1) {
        fprintf(stderr, "esperado %s\n", nome_token(esperados[0]));
    } else {
        fprintf(stderr, "esperado um de {");
        for (i = 0; esperados[i] != -1; i++)
            fprintf(stderr, "%s%s", i > 0 ? ", " : "", nome_token(esperados[i]));
        fprintf(stderr, "}\n");
    }
    exit(EXIT_FAILURE);
}

static void recuo(void)
{
    int i;
    for (i = 0; i < nivel_arvore; i++)
        fputs("  ", stdout);
}

void arvore_entra(const char *nao_terminal)
{
    if (imprimir_arvore) {
        recuo();
        printf("%s\n", nao_terminal);
    }
    nivel_arvore++;
}

void arvore_sai(void)
{
    nivel_arvore--;
}

/* Consome o token corrente se for do tipo esperado; senao, erro. */
void match(int esperado)
{
    int lista[2];
    if ((int) tok.tipo != esperado) {
        lista[0] = esperado;
        lista[1] = -1;
        erro_sintatico(lista);
    }
    if (SUPORTE_ARVORE && imprimir_arvore) {
        recuo();
        if (nome_token(tok.tipo)[0] == '\'')
            printf("%s\n", nome_token(tok.tipo));
        else
            printf("%s '%s'\n", nome_token(tok.tipo), tok.lexema);
    }
    avancar();
}

static Token token_de_arquivo(void)
{
    Token t;
    memset(&t, 0, sizeof t);
    fprintf(stderr, "opcao --tokens ainda nao implementada\n");
    exit(EXIT_FAILURE);
    return t;
}

/* ---- procedimentos (um por nao terminal) ---- */

/* programa = 'int' 'main' '(' ')' bloco ; */
void parse_programa(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* bloco = '{' { declaracao } { comando } '}' ; */
void parse_bloco(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* declaracao = tipo ID [ '[' INTEGERCONST ']' ] ';' ; */
void parse_declaracao(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* comando = comando_if | comando_for | atribuicao | comando_return | comando_print | bloco | ';' ; */
void parse_comando(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* tipo = 'int' | 'char' ; */
void parse_tipo(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* comando_if = 'if' '(' expressao ')' comando [ 'else' comando ] ; */
void parse_comando_if(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* comando_for = 'for' '(' atribuicao_interna ';' expressao ';' atribuicao_interna ')' comando ; */
void parse_comando_for(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* atribuicao = atribuicao_interna ';' ; */
void parse_atribuicao(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* comando_return = 'return' [ expressao ] ';' ; */
void parse_comando_return(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* comando_print = 'print' '(' conteudo_print ')' ';' ; */
void parse_comando_print(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* expressao = expressao_logica ; */
void parse_expressao(void)
{
    parse_expressao_logica();
}

/* atribuicao_interna = ID [ '[' expressao ']' ] '=' expressao ; */
void parse_atribuicao_interna(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* conteudo_print = STRINGCONST | expressao ; */
void parse_conteudo_print(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* expressao_logica = expressao_relacional { ( '&&' | '||' ) expressao_relacional } ; */
void parse_expressao_logica(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* expressao_relacional = expressao_aritmetica [ operador_relacional expressao_aritmetica ] ; */
void parse_expressao_relacional(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* expressao_aritmetica = termo { ( '+' | '-' ) termo } ; */
void parse_expressao_aritmetica(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* operador_relacional = '==' | '!=' | '<' | '>' | '<=' | '>=' ; */
void parse_operador_relacional(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* termo = fator { ( '*' | '/' | '%' ) fator } ; */
void parse_termo(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* fator = '(' expressao ')' | ID [ '[' expressao ']' ] | INTEGERCONST | CHARCONST | '!' fator ; */
void parse_fator(void)
{
    /* TODO(aluno): construcao ainda nao traduzida */
}

/* ---- programa principal ---- */

int main(int argc, char **argv)
{
    int i, fim[2];
    const char *arquivo = NULL, *tokens = NULL;
    FILE *entrada = stdin;

    for (i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--tree") == 0) {
            imprimir_arvore = 1;
        } else if (strcmp(argv[i], "--tokens") == 0 && i + 1 < argc) {
            tokens = argv[++i];
        } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
            fprintf(stderr, "uso: %s [--tree] [--tokens arquivo.tok | programa]\n", argv[0]);
            return 2;
        } else {
            arquivo = argv[i];
        }
    }
    if (imprimir_arvore && !SUPORTE_ARVORE)
        fprintf(stderr, "aviso: gere o analisador com --tree para imprimir a arvore\n");

    if (tokens != NULL) {
        arquivo_tokens = fopen(tokens, "r");
        if (arquivo_tokens == NULL) {
            fprintf(stderr, "nao foi possivel abrir %s\n", tokens);
            return 2;
        }
    } else {
        if (arquivo != NULL && (entrada = fopen(arquivo, "r")) == NULL) {
            fprintf(stderr, "nao foi possivel abrir %s\n", arquivo);
            return 2;
        }
        scanner_iniciar(entrada);
    }

    avancar();
    parse_programa();
    if (tok.tipo != TOKEN_FIM) {
        fim[0] = TOKEN_FIM;
        fim[1] = -1;
        erro_sintatico(fim);
    }
    printf("entrada aceita\n");

    if (arquivo_tokens != NULL) {
        fclose(arquivo_tokens);
    } else {
        scanner_finalizar();
        if (entrada != stdin)
            fclose(entrada);
    }
    return 0;
}
