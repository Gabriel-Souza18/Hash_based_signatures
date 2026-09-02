#!/bin/bash

# Script para testar HORST e MSS com número reduzido de execuções
# MSS com 1024 folhas leva ~5s por execução — 20 iterações ≈ 2 min
# HORST leva ~1s por execução — 20 iterações ≈ 20s

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  TESTE DE ALGORITMOS — HORST e MSS    ${NC}"
echo -e "${BLUE}========================================${NC}"

# Número de testes (reduzido dado o tempo elevado do MSS)
TESTES=${TESTES:-20}
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULTADO_DIR="resultados_metricas"
mkdir -p "$RESULTADO_DIR"
RESULTADO_FILE="$RESULTADO_DIR/resultados_mss_horst_${TIMESTAMP}.csv"

# Mensagem de teste padrão
MENSAGEM_TESTE="Esta é uma mensagem de teste para avaliar os algoritmos de assinatura digital."

# ── Compilação ────────────────────────────────────────────────────────────────
echo -e "${YELLOW}Compilando HORST...${NC}"
(cd ../HORST && make clean > /dev/null 2>&1 && make > /dev/null 2>&1)
if [ $? -ne 0 ]; then
    echo -e "${RED}Erro ao compilar HORST!${NC}"
    exit 1
fi
echo -e "${GREEN}HORST compilado com sucesso!${NC}"

echo -e "${YELLOW}Compilando MSS...${NC}"
(cd ../SHA256 && make > /dev/null 2>&1 && cd ../MSS && make clean > /dev/null 2>&1 && make > /dev/null 2>&1)
if [ $? -ne 0 ]; then
    echo -e "${RED}Erro ao compilar MSS!${NC}"
    exit 1
fi
echo -e "${GREEN}MSS compilado com sucesso!${NC}"

# ── Cabeçalho CSV ─────────────────────────────────────────────────────────────
HEADER="Algoritmo,Teste,Tempo_SecretKeys,Tempo_PublicKeys,Tempo_Masks,Tempo_Assinatura,Tempo_Verificacao,Hashes_Assinatura,Tamanho_SecretKeys,Tamanho_PublicKeys,Tamanho_Assinatura,Valgrind_Bytes,Valgrind_Erros"
printf "%s\n" "$HEADER" > "$RESULTADO_FILE"

echo -e "${GREEN}✓ Arquivo CSV criado: $RESULTADO_FILE${NC}"
echo -e "${BLUE}Header: $(head -1 "$RESULTADO_FILE")${NC}"
echo

# ── Funções auxiliares ────────────────────────────────────────────────────────
file_size_or_default() {
    local caminho="$1"
    local padrao="$2"
    if [ -f "$caminho" ]; then
        stat -c%s "$caminho"
    else
        echo "$padrao"
    fi
}

coletar_valgrind() {
    local executavel="$1"
    local dir="$2"
    local input="$3"
    local timeout_val="${4:-120}"
    shift 4
    local extra_args=("$@")

    local vg_log
    vg_log=$(mktemp /tmp/vg_XXXXXX.log)

    (cd "$dir" && echo -e "$input" | timeout "$timeout_val" \
        valgrind --tool=memcheck --leak-check=full --error-exitcode=0 \
        --log-file="$vg_log" \
        "$executavel" "${extra_args[@]}" > /dev/null 2>&1) || true

    local vg_output
    vg_output=$(cat "$vg_log" 2>/dev/null)
    rm -f "$vg_log"

    local bytes_uso
    bytes_uso=$(echo "$vg_output" | grep -i "total heap usage" | grep -oP '[0-9,]+(?= bytes allocated)' | tr -d ',' | head -1)
    bytes_uso=${bytes_uso:-"0"}

    local erros
    erros=$(echo "$vg_output" | grep -i "ERROR SUMMARY" | grep -oP '(?<=ERROR SUMMARY: )[0-9]+' | head -1)
    erros=${erros:-"0"}

    echo "${bytes_uso},${erros}"
}

# ── Função: testar HORST ──────────────────────────────────────────────────────
# ── Função: testar HORST ──────────────────────────────────────────────────────
testar_horst() {
    local teste_num=$1
    echo -e "${BLUE}Testando HORST - Teste $teste_num/${TESTES}${NC}"

    (cd ../HORST && rm -f *.bin mensagem.txt 2>/dev/null || true)

    local msg="mensagem_teste_horst_${teste_num}"
    local output_remet output_dest
    output_remet=$(cd ../HORST && timeout 30 ./remet_horst "$msg" 2>&1)
    local ret_remet=$?
    output_dest=$(cd ../HORST && timeout 30 ./dest_horst 2>&1)
    local ret_dest=$?

    local output="${output_remet}"$'\n'"${output_dest}"

    if [ $ret_remet -ne 0 ] || [ $ret_dest -ne 0 ]; then
        echo -e "${RED}Erro no teste HORST $teste_num${NC}"
        echo "HORST,$teste_num,0,0,0,0,0,0,32800,32,9152,0,0" >> "$RESULTADO_FILE"
        return 1
    fi

    local tempo_sk tempo_pk tempo_sign tempo_verif
    tempo_sk=$(echo "$output"   | grep "Tempo para gerar Chaves Secretas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_pk=$(echo "$output"   | grep "Tempo para gerar Chave Publica:"   | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_sign=$(echo "$output" | grep "Tempo para Assinar:"               | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_verif=$(echo "$output"| grep "Tempo Verificação:"                | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_sk=${tempo_sk:-"0"}
    tempo_pk=${tempo_pk:-"0"}
    tempo_sign=${tempo_sign:-"0"}
    tempo_verif=${tempo_verif:-"0"}

    local hashes_sign
    hashes_sign=$(echo "$output" | grep "Total de hashes SHA256 (assinatura)" | awk -F': ' '{print $2}')
    hashes_sign=${hashes_sign:-"0"}

    local tam_sk tam_pk tam_sign
    tam_sk=$(file_size_or_default "../HORST/seckeys.bin" "32800")
    tam_pk=$(file_size_or_default "../HORST/pubkey.bin"  "32")
    tam_sign=$(file_size_or_default "../HORST/assinatura.bin" "9152")

    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        echo -e "${YELLOW}  → Coletando métricas Valgrind (HORST)...${NC}"
        vg_metrics=$(coletar_valgrind "./remet_horst" "../HORST" "" 60 "mensagem_valgrind")
    fi

    echo "HORST,$teste_num,$tempo_sk,$tempo_pk,0,$tempo_sign,$tempo_verif,$hashes_sign,$tam_sk,$tam_pk,$tam_sign,$vg_metrics" >> "$RESULTADO_FILE"

    echo -e "${GREEN}  ✓ HORST $teste_num: SK=${tempo_sk}s | PK=${tempo_pk}s | Sign=${tempo_sign}s | Verif=${tempo_verif}s | Hashes=${hashes_sign}${NC}"
}

# ── Função: testar MSS ────────────────────────────────────────────────────────
testar_mss() {
    local teste_num=$1
    echo -e "${BLUE}Testando MSS  - Teste $teste_num/${TESTES}${NC}"

    (cd ../MSS && rm -f *.txt *.bin 2>/dev/null || true)
    printf "mensagem_teste_mss_%s\n" "$teste_num" > ../MSS/mensagem.txt

    local output_remet output_dest
    output_remet=$(cd ../MSS && timeout 60 ./remet_mss mensagem.txt public_key.txt assinatura.txt 2>&1)
    local ret_remet=$?
    output_dest=$(cd ../MSS && timeout 30 ./dest_mss mensagem.txt public_key.txt assinatura.txt 2>&1)
    local ret_dest=$?

    local output="${output_remet}"$'\n'"${output_dest}"

    if [ $ret_remet -ne 0 ] || [ $ret_dest -ne 0 ]; then
        echo -e "${RED}Erro no teste MSS $teste_num${NC}"
        echo "MSS,$teste_num,0,0,0,0,0,0,0,0,0,0,0" >> "$RESULTADO_FILE"
        return 1
    fi

    local tempo_folhas tempo_arvore tempo_sign tempo_verif
    tempo_folhas=$(echo "$output" | grep "Tempo para gerar Folhas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_arvore=$(echo "$output" | grep "Tempo para gerar Árvore:" | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_sign=$(echo "$output"   | grep "Tempo para Assinar:"      | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_verif=$(echo "$output"  | grep "Tempo Verificação:"       | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_folhas=${tempo_folhas:-"0"}
    tempo_arvore=${tempo_arvore:-"0"}
    tempo_sign=${tempo_sign:-"0"}
    tempo_verif=${tempo_verif:-"0"}

    local hashes_sign
    hashes_sign=$(echo "$output" | grep "Total de hashes SHA256 (assinatura):" | awk -F': ' '{print $2}')
    hashes_sign=${hashes_sign:-"0"}

    local tam_folhas tam_pubkey tam_sign_file
    tam_folhas=$(file_size_or_default "../MSS/folhas.txt"      "0")
    tam_pubkey=$(file_size_or_default  "../MSS/public_key.txt" "0")
    tam_sign_file=$(file_size_or_default "../MSS/assinatura.txt" "0")

    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        echo -e "${YELLOW}  → Coletando métricas Valgrind (MSS)...${NC}"
        vg_metrics=$(coletar_valgrind "./remet_mss" "../MSS" "" 120 "mensagem.txt" "public_key.txt" "assinatura.txt")
    fi

    echo "MSS,$teste_num,$tempo_folhas,$tempo_arvore,0,$tempo_sign,$tempo_verif,$hashes_sign,$tam_folhas,$tam_pubkey,$tam_sign_file,$vg_metrics" >> "$RESULTADO_FILE"

    echo -e "${GREEN}  ✓ MSS  $teste_num: Folhas=${tempo_folhas}s | Árvore=${tempo_arvore}s | Sign=${tempo_sign}s | Verif=${tempo_verif}s | Hashes=${hashes_sign}${NC}"
}

# ── Loop principal ─────────────────────────────────────────────────────────────
echo -e "${YELLOW}Iniciando $TESTES execuções para HORST e MSS...${NC}"
echo -e "${YELLOW}Tempo estimado: HORST ~$((TESTES * 2))s | MSS ~$((TESTES * 12))s${NC}"
echo

for i in $(seq 1 $TESTES); do
    echo -e "${YELLOW}=== EXECUÇÃO $i/$TESTES ===${NC}"

    testar_horst $i
    sleep 1

    testar_mss $i
    sleep 1

    echo
done

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  TESTES CONCLUÍDOS!                   ${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Resultados salvos em: ${YELLOW}$RESULTADO_FILE${NC}"

# Exporta automaticamente para JSON
JSON_FILE="${RESULTADO_FILE%.csv}.json"
python3 export_results_to_json.py "$RESULTADO_FILE" "$JSON_FILE"
echo -e "${GREEN}Resultados em JSON:   ${YELLOW}$JSON_FILE${NC}"
echo

echo -e "${GREEN}Script finalizado!${NC}"
