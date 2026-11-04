def met_lots():
    resultados = {
        "GEN_SK": {"tempo": [], "ram": [], "hashes": []},
        "GEN_PK": {"tempo": [], "ram": [], "hashes": []},
        "ASSINATURA": {"tempo": [], "ram": [], "hashes": []},
        "VERIFICACAO": {"tempo": [], "ram": [], "hashes": []},
    }

    validas = 0
    invalidas = 0

    with open("ResultadosLOTS.txt", "r", encoding="utf-8") as arquivo:
        for linha in arquivo:
            campos = {}

            for item in linha.split():
                if "=" in item:
                    chave, valor = item.split("=", 1)
                    campos[chave] = valor

            operacao = campos.get("OPERACAO")
            if operacao in resultados:
                try:
                    resultados[operacao]["tempo"].append(
                        int(campos["TEMPO_US"])
                    )
                    resultados[operacao]["ram"].append(
                        int(campos["RAM_BYTES"])
                    )
                    resultados[operacao]["hashes"].append(
                        int(campos["HASHES"])
                    )
                except (KeyError, ValueError):
                    continue

            if campos.get("RESULTADO") == "VALIDA":
                validas += 1
            elif campos.get("RESULTADO") == "INVALIDA":
                invalidas += 1

    print("ALGORITMO=LOTS")

    for operacao, valores in resultados.items():
        if not valores["tempo"]:
            continue

        quantidade = len(valores["tempo"])
        tempo = sum(valores["tempo"]) / quantidade
        ram = sum(valores["ram"]) / quantidade
        hashes = sum(valores["hashes"]) / quantidade

        print(
            f"OPERACAO={operacao} "
            f"EXECUCOES={quantidade} "
            f"TEMPO_US_MEDIO={tempo:.2f} "
            f"HASHES_MEDIO={hashes:.2f} "
            f"RAM_BYTES_MEDIO={ram:.2f}"
        )

    print(f"VALIDAS={validas} INVALIDAS={invalidas}")


if __name__ == "__main__":
    met_lots()
