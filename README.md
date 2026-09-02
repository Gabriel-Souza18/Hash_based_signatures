# Assinaturas Digitais Baseadas em Hash

## Repositório para códigos desenvolvidos na Iniciação Científica

Este projeto implementa e compara algoritmos de assinatura digital baseados em hash: **LOTS**, **WOTS**, **MSS**, **HORS**, **HORST** e **SPHINCS**.

Todos os algoritmos possuem executáveis separados para **remetente** (geração de chaves + assinatura) e **destinatário** (verificação).

---

## Estrutura do Projeto

```
HASH_BASED_SIGNATURES/
├── SHA256/              # Interface para uso da biblioteca SHA-256 da OpenSSL
├── LOTS/                # Lamport One-Time Signature
│   ├── remet_lots       # Remetente (keygen + assinatura)
│   └── dest_lots        # Destinatário (verificação)
├── WOTS/                # Winternitz One-Time Signature (FIPS 205)
│   ├── remet_wots       # Remetente
│   └── dest_wots        # Destinatário
├── MSS/                 # Merkle Signature Scheme
│   ├── remet_mss        # Remetente
│   └── dest_mss         # Destinatário
├── HORS/                # Hash to Obtain Random Subset
│   ├── remet_hors       # Remetente
│   └── dest_hors        # Destinatário
├── HORST/               # HORS with Trees
│   ├── remet_horst      # Remetente
│   └── dest_horst       # Destinatário
├── SPHINCS/             # SPHINCS (stateless)
│   ├── remet_sphincs    # Remetente
│   └── dest_sphincs     # Destinatário
├── ComunicaçãoMQTT/     # Comunicação MQTT (PC ↔ ESP)
├── TesteHash/           # Testes de hash
├── TestesGerais/        # Scripts de teste automatizados
├── makefile             # Makefile principal
└── README.md            # Este arquivo
```

---

## Compilação

### Compilar tudo
```bash
make             # Compila todos os módulos
make all         # Mesmo que 'make'
```

### Compilar módulos individuais
```bash
make sha256      # Compila apenas a biblioteca SHA256
make lots        # Compila apenas Lamport OTS
make wots        # Compila apenas WOTS
make mss         # Compila apenas MSS
make hors        # Compila apenas HORS
make horst       # Compila apenas HORST
make sphincs     # Compila apenas SPHINCS
make teste       # Compila TesteHash
```

### Testes
```bash
make master_tests   # Testa todos os algoritmos
make sing_tests     # Testa LOTS, WOTS, HORS e SPHINCS
make tree_test      # Testa MSS e HORST
```

### Limpeza e ajuda
```bash
make clean       # Remove todos os arquivos compilados
make rebuild     # Limpa e recompila tudo
make help        # Mostra ajuda
```

---

## Uso (Remetente / Destinatário)

Cada algoritmo possui dois executáveis independentes que se comunicam via arquivos:

```bash
# Exemplo com HORS
cd HORS
make
./remet_hors "Mensagem a ser assinada"   # Gera chaves, assina e salva arquivos
./dest_hors                               # Lê arquivos e verifica a assinatura
```

O mesmo padrão se aplica a todos os algoritmos (`remet_*` / `dest_*`).

---

## Ferramentas de Debug

### Valgrind (verificação de memória)
```bash
make hors
valgrind --track-origins=yes --leak-check=full ./HORS/remet_hors "teste"
valgrind --track-origins=yes --leak-check=full ./HORS/dest_hors
```

---

## Referências

- **An Overview of Hash Based Signatures** — Vikas Srivastava, Anubhab Baksi, and Sumit Kumar Debnath (NIT Jamshedpur / NTU Singapore)
- **SPHINCS: Practical Stateless Hash-Based Signatures** — Bernstein et al., 2015
- Aulas do YouTube do Christof Paar
- https://asecuritysite.com/hashsig/

---

## Autor: Gabriel da Silva Souza — Orientador: Charles Figueredo de Barros

Desenvolvido como parte da Iniciação Científica em Criptografia.
