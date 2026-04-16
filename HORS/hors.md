# HORS - Hash to Obtain Random Subset

## Como compilar

```bash
cd HORS
make clean && make
```

## Como rodar

```bash
./hors
```

Digite uma mensagem e o programa vai:
1. Gerar 1024 chaves secretas
2. Assinar a mensagem
3. Imprimir os 26 componentes da assinatura

## Exemplo

```bash
$ ./hors
Digite uma mensagem: oi tudo bem
Mensagem lida: oi tudo bem
Tamanho: 11 bytes
Chaves Geradas
Mensagem assinada!
=== Assinatura HORS ===
Componente 0: 4a1ced9fcf66cf9b1f27afb45280af3025e21951c4e0978f5fa4a507230dc5b7
Componente 1: 0f7f77e84d30b4c41410bc92b4a0a629e604de1110170a3438638d1e15f72736
...
Componente 25: 3aaf9989b44af7d502d86712336e5c39f778532391b2813e26fb9190d73553b6
=======================
```

## Parâmetros

- **HORS_T**: número de chaves secretas (padrão: 1024)
- **HORS_K**: componentes da assinatura (26 para T=1024)
- **HORS_N**: tamanho de cada chave (32 bytes = SHA-256)

## Teste com diferentes valores de T

| HORS_T | Bits/Índice | Componentes (K) | Tamanho Assinatura |
|--------|-------------|-----------------|------------------|
| **256** | 8 | 32 | 1024 bytes |
| **512** | 9 | 29 | 928 bytes |
| **1024** | 10 | 26 | 832 bytes |

Para compilar com outro T:
```bash
cd HORS
make clean && make CFLAGS="-Wall -O2 -I../SHA256 -DHORS_T=256"
```
