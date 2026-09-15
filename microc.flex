/*
 * microc.flex
 *
 * Esqueleto do analisador lexico (scanner) para a linguagem Micro C.
 * Disciplina: Compiladores I - FACOM
 *
 * Este arquivo NAO esta completo. Partes do reconhecimento de tokens
 * foram implementadas apenas como EXEMPLO, para orienta-lo(a) sobre o
 * padrao a seguir. As demais estao marcadas com "TODO(aluno)" e devem
 * ser completadas por voce.
 *
 * Compilacao:
 *      flex microc.flex
 *      gcc lex.yy.c -o lexer
 *
 * Uso:
 *      ./lexer test.mc
 */

%{
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>

    /* ---------------------------------------------------------------------
    * 1. VOCABULARIO DE TOKENS (equivalente a tokens.h)
    * ------------------------------------------------------------------- */
    
    #define TAMANHO_HASH 211 /* Um número primo para reduzir as colisoes*/
    
    typedef struct Symbol
    {
        char *lexema;
    } Symbol;

    typedef struct Tabela_IDENTIFICADORES
    {
        Symbol valor; 

        struct Tabela_IDENTIFICADORES *prox; /* O 'struct' aqui é obrigatório em C */

    } Tabela_ID;


    /* O vetor global que representa a tabela hash */
    Tabela_ID *Tabela_IDEN[TAMANHO_HASH] = {NULL};

    typedef enum 
    {
        /* Tokens fundamentais */
        UNDEF,          /* token indefinido (usado para reportar erros) */
        ID,             /* identificador                                */
        END_OF_FILE,    /* fim de arquivo                               */

        /* Constantes literais */
        INTEGERCONST,
        CHARCONST,
        STRINGCONST,

        /* Operadores aritmeticos */
        PLUS, MINUS, MUL, DIV, MOD,

        /* Operadores relacionais e logicos */
        EQ, NEQ, LT, GT, LEQ, GEQ, AND, OR, NOT,

        /* Simbolos de atribuicao e pontuacao */
        ASSIGN, SEMICOLON, COMMA, LPAREN, RPAREN,
        LBRACE, RBRACE, LBRACKET, RBRACKET,

        /* Palavras reservadas */
        MAIN, IF, ELSE, FOR, RETURN, INT, CHAR, PRINT
    } TokenType;

    int ultimo_token = UNDEF;

    /* Nomes dos tokens, usados apenas pelo main() de teste abaixo para imprimir o tipo de cada token de forma legivel. Mantenha esta lista
    * na MESMA ORDEM do enum TokenType. */

    static const char *nome_token[] = 
    {
        "UNDEF", "ID", "END_OF_FILE",
        "INTEGERCONST", "CHARCONST", "STRINGCONST",
        "PLUS", "MINUS", "MUL", "DIV", "MOD",
        "EQ", "NEQ", "LT", "GT", "LEQ", "GEQ", "AND", "OR", "NOT",
        "ASSIGN", "SEMICOLON", "COMMA", "LPAREN", "RPAREN",
        "LBRACE", "RBRACE", "LBRACKET", "RBRACKET",
        "MAIN", "IF", "ELSE", "FOR", "RETURN", "INT", "CHAR", "PRINT"
    };

    /* Valor semantico do token corrente. */
    typedef struct 
    {
        char *symbol;      /* lexema para ID, INTEGERCONST, CHARCONST, STRINGCONST */
        char *error_msg;   /* mensagem de erro, usada apenas quando tipo == UNDEF  */
    } YYSTYPE;

    YYSTYPE microc_yylval;

    /* Linha atual do arquivo-fonte sendo processada. Deve ser incrementada toda vez que uma quebra de linha for consumida pelo scanner (seja em
    * codigo "normal", dentro de comentarios ou dentro de strings). */
    int linha_atual = 1;

    /* Função auxiliar para registrar o histórico e retornar o token */
    static inline int retorna_token(int tok) 
    {
        ultimo_token = tok;
        return tok;
    }

    /*Funcao de dispersão*/
    unsigned long FuncaoHash(char* str)
    {   
        unsigned long hash = 5381;
        int c;
        /*gera um numero com a string*/
        while ((c = *str++)) {
            hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
        }

        return hash % TAMANHO_HASH;

    }

    /* Funcao auxiliar para preencher microc_yylval.symbol com uma copia do texto reconhecido (yytext). Foi criado
    uma tabela hash para implementacao da tabela de identificadores, todos os IDs tem tipo Symbol. */
    static void guarda_lexema(void) 
    {

        unsigned long indice = FuncaoHash(yytext);   
        Tabela_ID *atual = Tabela_IDEN[indice];

        while(atual!= NULL)
        {   
            /*Vê se o token já está guardado na tabela*/
            if(strcmp(atual->valor.lexema,yytext) == 0)
            {
                microc_yylval.symbol = atual->valor.lexema;
                return;
            }
            else
                atual = atual->prox;
        }

        /*caso não esteja cria um novo nó*/
        Tabela_ID *novo_no = (Tabela_ID*) malloc(sizeof (Tabela_ID));
        /*define o lexema, e conecta o novo no na posicao Indice*/
        novo_no->valor.lexema = strdup(yytext);
        novo_no->prox = Tabela_IDEN[indice];
        Tabela_IDEN[indice] = novo_no;
        microc_yylval.symbol = novo_no->valor.lexema;

        return;
    }
    /*Função para checar as palavras reservadas*/
    TokenType Checa_Palavra_Reservada(const char *lexema) 
    {
        const char *palavras_R[] = {"main", "if", "else", "for", "return", "int", "char", "print"};
        TokenType tokens_R[] = {MAIN, IF, ELSE, FOR, RETURN, INT, CHAR, PRINT};
        
        for(int i = 0; i < 8; i++) 
        {
            if (strcmp(lexema, palavras_R[i]) == 0) 
            {
                return tokens_R[i];
            }
        }
        return ID;
    }


%}

/* 2. SECAO DE DEFINICOES ------------------------------------- */

DIGIT       [0-9]
LETRA       [a-zA-Z_]
ALFANUM     [a-zA-Z0-9_]

%x COMMENT

%%

 /* -----------------------------------------------------------------------
  * 3. SECAO DE REGRAS
  * --------------------------------------------------------------------- */

 /* --- Fim de arquivo -----------------------------------------------------
  * Tratada explicitamente (em vez de depender do retorno automatico 0 do flex), pois o token UNDEF tambem vale 0 no enum TokenType -- se
  * dependessemos do comportamento padrao, um erro lexico seria confundido com o fim do arquivo pelo main() de teste abaixo. */
<INITIAL><<EOF>>             { return END_OF_FILE; }

 /* --- Espacos em branco e quebras de linha ---------------------------- */
\n                           { linha_atual++; }
[ \t\r]+                     { /* ignora espacos em branco */ }


 /* --- Comentarios ------------------------------------------------------
  * Estes ja estao implementados como exemplo de uso de estados (%x) e de tratamento de erro via EOF dentro de um estado especial. */
"//".*                       { /* comentario de linha: ignora ate o fim da linha */ }

"/*"                         { BEGIN(COMMENT); }
<COMMENT>"*/"                { BEGIN(INITIAL); }
<COMMENT>\n                  { linha_atual++; }
<COMMENT><<EOF>>             {
                                microc_yylval.error_msg = "EOF em comentario";
                                return UNDEF;
                             }
<COMMENT>.                   { /* consome qualquer outro caractere dentro do comentario */ }

 /* Fechamento de comentario sem abertura correspondente. */
"*/"                         {
                                microc_yylval.error_msg = "Comentario nao iniciado";
                                return UNDEF;
                             }

 /* --- Palavras reservadas e identificadores ----------------------------
  * DONE
*/
{LETRA}{ALFANUM}*            {
                                TokenType tokenT = Checa_Palavra_Reservada(yytext);
                                
                                if (tokenT == ID) {
                                    guarda_lexema();
                                }
                                
                                return retorna_token(tokenT);
                             }

  /* --- Constantes inteiras -----------------------------------------------
  * DONE
  * TODO(aluno): o padrao formal para um inteiro em Micro C e um ou mais
  * digitos, opcionalmente precedidos de um sinal de menos (numeros
  * negativos). A regra abaixo trata apenas inteiros sem sinal; use a
  * tecnica de lookahead discutida em aula (veja o operador MINUS mais
  * abaixo) para decidir quando um '-' faz parte do numero e quando ele
  * e, na verdade, o operador de subtracao. */
{DIGIT}+            {
                        guarda_lexema();
                        return retorna_token(INTEGERCONST);
                    }

"-"[0-9]+           { //Faz o filtro de -DIGIT para saber se é operando ou sinal
                        if (ultimo_token == ID || ultimo_token == INTEGERCONST || 
                            ultimo_token == RPAREN || ultimo_token == RBRACKET) {
                            yyless(1); 
                            return retorna_token(MINUS);
                        } else {
                            guarda_lexema();
                            return retorna_token(INTEGERCONST);
                        }
                    }
                    
 /* --- Constantes de caractere --------------------------------------------

   * DONE
  *A expressao inicial segue como, ' exceto outra ';  qualquer caracter, numero ou operador; '
  *A segunda segue o tratamento de erro para o CHARCONST
  */

'([^'\\\n\0]|\\.)'  {
                        /*Trata o char se houer uma barra invertida. Ex.: '\n' ou '\0'*/
                        if (yytext[1] == '\\') {
                            switch(yytext[2]) {
                                case 'n':  yytext[0] = '\n'; break;
                                case 't':  yytext[0] = '\t'; break;
                                case '\\': yytext[0] = '\\'; break;
                                case '\'': yytext[0] = '\''; break;
                                case '0':  yytext[0] = '\0'; break;
                                default:   yytext[0] = yytext[2]; break;
                            }
                        } else {
                            yytext[0] = yytext[1];
                        }
                        
                        /* Trunca a string, mantendo apenas o caractere e o terminador nulo */
                        yytext[1] = '\0';
                        
                        guarda_lexema();
                        return retorna_token(CHARCONST);
                    }

'[^'\n]*            { 
                        microc_yylval.error_msg = "Constante de caractere nao fechada";
                        return retorna_token(UNDEF);
                    }


 /* --- Constantes de string -------------------------------------------
  * DONE
  * TODO(aluno): reconhecer o padrao "[^"\n]*" (uma ou mais aspas
  * duplas delimitando o conteudo da string) e devolver STRINGCONST.
  * Voce deve tratar os seguintes erros (veja o enunciado, Secao 4.1):
  *   - EOF antes do fechamento da string ("EOF em string")
  *   - quebra de linha nao escapada dentro da string
  *     ("String nao terminada")
  *   - caractere nulo dentro da string
  *     ("String contem caractere nulo")
  * Alem disso, converta as sequencias de escape (\n, \t, \\, \", \0)
  * para os caracteres correspondentes antes de armazenar o lexema. */

  
\"([^"\\\n\0]|\\.)*\"       {
                                /* O leitor comeca em 1 para pular a aspa dupla de abertura */
                                int leitor = 1; 
                                int escritor = 0;
                                
                                /* Percorre o yytext ate encontrar a aspa de fechamento */
                                while (yytext[leitor] != '"') {
                                    if (yytext[leitor] == '\\') {
                                        leitor++; /* Avanca para inspecionar o caractere de escape */
                                        switch(yytext[leitor]) {
                                            case 'n':  yytext[escritor++] = '\n'; break;
                                            case 't':  yytext[escritor++] = '\t'; break;
                                            case '\\': yytext[escritor++] = '\\'; break;
                                            case '"':  yytext[escritor++] = '\"'; break;
                                            case '0':  yytext[escritor++] = '\0'; break;
                                            default:   yytext[escritor++] = yytext[leitor]; break;
                                        }
                                    } else {
                                        /* Caractere normal, apenas copia na posicao do escritor */
                                        yytext[escritor++] = yytext[leitor];
                                    }
                                    leitor++;
                                }
                                
                                /* Adiciona o terminador nulo real no final da string convertida.
                                 * Isso efetivamente remove a aspa dupla de fechamento do final. */
                                yytext[escritor] = '\0';
                                
                                guarda_lexema();
                                return retorna_token(STRINGCONST);
                            }

 /* 2. Erro: Caractere nulo (0x00 real do arquivo) no meio da string. */
 /*
 Começa com aspas duplas obrigatoriamente,[^*] o chapéu serve como negação para tudo que está dentro do colchetes
 | -> OR
 Barra invertida seguida de qualquer coisa.
 */
\"([^"\\\n\0]|\\.)*\0       {
                                microc_yylval.error_msg = "String contem caractere nulo";
                                return retorna_token(UNDEF);
                            }

 /* 3. Erro: Quebra de linha nao escapada dentro da string. */
\"([^"\\\n\0]|\\.)*\n       {
                                linha_atual++; /* Consome a quebra de linha para manter a contagem correta */
                                microc_yylval.error_msg = "String nao terminada";
                                return retorna_token(UNDEF);
                            }

 /* 4. Erro: Fim do arquivo (EOF) encontrado antes da aspa de fechamento. */
\"([^"\\\n\0]|\\.)*         {
                                microc_yylval.error_msg = "EOF em string";
                                return retorna_token(UNDEF);
                            }


 /* --- Operadores relacionais e logicos ---------------------------------
  * Done by: Pedro Candia */
"=="                         { return retorna_token(EQ); }
"="                          { return retorna_token(ASSIGN); }
"!="                         { return retorna_token(NEQ); }
"!"                          { return retorna_token(NOT); }
"<="                         { return retorna_token(LEQ); }
"<"                          { return retorna_token(LT); }
">="                         { return retorna_token(GEQ); }
">"                          { return retorna_token(GT); }
"&&"                         { return retorna_token(AND); }
"||"                         { return retorna_token(OR); }

 /* --- Operadores aritmeticos e simbolos de pontuacao ------------------- */
"+"                          { return retorna_token(PLUS); }
"-"                          { return retorna_token(MINUS); }
"*"                          { return retorna_token(MUL); }
"/"                          { return retorna_token(DIV); }
"%"                          { return retorna_token(MOD); }
";"                          { return retorna_token(SEMICOLON); }
","                          { return retorna_token(COMMA); }
"("                          { return retorna_token(LPAREN); }
")"                          { return retorna_token(RPAREN); }
"{"                          { return retorna_token(LBRACE); }
"}"                          { return retorna_token(RBRACE); }
"["                          { return retorna_token(LBRACKET); }
"]"                          { return retorna_token(RBRACKET); }

 /* --- Caractere invalido -------------------------------------------------
  * Casa com qualquer caractere que nao tenha correspondido a nenhuma regra anterior. Deve ser SEMPRE a ultima regra do arquivo. */
.                            {
                                microc_yylval.error_msg = strdup(yytext);
                                return retorna_token(UNDEF);
                             }

%%

/* -----------------------------------------------------------------------
 * 4. SUB-ROTINAS DO USUARIO
 * ------------------------------------------------------------------- */

/* yywrap: informa ao flex que, ao atingir o EOF, a leitura deve simplesmente parar (nao ha um proximo arquivo a processar). */
int yywrap(void) 
{
    return 1;
}

/* main() de teste: le o arquivo passado como argumento e imprime, para cada token reconhecido, seu tipo, lexema e linha -- no mesmo espirito
 * do utilitario "lexer" mencionado no enunciado (Secao 6). Este main() e apenas uma ferramenta de depuracao para voce testar seu scanner de
 * forma isolada; ele NAO faz parte da interface formal entre o scanner e o parser (isso sera tratado nos trabalhos seguintes). */

int main(int argc, char **argv) 
{
    if (argc < 2) 
    {
        fprintf(stderr, "Uso: %s <arquivo.mc>\n", argv[0]);
        return 1;
    }

    FILE *arquivo_fonte = fopen(argv[1], "r");
    if (!arquivo_fonte) 
    {
        fprintf(stderr, "Erro: nao foi possivel abrir o arquivo '%s'\n", argv[1]);
        return 1;
    }
    yyin = arquivo_fonte;

    int tipo; 
    while ((tipo = yylex()) != END_OF_FILE) 
    {
        if (tipo == UNDEF) 
        {
            fprintf(stderr, "ERRO LEXICO (linha %d): %s\n",
                    linha_atual, microc_yylval.error_msg);
            continue;
        }
        printf("Token: tipo = %-13s lexema = '%s'  linha = %d\n",
               nome_token[tipo], yytext, linha_atual);
    }

    fclose(arquivo_fonte);
    return 0;
}
