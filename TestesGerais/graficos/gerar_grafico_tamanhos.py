#!/usr/bin/env python3
"""
gerar_grafico_tamanhos.py
Gera dois gráficos científicos comparativos dos tamanhos de chaves e assinaturas
para os 6 esquemas baseados em hash.

Figura 1: SK, PK e Assinatura — escala logarítmica (cobre todas as ordens de grandeza)
Figura 2: PK vs Assinatura  — escala linear em KB
"""

import json
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt


# ─────────────────────────────────────────────────────────────────────────────
# Configuração geral
# ─────────────────────────────────────────────────────────────────────────────

ORDEM_ALGOS = ["LOTS", "WOTS", "HORS", "HORST", "MSS", "SPHINCS"]

CORES = {
    "SK":  "#1f77b4",   # azul
    "PK":  "#2ca02c",   # verde
    "SIG": "#d62728",   # vermelho
}

NOTA_SK = (
    "* Estratégia de armazenamento da SK varia por esquema: "
    "LOTS, WOTS+ e HORS armazenam valores expandidos; "
    "HORST e SPHINCS armazenam sementes; "
    "MSS armazena as folhas WOTS+ completas."
)


# ─────────────────────────────────────────────────────────────────────────────
# Carregamento de dados
# ─────────────────────────────────────────────────────────────────────────────

def carregar_dados(json_path=None):
    if json_path is None:
        metricas_dir = Path(__file__).resolve().parent.parent / "resultados_metricas"
        candidatos = sorted(metricas_dir.glob("consolidated_results_*.json"))
        if not candidatos:
            raise FileNotFoundError(
                "Nenhum arquivo consolidated_results_*.json encontrado em "
                f"{metricas_dir}"
            )
        json_path = candidatos[-1]

    print(f"Lendo dados de: {json_path}")
    with open(json_path, "r", encoding="utf-8") as f:
        data = json.load(f)

    algos_dict = data.get("algoritmos", {})
    algos = [a for a in ORDEM_ALGOS if a in algos_dict]

    sk  = [algos_dict[a]["tamanho_sk_bytes"]        for a in algos]
    pk  = [algos_dict[a]["tamanho_pk_bytes"]        for a in algos]
    sig = [algos_dict[a]["tamanho_assinatura_bytes"] for a in algos]

    return algos, sk, pk, sig, json_path


# ─────────────────────────────────────────────────────────────────────────────
# Helpers
# ─────────────────────────────────────────────────────────────────────────────

def formatar(bytes_val):
    """Formata bytes em B ou KB, com 1 decimal quando em KB."""
    if bytes_val >= 1024:
        return f"{bytes_val / 1024:.1f} KB"
    return f"{bytes_val} B"


def aplicar_estilo():
    """Aplica um estilo visual limpo, com fallback caso seaborn não exista."""
    if "seaborn-v0_8-whitegrid" in plt.style.available:
        plt.style.use("seaborn-v0_8-whitegrid")
    else:
        plt.style.use("default")


# ─────────────────────────────────────────────────────────────────────────────
# Figura 1 — SK, PK e Assinatura em escala logarítmica
# ─────────────────────────────────────────────────────────────────────────────

def gerar_figura_tamanhos_geral(algos, sk, pk, sig, out_dir):
    fig, ax = plt.subplots(figsize=(11, 6), dpi=300)

    x = np.arange(len(algos))
    largura = 0.26

    # Converte para KB; uso de log exige valores > 0
    sk_kb  = np.array([max(v, 1) for v in sk])  / 1024
    pk_kb  = np.array([max(v, 1) for v in pk])  / 1024
    sig_kb = np.array([max(v, 1) for v in sig]) / 1024

    rects_sk  = ax.bar(x - largura, sk_kb,  largura, label="Chave Secreta (SK)",
                       color=CORES["SK"],  edgecolor="black", linewidth=0.8)
    rects_pk  = ax.bar(x,           pk_kb,  largura, label="Chave Pública (PK)",
                       color=CORES["PK"],  edgecolor="black", linewidth=0.8)
    rects_sig = ax.bar(x + largura, sig_kb, largura, label="Assinatura",
                       color=CORES["SIG"], edgecolor="black", linewidth=0.8)

    ax.set_yscale("log")
    ax.set_ylim(0.02, 500)   # 20 B a 500 KB
    ax.set_ylabel("Tamanho (KB, escala logarítmica)", fontsize=12, fontweight="bold")
    ax.set_title("Tamanhos de chave e assinatura por esquema",
                 fontsize=13, fontweight="bold", pad=12)
    ax.set_xticks(x)
    ax.set_xticklabels(algos, fontsize=11, fontweight="bold")
    ax.grid(True, which="both", ls="--", alpha=0.35)
    ax.legend(loc="upper left", frameon=True, framealpha=0.95, fontsize=10)

    # Anotações: valores reais (não os de escala)
    def anotar(rects, valores):
        for rect, b in zip(rects, valores):
            h = rect.get_height()
            ax.annotate(formatar(b),
                        xy=(rect.get_x() + rect.get_width() / 2, h),
                        xytext=(0, 3),
                        textcoords="offset points",
                        ha="center", va="bottom",
                        fontsize=7.5, rotation=35)

    anotar(rects_sk,  sk)
    anotar(rects_pk,  pk)
    anotar(rects_sig, sig)

    # Nota sobre SK na base da figura
    fig.text(0.5, 0.005, NOTA_SK,
             ha="center", va="bottom",
             fontsize=8, style="italic", color="#444444",
             wrap=True)

    plt.tight_layout(rect=(0, 0.04, 1, 1))

    out_path = out_dir / "fig_tamanhos_geral.png"
    plt.savefig(out_path, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"✓ Figura 1 salva em: {out_path}")
    return out_path


# ─────────────────────────────────────────────────────────────────────────────
# Figura 2 — PK vs Assinatura em escala linear
# ─────────────────────────────────────────────────────────────────────────────

def gerar_figura_pk_vs_sig(algos, pk, sig, out_dir):
    fig, ax = plt.subplots(figsize=(10, 6), dpi=300)

    x = np.arange(len(algos))
    largura = 0.36

    pk_kb  = np.array(pk)  / 1024
    sig_kb = np.array(sig) / 1024

    rects_pk  = ax.bar(x - largura / 2, pk_kb,  largura,
                       label="Chave Pública (PK)",
                       color=CORES["PK"],  edgecolor="black", linewidth=0.8)
    rects_sig = ax.bar(x + largura / 2, sig_kb, largura,
                       label="Assinatura",
                       color=CORES["SIG"], edgecolor="black", linewidth=0.8)

    ax.set_ylabel("Tamanho (KB)", fontsize=12, fontweight="bold")
    ax.set_title("Chave pública vs. tamanho da assinatura",
                 fontsize=13, fontweight="bold", pad=12)
    ax.set_xticks(x)
    ax.set_xticklabels(algos, fontsize=11, fontweight="bold")
    ax.grid(True, ls="--", alpha=0.4)
    ax.legend(loc="upper left", frameon=True, framealpha=0.95, fontsize=10)

    # Limite superior com folga para os rótulos
    ymax = max(pk_kb.max(), sig_kb.max()) * 1.18
    ax.set_ylim(0, ymax)

    def anotar(rects, valores):
        for rect, b in zip(rects, valores):
            h = rect.get_height()
            ax.annotate(formatar(b),
                        xy=(rect.get_x() + rect.get_width() / 2, h),
                        xytext=(0, 3),
                        textcoords="offset points",
                        ha="center", va="bottom",
                        fontsize=8, rotation=30)

    anotar(rects_pk,  pk)
    anotar(rects_sig, sig)

    plt.tight_layout()

    out_path = out_dir / "fig_pk_vs_sig.png"
    plt.savefig(out_path, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"✓ Figura 2 salva em: {out_path}")
    return out_path


# ─────────────────────────────────────────────────────────────────────────────
# Main
# ─────────────────────────────────────────────────────────────────────────────

def main():
    aplicar_estilo()
    algos, sk, pk, sig, _ = carregar_dados()

    out_dir = Path(__file__).resolve().parent
    gerar_figura_tamanhos_geral(algos, sk, pk, sig, out_dir)
    gerar_figura_pk_vs_sig(algos, pk, sig, out_dir)

    print("\nTodas as figuras geradas com sucesso.")


if __name__ == "__main__":
    main()