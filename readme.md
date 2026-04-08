# Assinaturas Digitais Baseadas em Hash
## Repositório para códigos desenvolvidos na Iniciação Científica

Este projeto implementa e compara algoritmos de assinatura digital baseados em hash: **Lamport OTS**, **WOTS** (Winternitz One-Time Signature) e **MSS** (Merkle Signature Scheme).

---

## 📁 Estrutura do Projeto

```
sha256_c_cpp/
├── SHA256/              # Biblioteca SHA256
├── LamportOTS/          # Implementação Lamport OTS
├── WOTS/                # Implementação WOTS
├── MSS/                 # Implementação MSS (Merkle Signature Scheme)
├── TesteHash/           # Testes de hash
├── makefile             # Makefile principal
├── test_algorithms.sh   # Script de testes automatizados
└── readme.md            # Este arquivo
```
### Compilar tudo
```bash
make           # Compila biblioteca SHA256, Lamport, WOTS e MSS
make all       # Mesmo que 'make'
```

### Compilar módulos individuais
```bash
make sha256    # Compila apenas a biblioteca SHA256
make lamport   # Compila apenas Lamport OTS
make wots      # Compila apenas WOTS
make mss       # Compila apenas MSS
make teste     # Compila TesteHash
```bash
make sha256    # Compila apenas a biblioteca SHA256
make lamport   # Compila apenas Lamport OTS
make wots      # Compila apenas WOTS
make teste     # Compila TesteHash
```

### Testes e limpeza
```bash
make test      # Compila e executa testes automatizados
make clean     # Remove todos os arquivos compilados
make rebuild   # Limpa e recompila tudo
make help      # Mostra ajuda
```


## Ferramentas de Debug

### Valgrind (verificação de memória)
```bash
make lamport
valgrind --track-origins=yes --leak-check=full ./LamportOTS/lamport

make wots
valgrind --track-origins=yes --leak-check=full ./WOTS/wots
```

---

##  Histórico de Desenvolvimento

### Fase 1: Implementação SHA256 (22/09 - 03/10)
- [x] Implementação da função SHA256
- [x] Testes com strings de tamanhos variados
- [x] Medição de tempo de execução
- [x] Organização inicial da estrutura

### Fase 2: Lamport OTS (03/10 - 08/10)
- [x] Implementação completa do Lamport OTS
- [x] Sistema de leitura/escrita de chaves
- [x]Função de assinatura
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

- [] Procurar mais artigos com assunto relacionados para colocar no artigo.
- [] Inicar a escrita do artigo com alguns resultados preliminares e trabalhos relacionados 
- [] Implementar algoritmo HORS e HORST
- [] Testar algoritmos implementados 
- 

## Próximos Passos
- Separar em programas rementente/ destinatario

- Ver aula pra fixar WOTS

- 
---

## 📚 Referência Principal 

### An Overview of Hash Based Signatures <br>
#### Vikas Srivastava , Anubhab Baksi, and Sumit Kumar Debnath1<br>
- National Institute of Technology Jamshedpur, India /Nanyang Technological University, Singapore

### Aulas do youtube do Christof Paar

### https://asecuritysite.com/hashsig/
---
## Autor- Gabriel da Silva Souza

Desenvolvido como parte da Iniciação Científica em Criptografia.
