# Assinaturas digitais baseadas em Hash
## Repositorio pra Salvar codigos feitos na Iniciação cientifica

## Comandos importantes:
* make teste
* make lamport
* valgrind --track-origins=yes --leak-check=full ./testeHash

* make clean
### Primeiros testes com a função sha256 
# 22/09 a 03/10

* 24/09 - Erro na leitura das strings com 1 milhao de caracteres, tenho que testar em um pc melhor ou testar sem outros apps abertos<br>
Outras strings funcionaram perfeitamente, mas tenho que buscar uma base maior pra conferir os testes
e terminar de implementar funções de testes, anotar estatisticas(tempo, memoria)<br>


* 25/09 - Consegui rodar os testes com as strings maiores e ja fiz a medição do tempo que demora pra calcular a hash de cada string, as pequenas não consegui resultados, mas as grandes consegui achar resultados.<br>

* 02/09 - Ja iniciei os testes do LamportOTS, mas ainda falta algumas coisas, tive que reorganizar a pasta, e vou ter que refatorar algumas coisas, como mudar nome do utils.c, ou extrair essas funções, pois tenho que fazer uma função pra ler e escrever arquivo, pois vou ter que escrever as chaves.

* 03/09 - Mudei os nomes das .h e arrumei as leituras e escritas de arquivos, aparentemente a assinatura ta correta, so falta a função de verificação das assinaturas, Vou ler sobre Funçoes Esponjas.

---
### Comparando LOTS e WOTS
# 06/10 a 20/10
* 08/10 - Finalizei o LamportOTS, Esqueci de escrever aqui algumas mudanças que eu realizei, mas o principal foi a mudança no tamanho da mensagem e das chaves secretas, 

TODO: 
* Implementar W-OTS
* Comparar Wots e Lots
* Pesquisar se repetição nos valores das chaves interfere na segurança da assinatura.
* Ler sobre Funçoes Esponjas