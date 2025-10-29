# Assinaturas Digitais Baseadas em Hash
## Repositório para códigos desenvolvidos na Iniciação Científica

Este projeto implementa e compara algoritmos de assinatura digital baseados em hash: **Lamport OTS** e **WOTS** (Winternitz One-Time Signature).

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
├── TesteHash/           # Testes de hash
│   ├── main.c           # Programa de testes
│   ├── dicionario.c/h   # Manipulação de dicionários
│   └── makefile         # Compilação testes
│
├── makefile             # Makefile principal
├── test_algorithms.sh   # Script de testes automatizados
└── readme.md            # Este arquivo
```

---

## 🚀 Comandos de Compilação

### Compilar tudo
```bash
make           # Compila biblioteca SHA256, Lamport e WOTS
make all       # Mesmo que 'make'
```

### Compilar módulos individuais
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

---

## 🧪 Executando os Algoritmos

### Lamport OTS
```bash
./LamportOTS/lamport
# Opção 1: Gerar assinatura
# Opção 2: Verificar assinatura
```

### WOTS
```bash
./WOTS/wots
# Opção 1: Gerar assinatura
# Opção 2: Verificar assinatura
```

### Testes Automatizados
```bash
./test_algorithms.sh
# Executa 10 testes de cada algoritmo
# Gera arquivo CSV com resultados comparativos
```

---

## 📊 Métricas Coletadas

O script de testes automatizados coleta as seguintes métricas:

- **Tempo de geração de chaves secretas**
- **Tempo de geração de chaves públicas**
- **Tempo de geração de masks** (apenas WOTS)
- **Tempo de assinatura**
- **Número de hashes SHA256** utilizados
- **Tamanho das chaves e assinaturas** (em bytes)

Os resultados são salvos em formato CSV para análise posterior.

---

## 🔧 Ferramentas de Debug

### Valgrind (verificação de memória)
```bash
make lamport
valgrind --track-origins=yes --leak-check=full ./LamportOTS/lamport

make wots
valgrind --track-origins=yes --leak-check=full ./WOTS/wots
```

---

## 📝 Histórico de Desenvolvimento

### Fase 1: Implementação SHA256 (22/09 - 03/10)
- ✅ Implementação da função SHA256
- ✅ Testes com strings de tamanhos variados
- ✅ Medição de tempo de execução
- ✅ Organização inicial da estrutura

### Fase 2: Lamport OTS (03/10 - 08/10)
- ✅ Implementação completa do Lamport OTS
- ✅ Sistema de leitura/escrita de chaves
- ✅ Função de assinatura
- ✅ Função de verificação

### Fase 3: WOTS (10/10 - 20/10)
- ✅ Implementação do WOTS (W=16, L=67)
- ✅ Sistema de máscaras
- ✅ Comparação com Lamport OTS
- ✅ Testes de desempenho

### Fase 4: Reorganização e Testes (29/10)
- ✅ Reorganização modular do projeto
- ✅ Criação de makefiles individuais
- ✅ Biblioteca SHA256 separada
- ✅ Script de testes automatizados aprimorado
- ✅ Coleta detalhada de métricas de desempenho

---

## 🎯 Próximos Passos

- [ ] Implementar XMSS (Extended Merkle Signature Scheme)
- [ ] Analisar padrões de repetição em chaves
- [ ] Estudar funções esponja (Sponge Functions)
- [ ] Testes em aplicações IoT
- [ ] Análise de segurança comparativa

---

## 📚 Referências

- Lamport One-Time Signature Scheme
- WOTS+ (Winternitz One-Time Signature Plus)
- SHA-256 Cryptographic Hash Function
- RFC 8391 - XMSS: eXtended Merkle Signature Scheme

---

## 👤 Autor

Desenvolvido como parte da Iniciação Científica em Criptografia.

---

## 📄 Licença

Este projeto é de código aberto para fins acadêmicos e educacionais.
