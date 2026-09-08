#!/usr/bin/env python3
"""
gerar_grafico_tamanhos.py
Gera gráficos científicos comparativos dos tamanhos de Chaves Públicas,
Chaves Secretas e Assinaturas para os 6 esquemas baseados em hash.
"""

import os
import sys
import json
import numpy as np
import matplotlib.pyplot as plt
from pathlib import Path

def carregar_dados_consolidados(json_path=None):
    if json_path is None:
        metricas_dir = Path(__file__).resolve().parent.parent / "resultados_metricas"
        candidatos = sorted(list(metricas_dir.glob("consolidated_results_*.json")))
        if not candidatos:
            raise FileNotFoundError("Nenhum arquivo consolidated_results_*.json encontrado!")
        json_path = candidatos[-1]
    
    print(f"Lendo dados de: {json_path}")
    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)
    return data, json_path

def formatar_bytes(val):
    if val >= 1024:
        return f"{val/1024:.1f} KB"
    return f"{val} B"

def gerar_graficos():
    data, json_path = carregar_dados_consolidados()
    algos_dict = data.get("algoritmos", {})

    ordem_algos = ["LOTS", "WOTS", "HORS", "HORST", "MSS", "SPHINCS"]
    algos = [a for a in ordem_algos if a in algos_dict]

    # Chave secreta com MSS omitida para escala linear comparável
    sk_sizes = []
    for a in algos:
        if a == "MSS":
            sk_sizes.append(0)  # SK do MSS omitida conforme solicitado
        else:
            sk_sizes.append(algos_dict[a]["tamanho_sk_bytes"])
    pk_sizes = [algos_dict[a]["tamanho_pk_bytes"] for a in algos]
    sig_sizes = [algos_dict[a]["tamanho_assinatura_bytes"] for a in algos]

    # Conversão para KB
    sk_kb = [s / 1024 for s in sk_sizes]
    pk_kb = [s / 1024 for s in pk_sizes]
    sig_kb = [s / 1024 for s in sig_sizes]

    # Configuração de estilo visual acadêmico
    plt.style.use('seaborn-v0_8-whitegrid' if 'seaborn-v0_8-whitegrid' in plt.style.available else 'default')
    fig, (ax1, ax2) = plt.subplots(1, 2, figsize=(16, 7), dpi=300)

    # ──────────────────────────────────────────────────────────────────────────
    # GRÁFICO 1: Barras Agrupadas com Escala Linear (sem SK do MSS)
    # ──────────────────────────────────────────────────────────────────────────
    x = np.arange(len(algos))
    largura = 0.26

    cores = {
        "SK": "#1f77b4",   # Azul
        "PK": "#2ca02c",   # Verde
        "SIG": "#d62728"   # Vermelho
    }

    rects1 = ax1.bar(x - largura, sk_kb, largura, label="Chave Secreta (SK)*", color=cores["SK"], edgecolor='black', linewidth=0.8, alpha=0.9)
    rects2 = ax1.bar(x,           pk_kb, largura, label="Chave Pública (PK)",  color=cores["PK"], edgecolor='black', linewidth=0.8, alpha=0.9)
    rects3 = ax1.bar(x + largura, sig_kb, largura, label="Assinatura",          color=cores["SIG"], edgecolor='black', linewidth=0.8, alpha=0.9)

    ax1.set_ylabel("Tamanho em Kilobytes (KB)", fontsize=12, fontweight='bold')
    ax1.set_title("Comparação Geral de Tamanhos (Escala Linear em KB)", fontsize=13, fontweight='bold', pad=12)
    ax1.set_xticks(x)
    ax1.set_xticklabels(algos, fontsize=11, fontweight='bold')
    ax1.set_ylim(0, max(max(sk_kb), max(pk_kb), max(sig_kb)) * 1.18)
    ax1.legend(loc='upper left', frameon=True, framealpha=0.95, fontsize=10)
    ax1.grid(True, ls="--", alpha=0.4)

    # Rótulos dos valores sobre as barras do Gráfico 1
    for rects, orig_bytes in [(rects1, sk_sizes), (rects2, pk_sizes), (rects3, sig_sizes)]:
        for i, rect in enumerate(rects):
            height = rect.get_height()
            b = orig_bytes[i]
            if b > 0:
                texto = f"{height:.1f} KB" if height >= 1 else f"{b} B"
                ax1.annotate(texto,
                            xy=(rect.get_x() + rect.get_width() / 2, height),
                            xytext=(0, 4),
                            textcoords="offset points",
                            ha='center', va='bottom',
                            fontsize=8, rotation=35)
            elif rect in rects1 and algos[i] == "MSS":
                ax1.annotate("(Omitida)*",
                            xy=(rect.get_x() + rect.get_width() / 2, 0.4),
                            xytext=(0, 2),
                            textcoords="offset points",
                            ha='center', va='bottom',
                            fontsize=7.5, color="#555555", style='italic')

    # Nota explicativa na base da figura
    fig.text(0.05, 0.01, "* Nota: Chave Secreta do MSS omitida no gráfico da esquerda para preservar a escala linear comparável.",
             fontsize=9, style='italic', color='#444444')

    # ──────────────────────────────────────────────────────────────────────────
    # GRÁFICO 2: Foco em Chave Pública vs Tamanho da Assinatura (Bytes reais)
    # ──────────────────────────────────────────────────────────────────────────
    largura2 = 0.35
    rects_pk = ax2.bar(x - largura2/2, [s/1024 for s in pk_sizes], largura2, label="Chave Pública (PK)", color=cores["PK"], edgecolor='black', linewidth=0.8, alpha=0.9)
    rects_sig = ax2.bar(x + largura2/2, [s/1024 for s in sig_sizes], largura2, label="Assinatura", color=cores["SIG"], edgecolor='black', linewidth=0.8, alpha=0.9)

    ax2.set_ylabel("Tamanho em Kilobytes (KB)", fontsize=12, fontweight='bold')
    ax2.set_title("Chave Pública vs Assinatura (Escala Linear em KB)", fontsize=13, fontweight='bold', pad=12)
    ax2.set_xticks(x)
    ax2.set_xticklabels(algos, fontsize=11, fontweight='bold')
    ax2.legend(loc='upper left', frameon=True, framealpha=0.95, fontsize=10)
    ax2.grid(True, ls="--", alpha=0.4)

    for rect in rects_pk:
        height = rect.get_height()
        ax2.annotate(f"{height:.2f} KB" if height >= 1 else f"{height*1024:.0f} B",
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),
                    textcoords="offset points",
                    ha='center', va='bottom',
                    fontsize=8, rotation=30)

    for rect in rects_sig:
        height = rect.get_height()
        ax2.annotate(f"{height:.2f} KB" if height >= 1 else f"{height*1024:.0f} B",
                    xy=(rect.get_x() + rect.get_width() / 2, height),
                    xytext=(0, 3),
                    textcoords="offset points",
                    ha='center', va='bottom',
                    fontsize=8, rotation=30)

    plt.suptitle("Comparativo de Tamanhos de Chaves e Assinaturas — Assinaturas Baseadas em Hash", 
                 fontsize=15, fontweight='bold', y=0.99)
    plt.tight_layout()

    # Salva figura
    output_dir = Path(__file__).resolve().parent
    output_path = output_dir / "comparacao_tamanhos_chaves_assinaturas.png"
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    plt.close()

    print(f"\n✓ Gráfico gerado com sucesso: {output_path}")
    return output_path

if __name__ == "__main__":
    gerar_graficos()
