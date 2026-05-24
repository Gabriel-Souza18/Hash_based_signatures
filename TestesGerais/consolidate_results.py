#!/usr/bin/env python3
"""
Script para consolidar resultados de métricas em uma tabela única
Combina: test_algorithms.sh (tempo/hashes) + Valgrind (memória)
"""

import os
import sys
import csv
from pathlib import Path
from collections import defaultdict
import re

def parse_algorithm_csv(csv_file):
    """Lê CSV de métricas de algoritmos"""
    data = defaultdict(list)
    
    try:
        with open(csv_file, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                algo = row['Algoritmo']
                data[algo].append(row)
    except Exception as e:
        print(f"Erro ao ler {csv_file}: {e}", file=sys.stderr)
    
    return data

def parse_valgrind_csv(csv_file):
    """Lê CSV de Valgrind"""
    data = defaultdict(list)
    
    try:
        with open(csv_file, 'r') as f:
            reader = csv.DictReader(f)
            for row in reader:
                algo = row['Algoritmo']
                data[algo].append(row)
    except Exception as e:
        print(f"Erro ao ler {csv_file}: {e}", file=sys.stderr)
    
    return data

def calculate_averages(rows, numeric_fields):
    """Calcula médias de campos numéricos"""
    averages = {}
    
    for field in numeric_fields:
        values = []
        for row in rows:
            try:
                val = float(row.get(field, 0))
                if val > 0:  # Ignorar zeros
                    values.append(val)
            except ValueError:
                pass
        
        if values:
            averages[field] = sum(values) / len(values)
        else:
            averages[field] = 0
    
    return averages

def get_latest_csv(directory, pattern):
    """Encontra o CSV mais recente que corresponde ao padrão"""
    files = list(Path(directory).glob(pattern))
    if files:
        return sorted(files)[-1]
    return None

def main():
    if len(sys.argv) < 2:
        print("Usage: consolidate_results.py <results_dir>", file=sys.stderr)
        sys.exit(1)
    
    results_dir = sys.argv[1]
    
    # Encontrar CSVs mais recentes
    algo_csv = get_latest_csv(results_dir, 'resultados_*.csv')
    valgrind_csv = get_latest_csv(results_dir, 'valgrind_bytes_*.csv')
    
    algo_data = {}
    valgrind_data = {}
    
    if algo_csv:
        algo_data = parse_algorithm_csv(str(algo_csv))
    else:
        print("Aviso: Nenhum CSV de algoritmos encontrado", file=sys.stderr)
    
    if valgrind_csv:
        valgrind_data = parse_valgrind_csv(str(valgrind_csv))
    else:
        print("Aviso: Nenhum CSV de Valgrind encontrado", file=sys.stderr)
    
    # Cabeçalho consolidado
    header = [
        'Algoritmo',
        'Tempo_Medio_SK_s',
        'Tempo_Medio_PK_s',
        'Tempo_Medio_Masks_s',
        'Tempo_Medio_Assinatura_s',
        'Hashes_Medio',
        'Tamanho_SK_bytes',
        'Tamanho_PK_bytes',
        'Tamanho_Assinatura_bytes',
        'Valgrind_Total_Alocado_bytes',
        'Valgrind_Total_Liberado_bytes',
        'Valgrind_Bytes_Vazados',
        'Valgrind_Reachable_bytes'
    ]
    
    print(','.join(header))
    
    # Processar cada algoritmo
    all_algos = set(algo_data.keys()) | set(valgrind_data.keys())
    
    for algo in sorted(all_algos):
        # Médias de tempo/performance
        algo_rows = algo_data.get(algo, [])
        algo_avg = calculate_averages(
            algo_rows,
            ['Tempo_SecretKeys', 'Tempo_PublicKeys', 'Tempo_Masks', 'Tempo_Assinatura', 'Hashes_Assinatura']
        )
        
        # Tamanhos (devem ser iguais para todas as linhas do mesmo algoritmo)
        tamanho_sk = tamanho_pk = tamanho_assinatura = 0
        if algo_rows:
            try:
                tamanho_sk = int(algo_rows[0].get('Tamanho_SecretKeys', 0))
                tamanho_pk = int(algo_rows[0].get('Tamanho_PublicKeys', 0))
                tamanho_assinatura = int(algo_rows[0].get('Tamanho_Assinatura', 0))
            except (ValueError, KeyError):
                pass
        
        # Médias de Valgrind
        valgrind_rows = valgrind_data.get(algo, [])
        valgrind_avg = calculate_averages(
            valgrind_rows,
            ['Total_Alocado', 'Total_Liberado', 'Bytes_Vazados', 'Reachable']
        )
        
        # Montar linha consolidada
        row = [
            algo,
            f"{algo_avg.get('Tempo_SecretKeys', 0):.6f}",
            f"{algo_avg.get('Tempo_PublicKeys', 0):.6f}",
            f"{algo_avg.get('Tempo_Masks', 0):.6f}",
            f"{algo_avg.get('Tempo_Assinatura', 0):.6f}",
            f"{algo_avg.get('Hashes_Assinatura', 0):.0f}",
            f"{tamanho_sk}",
            f"{tamanho_pk}",
            f"{tamanho_assinatura}",
            f"{valgrind_avg.get('Total_Alocado', 0):.0f}",
            f"{valgrind_avg.get('Total_Liberado', 0):.0f}",
            f"{valgrind_avg.get('Bytes_Vazados', 0):.0f}",
            f"{valgrind_avg.get('Reachable', 0):.0f}"
        ]
        
        print(','.join(row))

if __name__ == '__main__':
    main()
