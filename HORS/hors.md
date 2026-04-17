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

## Valores recomendados de T

| T | log₂(T) | K (componentes) | Assinatura | Segurança |
|---|---------|---|---|---|
| 2 | 1 | 256 | 8 KB | ❌ Nenhuma |
| 16 | 4 | 64 | 2 KB | ❌ Fraca |
| 256 | 8 | 32 | 1 KB | ⚠️ Moderada |
| 1024 | 10 | 26 | 832 B | ✅ Boa |
| 4096 | 12 | 22 | 704 B | ✅ Ótima |
| 65536 | 16 | 16 | 512 B | ✅ Excelente |

**Recomendação:** Use `HORS_T = 1024` ou maior para bom equilíbrio entre tamanho de assinatura e segurança.

## Quantas assinaturas são seguras?

HORS é um esquema **"few-time"** (poucos usos). Cada mensagem diferente que você assina usa diferentes índices. Se dois índices se repetem, a chave secreta é comprometida.

### Fórmula do limite seguro

```
Número seguro de assinaturas ≈ √(HORS_T / 2)
```

Baseado no **paradoxo do aniversário**: com espaço de tamanho N, após √N tentativas aleatórias, há 50% de chance de colisão.

### Exemplos práticos

| HORS_T | Assinaturas seguras | Cálculo |
|--------|---|---|
| 256 | ~11 | √(256/2) ≈ 11 |
| 512 | ~16 | √(512/2) ≈ 16 |
| 1024 | ~23 | √(1024/2) ≈ 23 |
| 4096 | ~45 | √(4096/2) ≈ 45 |
| 65536 | ~181 | √(65536/2) ≈ 181 |

### Probabilidade de colisão

A probabilidade exata após `n` assinaturas é:

```
P(colisão) = 1 - e^(-(n×k)² / (2×t))

Onde:
  n = número de assinaturas
  k = número de componentes (26 para T=1024)
  t = número total de índices (1024)
```

**Exemplo com T=1024, k=26:**

| Assinaturas | P(colisão) | Risco |
|---|---|---|
| 1 | 0.08% | ✅ Muito seguro |
| 5 | 2% | ✅ Seguro |
| 10 | 8% | ✅ Seguro |
| 23 | 50% | ⚠️ Metade das chances |
| 50 | 98% | ❌ Altamente inseguro |

**Conclusão:** Com `HORS_T = 1024`, você pode assinar com segurança ~23 mensagens diferentes antes que a probabilidade de colisão chegue a 50%.
