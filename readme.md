# Assinaturas Digitais Baseadas em Hash
## Repositório para códigos desenvolvidos na Iniciação Científica

Este projeto implementa e compara algoritmos de assinatura digital baseados em hash: **Lamport OTS**, **WOTS** (Winternitz One-Time Signature) e **MSS** (Merkle Signature Scheme).

---

## 📁 Estrutura do Projeto

```
sha256_c_cpp/
├── SHA256/              # Biblioteca SHA256
│   ├── sha256.c         # Implementação SHA256
│   ├── sha256.h         # Header SHA256
│   └── makefile         # Compilação da biblioteca
│
├── LamportOTS/          # Implementação Lamport OTS
│   ├── lamport.c        # Programa principal
│   ├── keys.c/h         # Gerenciamento de chaves
│   ├── utils.c/h        # Funções utilitárias
│   └── makefile         # Compilação Lamport
│
├── WOTS/                # Implementação WOTS
│   ├── wots.c           # Programa principal
│   ├── keys.c/h         # Gerenciamento de chaves
│   ├── utils.c/h        # Funções utilitárias
│   └── makefile         # Compilação WOTS
│
├── MSS/                 # Implementação MSS (Merkle Signature Scheme)
│   ├── mss.c            # Programa principal
│   ├── Arvore.c/h       # Estrutura da árvore de Merkle
│   └── makefile         # Compilação MSS
│
├── TesteHash/           # Testes de hash
│   ├── main.c           # Programa de testes
│   ├── dicionario.c/h   # Manipulação de dicionários
│   └── makefile         # Compilação testes
│
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
- ✅ Implementação da função SHA256
- ✅ Testes com strings de tamanhos variados
- ✅ Medição de tempo de execução
- ✅ Organização inicial da estrutura

### Fase 2: Lamport OTS (03/10 - 08/10)
- ✅ Implementação completa do Lamport OTS
- ✅ Sistema de leitura/escrita de chaves
- ✅Função de assinatura
- ✅ Função de verificação

### Fase 3: WOTS (10/10 - 20/10)
- ✅Implementação do WOTS (W=16, L=67)
- ✅ Sistema de máscaras
- ✅ Comparação com Lamport OTS
- ✅ Testes de desempenho

### Fase 4: Reorganização e Testes (21/10 - 29/10)
- ✅  Reorganização modular do projeto
- ✅ Criação de makefiles individuais
- ✅Biblioteca SHA256 separada
- ✅Script de testes automatizados aprimorado
- ✅ Coleta detalhada de métricas de desempenho

### Fase 5: MSS(29/10 -)
- ✅Assisti a aula do Christof Paar sobre MSS
- ✅ Implementando Arvore que gera Hashs 
- ✅ Corrigindo problemas e gerando assinatura
---

## Próximos Passos
- implementar verificação da assinaturaMSS, e um sistema pra escolher quem vc quer ser no p2p

- corrigir implementação do lamporte colocar nome como LOTS, usar memoria igual no WOTS

- ver aula pra fixar WOTS

- Testar em arduino(aprender como rodar isso em arduino)

---

## 📚 Referência Principal 

### An Overview of Hash Based Signatures <br>
#### Vikas Srivastava , Anubhab Baksi, and Sumit Kumar Debnath1<br>
- National Institute of Technology Jamshedpur, India /Nanyang Technological University, Singapore

### Aulas do youtube do Christof Paar
---
## Autor- Gabriel da Silva Souza

Desenvolvido como parte da Iniciação Científica em Criptografia.
