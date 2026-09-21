#!/bin/bash

# Script para testar HORST, MSS e SPHINCS
# NOVO: três colunas de hashes — Keygen, Assinatura e Verificação

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  TESTE — HORST, MSS e SPHINCS         ${NC}"
echo -e "${BLUE}========================================${NC}"

TESTES=${TESTES:-10}
USAR_VALGRIND=${USAR_VALGRIND:-0}
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULTADO_DIR="resultados_metricas"
mkdir -p "$RESULTADO_DIR"
RESULTADO_FILE="$RESULTADO_DIR/resultados_heavy_${TIMESTAMP}.csv"

MENSAGEM_TESTE="Esta é uma mensagem de teste para avaliar os algoritmos de assinatura digital."

# ── Compilação ────────────────────────────────────────────────
echo -e "${YELLOW}Compilando HORST...${NC}"
(cd ../HORST && make clean > /dev/null 2>&1 && make > /dev/null 2>&1) || { echo -e "${RED}Erro HORST${NC}"; exit 1; }

echo -e "${YELLOW}Compilando MSS...${NC}"
(cd ../SHA256 && make > /dev/null 2>&1 && cd ../MSS && make clean > /dev/null 2>&1 && make > /dev/null 2>&1) || { echo -e "${RED}Erro MSS${NC}"; exit 1; }

echo -e "${YELLOW}Compilando SPHINCS...${NC}"
(cd ../SPHINCS && make clean > /dev/null 2>&1 && make > /dev/null 2>&1) || { echo -e "${RED}Erro SPHINCS${NC}"; exit 1; }

echo -e "${GREEN}Compilação concluída!${NC}"
echo

# ── Cabeçalho do CSV ──────────────────────────────────────────
HEADER="Algoritmo,Teste,Tempo_SecretKeys,Tempo_PublicKeys,Tempo_Masks,Tempo_Assinatura,Tempo_Verificacao,Hashes_Keygen,Hashes_Assinatura,Hashes_Verificacao,Tamanho_SecretKeys,Tamanho_PublicKeys,Tamanho_Assinatura"
printf "%s\n" "$HEADER" > "$RESULTADO_FILE"
echo -e "${GREEN}✓ CSV: $RESULTADO_FILE${NC}"
echo

# ── Auxiliares ────────────────────────────────────────────────
extrair_valor() {
    local texto="$1"
    local padrao="$2"
    echo "$texto" | grep "$padrao" | sed 's/.*: *\([0-9.]*\).*/\1/' | head -1
}

file_size_or_default() {
    local caminho="$1"
    local padrao="$2"
    if [ -f "$caminho" ]; then
        stat -c%s "$caminho"
    else
        echo "$padrao"
    fi
}

# Extrai 3 hashes. Aceita formatos novos (Hashes Keygen/Assinatura/Verificacao)
# e cai para formatos antigos como fallback.
extrair_hashes() {
    local texto="$1"
    local h_keygen h_sign h_verif

    h_keygen=$(echo "$texto" | grep -E "Hashes Keygen:"     | awk -F': ' '{print $2}' | awk '{print $1}' | head -1)
    h_sign=$(echo   "$texto" | grep -E "Hashes Assinatura:" | awk -F': ' '{print $2}' | awk '{print $1}' | head -1)
    h_verif=$(echo  "$texto" | grep -E "Hashes Verificacao:"| awk -F': ' '{print $2}' | awk '{print $1}' | head -1)

    if [ -z "$h_sign" ]; then
        h_sign=$(echo "$texto" | grep -E "Total de hashes SHA256 \(assinatura\):|Total de hashes SHA256:" | awk -F': ' '{print $2}' | awk '{print $1}' | head -1)
    fi

    echo "${h_keygen:-0},${h_sign:-0},${h_verif:-0}"
}

# ── HORST ─────────────────────────────────────────────────────
testar_horst() {
    local teste_num=$1
    echo -e "${BLUE}Testando HORST - $teste_num/${TESTES}${NC}"

    (cd ../HORST && rm -f *.bin mensagem.txt 2>/dev/null || true)

    local msg="mensagem_teste_horst_${teste_num}"
    local output_remet output_dest
    output_remet=$(cd ../HORST && timeout 30 ./remet_horst "$msg" 2>&1)
    local ret_remet=$?
    output_dest=$(cd ../HORST && timeout 30 ./dest_horst 2>&1)
    local ret_dest=$?

    local output="${output_remet}"$'\n'"${output_dest}"

    if [ $ret_remet -ne 0 ] || [ $ret_dest -ne 0 ]; then
        echo -e "${RED}Erro HORST $teste_num${NC}"
        echo "HORST,$teste_num,0,0,0,0,0,0,0,0,32800,32,9152" >> "$RESULTADO_FILE"
        return 1
    fi

    local tempo_sk tempo_pk tempo_sign tempo_verif
    tempo_sk=$(echo "$output"    | grep "Tempo para gerar Chaves Secretas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_pk=$(echo "$output"    | grep "Tempo para gerar Chave Publica:"   | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_sign=$(echo "$output"  | grep "Tempo para Assinar:"               | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_verif=$(echo "$output" | grep "Tempo Verificação:"                | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_sk=${tempo_sk:-"0"}
    tempo_pk=${tempo_pk:-"0"}
    tempo_sign=${tempo_sign:-"0"}
    tempo_verif=${tempo_verif:-"0"}

    local hashes=$(extrair_hashes "$output")
    local h_keygen=$(echo "$hashes" | cut -d',' -f1)
    local h_sign=$(echo   "$hashes" | cut -d',' -f2)
    local h_verif=$(echo  "$hashes" | cut -d',' -f3)

    local tam_sk tam_pk tam_sign
    tam_sk=$(file_size_or_default "../HORST/seckeys.bin" "32800")
    tam_pk=$(file_size_or_default "../HORST/pubkey.bin"  "32")
    tam_sign=$(file_size_or_default "../HORST/assinatura.bin" "9152")

    echo "HORST,$teste_num,$tempo_sk,$tempo_pk,0,$tempo_sign,$tempo_verif,$h_keygen,$h_sign,$h_verif,$tam_sk,$tam_pk,$tam_sign" >> "$RESULTADO_FILE"

    echo -e "${GREEN}  ✓ HORST $teste_num: SK=${tempo_sk}s | PK=${tempo_pk}s | Sign=${tempo_sign}s | Verif=${tempo_verif}s | H=${h_keygen}/${h_sign}/${h_verif}${NC}"
}

# ── MSS ───────────────────────────────────────────────────────
testar_mss() {
    local teste_num=$1
    echo -e "${BLUE}Testando MSS  - $teste_num/${TESTES}${NC}"

    (cd ../MSS && rm -f *.txt *.bin 2>/dev/null || true)
    printf "mensagem_teste_mss_%s\n" "$teste_num" > ../MSS/mensagem.txt

    local output_remet output_dest
    output_remet=$(cd ../MSS && timeout 60 ./remet_mss mensagem.txt public_key.txt assinatura.txt 2>&1)
    local ret_remet=$?
    output_dest=$(cd ../MSS && timeout 30 ./dest_mss mensagem.txt public_key.txt assinatura.txt 2>&1)
    local ret_dest=$?

    local output="${output_remet}"$'\n'"${output_dest}"

    if [ $ret_remet -ne 0 ] || [ $ret_dest -ne 0 ]; then
        echo -e "${RED}Erro MSS $teste_num${NC}"
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

    local hashes=$(extrair_hashes "$output")
    local h_keygen=$(echo "$hashes" | cut -d',' -f1)
    local h_sign=$(echo   "$hashes" | cut -d',' -f2)
    local h_verif=$(echo  "$hashes" | cut -d',' -f3)

    local tam_folhas tam_pubkey tam_sign_file
    tam_folhas=$(file_size_or_default "../MSS/folhas.txt"      "0")
    tam_pubkey=$(file_size_or_default "../MSS/public_key.txt"  "0")
    tam_sign_file=$(file_size_or_default "../MSS/assinatura.txt" "0")

    echo "MSS,$teste_num,$tempo_folhas,$tempo_arvore,0,$tempo_sign,$tempo_verif,$h_keygen,$h_sign,$h_verif,$tam_folhas,$tam_pubkey,$tam_sign_file" >> "$RESULTADO_FILE"

    echo -e "${GREEN}  ✓ MSS  $teste_num: Folhas=${tempo_folhas}s | Árvore=${tempo_arvore}s | Sign=${tempo_sign}s | Verif=${tempo_verif}s | H=${h_keygen}/${h_sign}/${h_verif}${NC}"
}

# ── SPHINCS ───────────────────────────────────────────────────
testar_sphincs() {
    local teste_num=$1
    echo -e "${BLUE}Testando SPHINCS - $teste_num/${TESTES}${NC}"

    (cd ../SPHINCS && rm -f *.bin mensagem.txt 2>/dev/null || true)
    printf "mensagem_teste_sphincs_%s\n" "$teste_num" > ../SPHINCS/mensagem.txt

    local output_remet=$(cd ../SPHINCS && timeout 60 ./remet_sphincs mensagem.txt pubkey.bin sig.bin sk_seed.bin seckey.bin 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro SPHINCS $teste_num (remetente)${NC}"
        echo "SPHINCS,$teste_num,0,0,0,0,0,0,0,0,96,64,36840" >> "$RESULTADO_FILE"
        return 1
    fi

    local output_dest=$(cd ../SPHINCS && timeout 60 ./dest_sphincs mensagem.txt pubkey.bin sig.bin sk_seed.bin 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro SPHINCS $teste_num (destinatário)${NC}"
        echo "SPHINCS,$teste_num,0,0,0,0,0,0,0,0,96,64,36840" >> "$RESULTADO_FILE"
        return 1
    fi

    local output="${output_remet}"$'\n'"${output_dest}"

    local tempo_secret_keys=$(extrair_valor "$output" "Tempo Geração de Chaves:")
    local tempo_public_keys="0"
    local tempo_assinatura=$(extrair_valor "$output" "Tempo Assinatura:")
    local tempo_verificacao=$(extrair_valor "$output_dest" "Tempo Verificação:")

    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    tempo_verificacao=${tempo_verificacao:-"0"}

    local hashes=$(extrair_hashes "$output")
    local h_keygen=$(echo "$hashes" | cut -d',' -f1)
    local h_sign=$(echo   "$hashes" | cut -d',' -f2)
    local h_verif=$(echo  "$hashes" | cut -d',' -f3)

    local tamanho_secret=$(file_size_or_default "../SPHINCS/seckey.bin" "96")
    local tamanho_public=$(file_size_or_default "../SPHINCS/pubkey.bin" "64")
    local tamanho_assinatura=$(file_size_or_default "../SPHINCS/sig.bin" "36840")

    echo "SPHINCS,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$tempo_verificacao,$h_keygen,$h_sign,$h_verif,$tamanho_secret,$tamanho_public,$tamanho_assinatura" >> "$RESULTADO_FILE"

    echo -e "${GREEN}  ✓ SPHINCS $teste_num: Keygen=${tempo_secret_keys}s | Sign=${tempo_assinatura}s | Verif=${tempo_verificacao}s | H=${h_keygen}/${h_sign}/${h_verif}${NC}"
}

# ── Loop principal ────────────────────────────────────────────
echo -e "${YELLOW}Iniciando $TESTES execuções...${NC}"
echo

for i in $(seq 1 $TESTES); do
    echo -e "${YELLOW}=== EXECUÇÃO $i/$TESTES ===${NC}"
    testar_horst $i
    sleep 1
    testar_mss $i
    sleep 1
    testar_sphincs $i
    sleep 1
    echo
done

echo -e "${GREEN}TESTES CONCLUÍDOS!${NC}"
echo -e "${GREEN}Resultados salvos em: ${YELLOW}$RESULTADO_FILE${NC}"

JSON_FILE="${RESULTADO_FILE%.csv}.json"
python3 export_results_to_json.py "$RESULTADO_FILE" "$JSON_FILE"
echo -e "${GREEN}Resultados em JSON: ${YELLOW}$JSON_FILE${NC}"