#!/usr/bin/env python3
"""
Consolida resultados de métricas em uma tabela única.
- Combina CSVs de test_algorithms.sh e test_heavy_algorithms.sh
- Agrega Valgrind (quando disponível)
- Produz uma linha por algoritmo com médias

NOVO: três colunas de hashes separadas (Keygen, Assinatura, Verificação)
"""

import sys
import csv
from pathlib import Path
from collections import defaultdict


# Campos cujas médias serão calculadas.
# Zeros legítimos (ex.: Tempo_Masks=0 para HORS) NÃO são removidos.
TIME_FIELDS = [
    'Tempo_SecretKeys', 'Tempo_PublicKeys', 'Tempo_Masks',
    'Tempo_Assinatura', 'Tempo_Verificacao',
]
HASH_FIELDS = ['Hashes_Keygen', 'Hashes_Assinatura', 'Hashes_Verificacao']
VALGRIND_FIELDS = ['Total_Alocado', 'Total_Liberado', 'Bytes_Vazados', 'Reachable']


def parse_csv(csv_file):
    """Lê CSV genérico, agrupando linhas por algoritmo."""
    data = defaultdict(list)
    try:
        with open(csv_file, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                data[row['Algoritmo']].append(row)
    except Exception as e:
        print(f"Erro ao ler {csv_file}: {e}", file=sys.stderr)
    return data


def calculate_averages(rows, numeric_fields):
    """
    Calcula médias. Campos ausentes viram 0.
    Não remove zeros — zeros legítimos (ex.: máscaras) são preservados.
    """
    averages = {}
    for field in numeric_fields:
        values = []
        for row in rows:
            if field in row and row[field] not in (None, ''):
                try:
                    values.append(float(row[field]))
                except ValueError:
                    pass
        averages[field] = sum(values) / len(values) if values else 0
    return averages


def get_latest_csv(directory, pattern, exclude_pattern=None):
    """Retorna o CSV mais recente que bate com o padrão."""
    files = list(Path(directory).glob(pattern))
    if exclude_pattern:
        files = [f for f in files if exclude_pattern not in f.name]
    return sorted(files)[-1] if files else None


def main():
    if len(sys.argv) < 2:
        print("Usage: consolidate_results.py <results_dir>", file=sys.stderr)
        sys.exit(1)

    results_dir = sys.argv[1]

    algo_csv = get_latest_csv(results_dir, 'resultados_OT*.csv', exclude_pattern='heavy')
    heavy_csv = get_latest_csv(results_dir, 'resultados_heavy_*.csv')
    valgrind_csv = get_latest_csv(results_dir, 'valgrind_bytes_*.csv')

    algo_data = {}
    valgrind_data = {}

    if algo_csv:
        for algo, rows in parse_csv(str(algo_csv)).items():
            algo_data[algo] = rows
    else:
        print("Aviso: nenhum CSV de algoritmos leves encontrado.", file=sys.stderr)

    if heavy_csv:
        for algo, rows in parse_csv(str(heavy_csv)).items():
            algo_data[algo] = rows
    else:
        print("Aviso: nenhum CSV de algoritmos pesados encontrado.", file=sys.stderr)

    if valgrind_csv:
        valgrind_data = parse_csv(str(valgrind_csv))

    # ── Cabeçalho consolidado ────────────────────────────────
    header = [
        'Algoritmo',
        'Tempo_Medio_SK_s',
        'Tempo_Medio_PK_s',
        'Tempo_Medio_Masks_s',
        'Tempo_Medio_Assinatura_s',
        'Tempo_Medio_Verificacao_s',
        'Hashes_Medio_Keygen',
        'Hashes_Medio_Assinatura',
        'Hashes_Medio_Verificacao',
        'Tamanho_SK_bytes',
        'Tamanho_PK_bytes',
        'Tamanho_Assinatura_bytes',
        'Valgrind_Total_Alocado_bytes',
        'Valgrind_Total_Liberado_bytes',
        'Valgrind_Bytes_Vazados',
        'Valgrind_Reachable_bytes',
    ]
    print(','.join(header))

    all_algos = set(algo_data.keys()) | set(valgrind_data.keys())

    for algo in sorted(all_algos):
        rows = algo_data.get(algo, [])

        avg_time = calculate_averages(rows, TIME_FIELDS)
        avg_hash = calculate_averages(rows, HASH_FIELDS)

        # Tamanhos: pega o primeiro valor válido
        tamanho_sk = tamanho_pk = tamanho_sign = 0
        if rows:
            def first_int(field):
                for r in rows:
                    try:
                        return int(r.get(field, 0))
                    except (ValueError, TypeError):
                        continue
                return 0
            tamanho_sk   = first_int('Tamanho_SecretKeys')
            tamanho_pk   = first_int('Tamanho_PublicKeys')
            tamanho_sign = first_int('Tamanho_Assinatura')

        vg_rows = valgrind_data.get(algo, [])
        vg_avg = calculate_averages(vg_rows, VALGRIND_FIELDS)

        row = [
            algo,
            f"{avg_time.get('Tempo_SecretKeys', 0):.6f}",
            f"{avg_time.get('Tempo_PublicKeys', 0):.6f}",
            f"{avg_time.get('Tempo_Masks', 0):.6f}",
            f"{avg_time.get('Tempo_Assinatura', 0):.6f}",
            f"{avg_time.get('Tempo_Verificacao', 0):.6f}",
            f"{avg_hash.get('Hashes_Keygen', 0):.0f}",
            f"{avg_hash.get('Hashes_Assinatura', 0):.0f}",
            f"{avg_hash.get('Hashes_Verificacao', 0):.0f}",
            f"{tamanho_sk}",
            f"{tamanho_pk}",
            f"{tamanho_sign}",
            f"{vg_avg.get('Total_Alocado', 0):.0f}",
            f"{vg_avg.get('Total_Liberado', 0):.0f}",
            f"{vg_avg.get('Bytes_Vazados', 0):.0f}",
            f"{vg_avg.get('Reachable', 0):.0f}",
        ]
        print(','.join(row))


if __name__ == '__main__':
    main()