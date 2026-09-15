// test.mc - Programa de teste para o scanner de Micro C
//
// Este arquivo exercita todos os tokens da linguagem: palavras
// reservadas, identificadores, constantes inteiras (positivas e negativas), 
// constantes de caractere e de string (com escapes), operadores aritmeticos, 
// relacionais e logicos, simbolos de pontuacao, comentarios e erros lexicos.

/* Calcula o fatorial de um numero inteiro utilizando um laco for. */
int fatorial(int n) {
    int resultado;
    int i;

    resultado = 1;
    for (i = 1; i <= n; i = i + 1) {
        resultado = resultado * i;
    }
    return resultado;
}

int main() {
    int x;
    int y;
    char c;
    char letras[10];

    x = 5;
    y = fatorial(x);

    if (y > 100) {
        print("Resultado grande\n"); // Teste de escape valido: \n
    } else {
        print("Resultado pequeno\t!"); // Teste de escape valido: \t
    }

    c = 'A';
    letras[0] = 'H';
    letras[1] = 'i';

    // --- TESTES EXTRAS DE TOKENS VALIDOS ---

    // 1. Operadores restantes (Divisao e Modulo)
    int div = 10 / 2;
    int resto = 10 % 3;

    // 2. Ambiguidade do Sinal de Menos (Lookahead)
    int negativo = -15;       // Token MINUS ou INTEGERCONST? (Deve ser INTEGERCONST)
    int subtracao = x - 5;    // (Deve ser ID, MINUS, INTEGERCONST)
    int duplo = x - -3;       // (Deve ser ID, MINUS, INTEGERCONST negativo)
    letras[-1] = '\0';        // (Deve capturar o -1 como negativo dentro do colchete e o escape \0)

    // 3. Sequencias de escape complexas
    char aspa_simples = '\'';
    char barra = '\\';
    print("Ele disse: \"Compiladores eh incrivel!\" \\o/");


    // --- TESTES DE ERROS LEXICOS (A PARTIR DAQUI O SCANNER DEVE REPORTAR UNDEF) ---

    // Erro 1: Caracteres invalidos
    int @invalido = 0;
    # 

    // Erro 2: Comentario nao iniciado
    */ 

    // Erro 3: Constante de caractere malformada (nao fechada)
    char erro_char = 'X ;

    // Erro 4: String nao terminada (encontra quebra de linha antes de fechar)
    print("Esta string vai quebrar a linha
    e gerar um erro");

    // Erro 5: Caractere nulo real na string (Representado aqui escapado para o editor, 
    // mas o professor testara com um byte 0x00 real no arquivo)

    return 0;
}

// Erro 6: EOF em comentario (Fica no final do arquivo propositalmente)
/* Este comentario em bloco comecou, mas o arquivo vai acabar antes do