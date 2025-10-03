# Assinaturas digitais baseadas em Hash
## Repositorio pra Salvar codigos feitos na Iniciação cientifica

## Makefile
execute o comando para compilar 
```
make compilar
```
execute o comando para rodar 
```
make run
```

### Primeiros testes com a função sha256

* 24/09 - Erro na leitura das strings com 1 milhao de caracteres, tenho que testar em um pc melhor ou testar sem outros apps abertos<br>
Outras strings funcionaram perfeitamente, mas tenho que buscar uma base maior pra conferir os testes
e terminar de implementar funções de testes, anotar estatisticas(tempo, memoria)<br>


* 25/09 - Consegui rodar os testes com as strings maiores e ja fiz a medição do tempo que demora pra calcular a hash de cada string, as pequenas não consegui resultados, mas as grandes consegui achar resultados.<br>

** 02/09 - Ja iniciei os testes do LamportOTS, mas ainda falta algumas coisas, tive que reorganizar a pasta, e vou ter que refatorar algumas coisas, como mudar nome do utils.c, ou extrair essas funções, pois tenho que fazer uma função pra ler e escrever arquivo, pois vou ter que escrever as chaves.
---


TODO: 
* Buscar mais strings pra testar(V)<br>
* Implementar estatisticas(V)
* Implementar lamport OTS