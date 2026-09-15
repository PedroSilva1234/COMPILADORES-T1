## USO_IA.md
Ferramenta: Google Gemini (3.1 Pro)
Trecho: Implementação do lookahead para determinar se o '-' era o operando ou o sinal de negativo em um número.
Finalidade: Discutir possíveis maneiras de determinar se o '-' era operador ou sinal.
O que eu fiz: Pedi para que ele me ajudasse a entender como implementar a estratégiade lookahead. Ele me trouxe uma resposta, mas a solução não era definitiva, então discuti com ele para poder elaborar um algoritmo que de fato solucionasse o problema, e chegamos na abordagem de implementar um int ultimo_token que guarda o ultimo token significativo para poder comparar com o atual e determinar o que seria o '-'.

Ferramenta: Google Gemini (3.1 Pro)
Trecho: Dica para como implementar a tabela de identificadores, utilizando uma tabela hash com apenas strings.
Finalidade: Buscar ideias conhecidas para a criação da hash usando strings como chave.
O que eu fiz: Pedi para ele me recomendar estratégias de algoritmos que ajudavam a gerar um gerador de indicies com strings para a hash. Ele me recomendou o djb2 e me explicou como esse indicie era calculado. Eu li e implementei com a dica dele, assim formando a hash

Ferramenta: Google Gemini (3.1 Pro)
Trecho: Gerar e testar a expressão regular feita para a 'CONSTCHAR'.
Finalidade: Testar casos de teste com a expressão '([^'\n]|.)' que deveria ser responsavel pela const char 'x'.
O que eu fiz: Pedi para ele me gerar casos testes dessa expressão, a fim de ver se ela gerava o resultado correto. Entretanto, ele analisou e sugeriu uma correção para a expressão, que não acatava casos teste com '\n' ou '\constante. Assim, corrigindo a minha expressão para '([^'\\\n]|\\.)' 

Ferramenta: Google Gemini (3.1 Pro)
Trecho: Captura de strings
Finalidade: Ajustar o REGEX para a captura de strings.
O que eu fiz: Pedi ajuda para corrigir o REGEX que eu tinha feito, pois a leitura de strings não estava funcionando da maneira como deveria, a implementação do \n entre outros não estava correta. Com isso, corrigi o código e deu certo.