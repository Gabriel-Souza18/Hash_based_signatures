#!/usr/bin/env python3
"""
Script para coletar métricas de Valgrind (heap bytes allocados/livrados)
dos arquivos de log Valgrind
"""

import os
import sys
import re
from pathlib import Path

def extract_valgrind_metrics(valgrind_file):
    """Extrai métricas de um arquivo .valgrind"""
    metrics = {
        'total_alloc': 0,
        'total_freed': 0,
        'bytes_leaked': 0,
        'reachable': 0
    }
    
    try:
        with open(valgrind_file, 'r') as f:
            content = f.read()
            
            # Padrões para extrair valores
            # Total alloc'd
            match = re.search(r'total\s+alloc\'d\s+(\d+)', content)
            if match:
                metrics['total_alloc'] = int(match.group(1))
            
            # Total freed
            match = re.search(r'total\s+freed\s+(\d+)', content)
            if match:
                metrics['total_freed'] = int(match.group(1))
            
            # Definitely lost
            match = re.search(r'(?:definitely lost|lost):\s+(\d+)\s+bytes', content)
            if match:
                metrics['bytes_leaked'] = int(match.group(1))
            
            # Reachable
            match = re.search(r'reachable:\s+(\d+)\s+bytes', content)
            if match:
                metrics['reachable'] = int(match.group(1))
    
    except Exception as e:
        print(f"Erro ao ler {valgrind_file}: {e}", file=sys.stderr)
    
    return metrics

def parse_filename(filename):
    """Extrai algoritmo e tipo de execução do nome do arquivo"""
    # Formato: ALGORITMO_EXECUTAVEL_TAG.valgrind
    # Ex: LOTS_lots_remetente.valgrind
    parts = filename.replace('.valgrind', '').replace('.stdout', '').split('_')
    if len(parts) >= 2:
        algo = parts[0]
        tag = '_'.join(parts[2:]) if len(parts) > 2 else 'unknown'
        return algo, tag
    return 'unknown', 'unknown'

def main():
    if len(sys.argv) < 2:
        print("Usage: collect_valgrind_metrics.py <valgrind_logs_dir>", file=sys.stderr)
        sys.exit(1)
    
    valgrind_dir = sys.argv[1]
    
    # CSV Header
    print("Algoritmo,Tag,Total_Alocado,Total_Liberado,Bytes_Vazados,Reachable")
    
    # Processar todos os arquivos .valgrind
    for valgrind_file in sorted(Path(valgrind_dir).glob('*.valgrind')):
        algo, tag = parse_filename(valgrind_file.name)
        metrics = extract_valgrind_metrics(str(valgrind_file))
        
        print(f"{algo},{tag},{metrics['total_alloc']},{metrics['total_freed']},{metrics['bytes_leaked']},{metrics['reachable']}")

if __name__ == '__main__':
    main()
