#!/usr/bin/env python3
"""
export_results_to_json.py
Converte arquivos CSV de métricas de testes de assinaturas baseadas em hash
para formato JSON estruturado, suportando tanto CSVs de execuções individuais
quanto CSVs de tabelas consolidadas.
"""

import sys
import os
import csv
import json
from datetime import datetime
from collections import defaultdict
import statistics

def parse_float(val, default=0.0):
    try:
        return float(val)
    except (ValueError, TypeError):
        return default

def parse_int(val, default=0):
    try:
        return int(float(val))
    except (ValueError, TypeError):
        return default

def csv_to_json(csv_path, json_path=None):
    if not os.path.exists(csv_path):
        print(f"Erro: Arquivo CSV não encontrado: {csv_path}", file=sys.stderr)
        return None

    if json_path is None:
        json_path = os.path.splitext(csv_path)[0] + ".json"

    with open(csv_path, mode="r", encoding="utf-8") as f:
        reader = list(csv.DictReader(f))

    if not reader:
        print(f"Aviso: Arquivo CSV vazio: {csv_path}", file=sys.stderr)
        return None

    first_row = reader[0]
    is_consolidated = "Tempo_Medio_Assinatura_s" in first_row or "Tempo_Medio_SK_s" in first_row

    if is_consolidated:
        output = {
            "timestamp_geracao": datetime.now().isoformat(),
            "arquivo_origem_csv": os.path.basename(csv_path),
            "tipo": "consolidado_medias",
            "total_algoritmos": len(reader),
            "algoritmos": {}
        }
        for row in reader:
            algo = row.get("Algoritmo", "").strip()
            if not algo:
                continue
            output["algoritmos"][algo] = {
                "tempo_medio_sk_s": parse_float(row.get("Tempo_Medio_SK_s", 0)),
                "tempo_medio_pk_s": parse_float(row.get("Tempo_Medio_PK_s", 0)),
                "tempo_medio_masks_s": parse_float(row.get("Tempo_Medio_Masks_s", 0)),
                "tempo_medio_assinatura_s": parse_float(row.get("Tempo_Medio_Assinatura_s", 0)),
                "tempo_medio_verificacao_s": parse_float(row.get("Tempo_Medio_Verificacao_s", 0)),
                "hashes_medio": parse_int(row.get("Hashes_Medio", 0)),
                "tamanho_sk_bytes": parse_int(row.get("Tamanho_SK_bytes", 0)),
                "tamanho_pk_bytes": parse_int(row.get("Tamanho_PK_bytes", 0)),
                "tamanho_assinatura_bytes": parse_int(row.get("Tamanho_Assinatura_bytes", 0)),
                "valgrind_total_alocado_bytes": parse_int(row.get("Valgrind_Total_Alocado_bytes", 0)),
                "valgrind_total_liberado_bytes": parse_int(row.get("Valgrind_Total_Liberado_bytes", 0)),
                "valgrind_bytes_vazados": parse_int(row.get("Valgrind_Bytes_Vazados", 0)),
                "valgrind_reachable_bytes": parse_int(row.get("Valgrind_Reachable_bytes", 0)),
            }
    else:
        algos_data = defaultdict(list)
        for row in reader:
            algo = row.get("Algoritmo", "").strip()
            if not algo:
                continue

            exec_data = {
                "teste": parse_int(row.get("Teste", 0)),
                "tempo_secret_keys_s": parse_float(row.get("Tempo_SecretKeys", 0)),
                "tempo_public_keys_s": parse_float(row.get("Tempo_PublicKeys", 0)),
                "tempo_masks_s": parse_float(row.get("Tempo_Masks", 0)),
                "tempo_assinatura_s": parse_float(row.get("Tempo_Assinatura", 0)),
                "tempo_verificacao_s": parse_float(row.get("Tempo_Verificacao", 0)),
                "hashes_assinatura": parse_int(row.get("Hashes_Assinatura", 0)),
                "tamanho_secret_keys_bytes": parse_int(row.get("Tamanho_SecretKeys", 0)),
                "tamanho_public_keys_bytes": parse_int(row.get("Tamanho_PublicKeys", 0)),
                "tamanho_assinatura_bytes": parse_int(row.get("Tamanho_Assinatura", 0)),
                "valgrind_bytes": parse_int(row.get("Valgrind_Bytes", 0)),
                "valgrind_erros": parse_int(row.get("Valgrind_Erros", 0)),
            }
            algos_data[algo].append(exec_data)

        output = {
            "timestamp_geracao": datetime.now().isoformat(),
            "arquivo_origem_csv": os.path.basename(csv_path),
            "tipo": "execucoes_individuais_e_resumo",
            "total_algoritmos": len(algos_data),
            "algoritmos": {}
        }

        for algo, execs in algos_data.items():
            total_execs = len(execs)
            
            def calc_stat(key, int_mode=False):
                vals = [e[key] for e in execs if e[key] > 0]
                if not vals:
                    return 0 if int_mode else 0.0
                avg = sum(vals) / len(vals)
                return round(avg) if int_mode else round(avg, 6)

            def calc_min_max_stdev(key):
                vals = [e[key] for e in execs if e[key] > 0]
                if not vals:
                    return {"min": 0.0, "max": 0.0, "stdev": 0.0}
                return {
                    "min": round(min(vals), 6),
                    "max": round(max(vals), 6),
                    "stdev": round(statistics.stdev(vals), 6) if len(vals) > 1 else 0.0
                }

            first_e = execs[0] if execs else {}
            vg_bytes = max(e["valgrind_bytes"] for e in execs) if execs else 0
            vg_erros = max(e["valgrind_erros"] for e in execs) if execs else 0

            resumo = {
                "total_execucoes": total_execs,
                "tempo_secret_keys_medio_s": calc_stat("tempo_secret_keys_s"),
                "tempo_public_keys_medio_s": calc_stat("tempo_public_keys_s"),
                "tempo_masks_medio_s": calc_stat("tempo_masks_s"),
                "tempo_assinatura_medio_s": calc_stat("tempo_assinatura_s"),
                "tempo_verificacao_medio_s": calc_stat("tempo_verificacao_s"),
                "tempo_verificacao_stats": calc_min_max_stdev("tempo_verificacao_s"),
                "tempo_assinatura_stats": calc_min_max_stdev("tempo_assinatura_s"),
                "hashes_assinatura_medio": calc_stat("hashes_assinatura", int_mode=True),
                "tamanho_secret_keys_bytes": first_e.get("tamanho_secret_keys_bytes", 0),
                "tamanho_public_keys_bytes": first_e.get("tamanho_public_keys_bytes", 0),
                "tamanho_assinatura_bytes": first_e.get("tamanho_assinatura_bytes", 0),
                "valgrind_bytes_alocados": vg_bytes,
                "valgrind_erros": vg_erros
            }

            output["algoritmos"][algo] = {
                "resumo": resumo,
                "execucoes": execs
            }

    with open(json_path, mode="w", encoding="utf-8") as f:
        json.dump(output, f, indent=2, ensure_ascii=False)

    print(f"✓ JSON gerado com sucesso: {json_path}")
    return json_path

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Uso: python3 export_results_to_json.py <arquivo_csv> [arquivo_json_saida]")
        sys.exit(1)

    csv_arg = sys.argv[1]
    json_arg = sys.argv[2] if len(sys.argv) >= 3 else None
    csv_to_json(csv_arg, json_arg)
