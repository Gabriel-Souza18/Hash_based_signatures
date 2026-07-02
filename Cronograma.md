
##  Histórico de Desenvolvimento

### Fase 1: Implementação SHA256 (22/09 - 03/10)
- [x] Implementação da função SHA256
- [x] Testes com strings de tamanhos variados
- [x] Medição de tempo de execução
- [x] Organização inicial da estrutura

### Fase 2: Lamport OTS (03/10 - 08/10)
- [x] Implementação completa do Lamport OTS
- [x] Sistema de leitura/escrita de chaves
- [x] Função de assinatura
- [x] Função de verificação

### Fase 3: WOTS (10/10 - 20/10)
- [x] Implementação do WOTS (W=16, L=67)
- [x] Sistema de máscaras
- [x] Comparação com Lamport OTS
- [x] Testes de desempenho

### Fase 4: Reorganização e Testes (21/10 - 29/10)
- [x]  Reorganização modular do projeto
- [x] Criação de makefiles individuais
- [x] Biblioteca SHA256 separada
- [x] Script de testes automatizados aprimorado
- [x] Coleta detalhada de métricas de desempenho

### Fase 5: MSS(29/10 -17/11)
- [x] Assisti a aula do Christof Paar sobre MSS
- [x] Implementando Arvore que gera Hashs 
- [x] Corrigindo problemas e gerando assinatura
- [x] implementar verificação da assinatura 
- [x] Corrigir Lamport 
---
# Fase 6: TESTES(18/11)
- [x] Colocar no padrao do NIST FIPS ( NAO COLOQUEI O MSS)
- [x] Realizar teste de memoria com valgrind


# Fase 7: HORS e artigo

- [x] Procurar mais artigos com assunto relacionados para colocar no artigo.
- [x] Inicar a escrita do artigo com alguns resultados preliminares e trabalhos relacionados 
- [x] Implementar algoritmo HORS
- [x] Implementar HORTS
- [x] Testar algoritmos implementados 
- 

# Fase 8 - shortpaper SBSEG
- [x] Refazer testes com valgrind
- [x] Focar no artigo
- [x] Trocar lib de hash para openssl


# Fase 9 - SPHINCS
- [x] Pesquisar e entender SPHINCS
- [x] Implementar o SPHICS
- [] Realizar testes
- [] Atualizar Documentação  e testes
- [] Separar todos os algoritmos em destinatario e remetente

## Falar na proxima reunião 

Apresentação de segunda

Implementação:
    sphichs e falar que ainda nao terminou
    LOTS e WOTS separados em remetente de destinatario

Perguntar sobre tempo de apresentação de TCC

MQTT

INA226

## Anotaçoes reuniao
medir o de dados (quantos bits ele consegue transmitir)
pesquisar se da pra medir consumo de enegia do esp32
procurar tamanho do pacote MQTT

## Corrigir TCC