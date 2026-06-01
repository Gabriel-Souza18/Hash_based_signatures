# Assinaturas Digitais Baseadas em Hash
## Repositório para códigos desenvolvidos na Iniciação Científica

Este projeto implementa e compara algoritmos de assinatura digital baseados em hash: **Lamport OTS**, **WOTS** (Winternitz One-Time Signature) e **MSS** (Merkle Signature Scheme).

---

## 📁 Estrutura do Projeto

```
HASH_BASED_SIGNATURES/
├── SHA256/              # INterface para uso da Biblioteca Sha256 da OpenSSL
├── LOTS/                # Implementação Lamport OTS
├── WOTS/                # Implementação WOTS
├── MSS/                 # Implementação MSS (Merkle Signature Scheme)
├── HORS/                # Implementação do HORS
├── HORST/               # Implementação do HORST
├── TesteHash/           # Testes de hash
├── TesteGerais/           # Testes 
├── makefile             # Makefile principal
└── readme.md            # Este arquivo
```
### Compilar tudo
```bash
make          
make all       # Mesmo que 'make'
```

### Compilar módulos individuais
```bash
make sha256    # Compila apenas a biblioteca SHA256
make lots      # Compila apenas Lamport OTS
make wots      # Compila apenas WOTS
make mss       # Compila apenas MSS
make hors      # Compila apenas HORS
make horst     # Compila apenas HORST
make teste     # Compila TesteHash
```
### Testes
```bash
make master_tests   #Testa todos os algortimos
make sing_tests     #Testa somente LOTS, WOTS e HORS
make tree_test      #Testa somente MSS e HORST

```


### limpeza e ajuda
```bash

make clean     # Remove todos os arquivos compilados
make rebuild   # Limpa e recompila tudo
make help      # Mostra ajuda
```


## Ferramentas de Debug

### Valgrind (verificação de memória)
Comandos para testes com memorias,

exemplos de comandos:
```bash

make lots
valgrind --track-origins=yes --leak-check=full ./LOTS/lots

make exemplo
valgrind --track-origins=yes --leak-check=full ./"DIR"/"Executavel"
```

---
## Strings de busca usadas
- hash AND signature AND digital AND (ESP32 OR arduino OR iot)

## 📚 Referência Principal 

### An Overview of Hash Based Signatures <br>
#### Vikas Srivastava , Anubhab Baksi, and Sumit Kumar Debnath1<br>
- National Institute of Technology Jamshedpur, India /Nanyang Technological University, Singapore

### Aulas do youtube do Christof Paar

### https://asecuritysite.com/hashsig/
---
## Autor: Gabriel da Silva Souza - Orientador: Charles Figueredo de Barros

Desenvolvido como parte da Iniciação Científica em Criptografia.
