#!/usr/bin/env python3
import csv
import re
from pathlib import Path


RE_TOTAL = re.compile(
    r"total heap usage:\s*([\d,]+)\s*allocs,\s*([\d,]+)\s*frees,\s*([\d,]+)\s*bytes allocated",
    re.IGNORECASE,
)
RE_IN_USE = re.compile(r"in use at exit:\s*([\d,]+)\s*bytes", re.IGNORECASE)


def to_int(numero: str) -> int:
    return int(numero.replace(",", "").strip())


def parse_nome_arquivo(path: Path):
    # Ex.: HORS_hors_test_destinatario / MSS_mss_remetente_gerar_arvore
    stem = path.stem

    if "_" not in stem:
        return stem, "", ""

    algoritmo, resto = stem.split("_", 1)

    marcadores = ["_remetente", "_destinatario"]
    pos_marcador = -1
    marcador_usado = ""
    for marcador in marcadores:
        pos = resto.rfind(marcador)
        if pos > pos_marcador:
            pos_marcador = pos
            marcador_usado = marcador

    if pos_marcador == -1:
        return algoritmo, resto, ""

    executavel = resto[:pos_marcador]
    papel = resto[pos_marcador + 1 :]  # remove underscore inicial

    if not executavel:
        executavel = resto.replace(marcador_usado, "", 1)

    return algoritmo, executavel, papel


def extrair_metricas(texto: str):
    total = RE_TOTAL.search(texto)
    in_use = RE_IN_USE.search(texto)

    if total:
        allocs = to_int(total.group(1))
        frees = to_int(total.group(2))
        bytes_alocados = to_int(total.group(3))
    else:
        allocs = 0
        frees = 0
        bytes_alocados = 0

    bytes_em_uso_saida = to_int(in_use.group(1)) if in_use else 0

    return allocs, frees, bytes_alocados, bytes_em_uso_saida


def main():
    raiz = Path(__file__).resolve().parent

    diretorio_logs = raiz / "resultados_valgrind"
    if not diretorio_logs.exists():
        diretorio_logs = raiz / "resultados" / "valgrind"

    if not diretorio_logs.exists():
        print("Nenhum diretório de logs encontrado.")
        return

    arquivos = sorted(diretorio_logs.glob("*.valgrind"))
    if not arquivos:
        print(f"Nenhum arquivo .valgrind encontrado em: {diretorio_logs}")
        return

    linhas = []
    for arq in arquivos:
        texto = arq.read_text(encoding="utf-8", errors="ignore")
        algoritmo, executavel, papel = parse_nome_arquivo(arq)
        allocs, frees, bytes_alocados, bytes_em_uso_saida = extrair_metricas(texto)

        linhas.append(
            {
                "arquivo": arq.name,
                "algoritmo": algoritmo,
                "executavel": executavel,
                "papel": papel,
                "allocs": allocs,
                "frees": frees,
                "bytes_alocados": bytes_alocados,
                "bytes_em_uso_na_saida": bytes_em_uso_saida,
            }
        )

    saida_csv = raiz / "tabela_bytes_valgrind.csv"
    with saida_csv.open("w", newline="", encoding="utf-8") as f:
        writer = csv.DictWriter(
            f,
            fieldnames=[
                "arquivo",
                "algoritmo",
                "executavel",
                "papel",
                "allocs",
                "frees",
                "bytes_alocados",
                "bytes_em_uso_na_saida",
            ],
        )
        writer.writeheader()
        writer.writerows(linhas)

    print(f"Tabela gerada: {saida_csv}")
    print(f"Arquivos processados: {len(linhas)}")


if __name__ == "__main__":
    main()
