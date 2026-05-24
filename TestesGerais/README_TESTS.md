# Suite de Testes - Hash Based Signatures

## Visão Geral

Esta suite automatiza testes completos dos algoritmos de assinatura (LOTS, WOTS, HORS, HORST, MSS):
- Coleta métricas de performance (tempo, hashes, tamanho)
- Coleta métricas de memória com Valgrind
- Consolida tudo em uma tabela única para análise

## Scripts

### 1. `master_tests.sh` (Script Principal)
Executa a sequência completa de testes:
```bash
cd TestesGerais
./master_tests.sh
```

**O que faz:**
1. Roda `test_algorithms.sh` → gera `resultados_*.csv`
2. Roda `run_valgrind_all.sh` → gera logs em `Resultados_Valgrind/`
3. Coleta bytes de Valgrind → gera `valgrind_bytes_*.csv`
4. Consolida tudo → gera `consolidated_results_*.csv`

**Tempo estimado:** 5-10 minutos (depende do número de testes)

### 2. `test_algorithms.sh`
Testa performance dos algoritmos (LOTS, WOTS, HORS):
```bash
./test_algorithms.sh
```

**Saída:**
- `resultados_metricas/resultados_YYYYMMDD_HHMMSS.csv`

**Colunas:**
- Algoritmo, Teste, Tempo_SecretKeys, Tempo_PublicKeys, Tempo_Masks, Tempo_Assinatura, Hashes_Assinatura, Tamanho_SecretKeys, Tamanho_PublicKeys, Tamanho_Assinatura

### 3. `run_valgrind_all.sh`
Testa com Valgrind (memory profiling):
```bash
./run_valgrind_all.sh
```

**Saída:**
- `Resultados_Valgrind/ALGORITMO_EXECUTAVEL_TAG.stdout`
- `Resultados_Valgrind/ALGORITMO_EXECUTAVEL_TAG.valgrind`

### 4. `collect_valgrind_metrics.py`
Coleta bytes de Valgrind dos logs:
```bash
python3 collect_valgrind_metrics.py Resultados_Valgrind > resultados_metricas/valgrind_bytes.csv
```

**Saída:**
- `resultados_metricas/valgrind_bytes_YYYYMMDD_HHMMSS.csv`

**Colunas:**
- Algoritmo, Tag, Total_Alocado, Total_Liberado, Bytes_Vazados, Reachable

### 5. `consolidate_results.py`
Consolida métricas de algoritmos + Valgrind em tabela única:
```bash
python3 consolidate_results.py resultados_metricas > resultados_metricas/consolidated.csv
```

**Saída:**
- `resultados_metricas/consolidated_results_YYYYMMDD_HHMMSS.csv`

**Colunas:**
- Algoritmo
- Tempo_Medio_SK_s, Tempo_Medio_PK_s, Tempo_Medio_Masks_s, Tempo_Medio_Assinatura_s
- Hashes_Medio
- Tamanho_SK_bytes, Tamanho_PK_bytes, Tamanho_Assinatura_bytes
- Valgrind_Total_Alocado_bytes, Valgrind_Total_Liberado_bytes, Valgrind_Bytes_Vazados, Valgrind_Reachable_bytes

## Estrutura de Diretórios

```
TestesGerais/
├── master_tests.sh              # Script principal (executa tudo)
├── test_algorithms.sh           # Testa performance
├── run_valgrind_all.sh          # Testa com Valgrind
├── collect_valgrind_metrics.py  # Coleta bytes de Valgrind
├── consolidate_results.py       # Consolida resultados
├── resultados_metricas/         # Resultados de performance
│   ├── resultados_*.csv
│   ├── valgrind_bytes_*.csv
│   └── consolidated_results_*.csv
└── Resultados_Valgrind/         # Logs brutos de Valgrind
    ├── LOTS_lots_remetente.valgrind
    ├── LOTS_lots_remetente.stdout
    ├── ... (outros arquivos)
```

## Fluxo Recomendado

### Opção 1: Teste Completo (Recomendado)
```bash
cd TestesGerais
./master_tests.sh
# Abre resultados_metricas/consolidated_results_*.csv
```

### Opção 2: Testes Individuais
```bash
# Apenas performance
./test_algorithms.sh

# Apenas Valgrind
./run_valgrind_all.sh

# Consolidar manualmente
python3 collect_valgrind_metrics.py Resultados_Valgrind
python3 consolidate_results.py resultados_metricas
```

## Notas Importantes

1. **Limpeza automática**: Cada teste limpa arquivos temporários (`.o`, executáveis) ao final
2. **Médias**: `consolidate_results.py` calcula média de múltiplos testes
3. **Valgrind**: Timeout de 40s por teste; logs completos em `Resultados_Valgrind/`
4. **Pasta resultados_metricas**: Use para análise, tabelas e gráficos

## Exemplo de Uso

```bash
cd /home/gabriel/Documentos/Hash_based_signatures/TestesGerais

# Executar suite completa
./master_tests.sh

# Verificar resultados consolidados
head consolidated_results_*.csv | less
```

## Troubleshooting

- **Erro: "Executavel ausente"** → Compilar antes: `cd .. && make all`
- **Erro: "Falha ao coletar Valgrind"** → Verificar se Valgrind está instalado: `which valgrind`
- **CSV vazio** → Verificar se os testes geraram arquivos: `ls resultados_metricas/`

