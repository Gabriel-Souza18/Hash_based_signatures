#!/bin/bash

# Script para testar algoritmos de assinatura (LOTS, WOTS+, HORS)
# e salvar os resultados em tabela CSV
#
# NOVO: três colunas de hashes — Keygen, Assinatura e Verificação

RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m'

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  TESTE DE ALGORITMOS DE ASSINATURA    ${NC}"
echo -e "${BLUE}  (LOTS, WOTS+, HORS)                  ${NC}"
echo -e "${BLUE}========================================${NC}"

TESTES=${TESTES:-10}
USAR_VALGRIND=${USAR_VALGRIND:-0}
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULTADO_DIR="resultados_metricas"
mkdir -p "$RESULTADO_DIR"
RESULTADO_FILE="$RESULTADO_DIR/resultados_OT${TIMESTAMP}.csv"

MENSAGEM_TESTE="Esta é uma mensagem de teste para avaliar os algoritmos de assinatura digital."

# ── Compilação ───────────────────────────────────────────────
echo -e "${YELLOW}Compilando algoritmos...${NC}"
compile_err=0
(cd ../SHA256 && make > /dev/null 2>&1) || { echo -e "${RED}Erro SHA256${NC}"; compile_err=1; }
(cd ../LOTS   && make clean > /dev/null 2>&1; make > /dev/null 2>&1) || { echo -e "${RED}Erro LOTS${NC}"; compile_err=1; }
(cd ../WOTS   && make clean > /dev/null 2>&1; make > /dev/null 2>&1) || { echo -e "${RED}Erro WOTS${NC}"; compile_err=1; }
(cd ../HORS   && make clean > /dev/null 2>&1; make > /dev/null 2>&1) || { echo -e "${RED}Erro HORS${NC}"; compile_err=1; }

[ "$compile_err" -ne 0 ] && { echo -e "${RED}Erro na compilação!${NC}"; exit 1; }
echo -e "${GREEN}Compilação concluída!${NC}"
echo

# ── Cabeçalho do CSV ─────────────────────────────────────────
HEADER="Algoritmo,Teste,Tempo_SecretKeys,Tempo_PublicKeys,Tempo_Masks,Tempo_Assinatura,Tempo_Verificacao,Hashes_Keygen,Hashes_Assinatura,Hashes_Verificacao,Tamanho_SecretKeys,Tamanho_PublicKeys,Tamanho_Assinatura"
printf "%s\n" "$HEADER" > "$RESULTADO_FILE"

echo -e "${GREEN}✓ Arquivo CSV criado: $RESULTADO_FILE${NC}"
echo -e "${BLUE}Header: $(head -1 "$RESULTADO_FILE")${NC}"
echo

# ── Funções auxiliares ───────────────────────────────────────
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

# Extrai 3 valores de hash do output.
# Aceita os formatos novos:
#   "Hashes Keygen: N"
#   "Hashes Assinatura: N"
#   "Hashes Verificacao: N"
# Se não encontrar, tenta formatos antigos como fallback.
extrair_hashes() {
    local texto="$1"
    local h_keygen h_sign h_verif

    h_keygen=$(echo "$texto" | grep -E "Hashes Keygen:" | awk -F': ' '{print $2}' | awk '{print $1}' | head -1)
    h_sign=$(echo   "$texto" | grep -E "Hashes Assinatura:" | awk -F': ' '{print $2}' | awk '{print $1}' | head -1)
    h_verif=$(echo  "$texto" | grep -E "Hashes Verificacao:" | awk -F': ' '{print $2}' | awk '{print $1}' | head -1)

    # Fallbacks para formatos antigos
    if [ -z "$h_sign" ]; then
        h_sign=$(echo "$texto" | grep -E "Total de hashes SHA256 \(assinatura\):|Total de hashes SHA256:" | awk -F': ' '{print $2}' | awk '{print $1}' | head -1)
    fi

    echo "${h_keygen:-0},${h_sign:-0},${h_verif:-0}"
}

# ── Teste LOTS ───────────────────────────────────────────────
testar_lamport() {
    local teste_num=$1
    echo -e "${BLUE}Testando Lamport (LOTS) - Teste $teste_num${NC}"

    (cd ../LOTS && rm -f *.txt *.bin 2>/dev/null || true)
    printf "%s\n" "$MENSAGEM_TESTE" > ../LOTS/mensagem.txt

    local output_remet=$(cd ../LOTS && timeout 30 ./remet_lots mensagem.txt publicKeys.txt assinatura.txt 2>&1)
    [ $? -ne 0 ] && { echo -e "${RED}Erro LOTS (remetente)${NC}"; return 1; }

    local output_dest=$(cd ../LOTS && timeout 30 ./dest_lots mensagem.txt publicKeys.txt assinatura.txt 2>&1)
    [ $? -ne 0 ] && { echo -e "${RED}Erro LOTS (destinatário)${NC}"; return 1; }

    local tempo_secret_keys=$(extrair_valor "$output_remet" "SecretsKeys:")
    local tempo_public_keys=$(extrair_valor "$output_remet" "PublicKeys:")
    local tempo_assinatura=$(extrair_valor "$output_remet" "Mensagem Assinada em:")
    local tempo_verificacao=$(extrair_valor "$output_dest" "Tempo Verificação:")

    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    tempo_verificacao=${tempo_verificacao:-"0"}

    # 3 hashes: keygen, assinatura, verificação
    local hashes=$(extrair_hashes "$output_remet"$'\n'"$output_dest")
    local h_keygen=$(echo "$hashes" | cut -d',' -f1)
    local h_sign=$(echo   "$hashes" | cut -d',' -f2)
    local h_verif=$(echo  "$hashes" | cut -d',' -f3)

    local tamanho_secret=$(file_size_or_default "../LOTS/secretKeys.txt" "16384")
    local tamanho_public=$(file_size_or_default "../LOTS/publicKeys.txt" "33280")
    local tamanho_assinatura=$(file_size_or_default "../LOTS/assinatura.txt" "16640")

    echo "LOTS,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$tempo_verificacao,$h_keygen,$h_sign,$h_verif,$tamanho_secret,$tamanho_public,$tamanho_assinatura" >> "$RESULTADO_FILE"

    echo -e "${GREEN}LOTS $teste_num: SK=${tempo_secret_keys}s | PK=${tempo_public_keys}s | Sign=${tempo_assinatura}s | Verif=${tempo_verificacao}s | Hashes (kg/sg/vf)=${h_keygen}/${h_sign}/${h_verif}${NC}"
}

# ── Teste WOTS+ ──────────────────────────────────────────────
testar_wots() {
    local teste_num=$1
    echo -e "${BLUE}Testando WOTS+ - Teste $teste_num${NC}"

    (cd ../WOTS && rm -f *.txt *.bin 2>/dev/null || true)
    printf "%s\n" "$MENSAGEM_TESTE" > ../WOTS/mensagem.txt

    local output_remet=$(cd ../WOTS && timeout 30 ./remet_wots mensagem.txt PublicKeys.bin Assinatura.bin 2>&1)
    [ $? -ne 0 ] && { echo -e "${RED}Erro WOTS (remetente)${NC}"; return 1; }

    local output_dest=$(cd ../WOTS && timeout 30 ./dest_wots mensagem.txt PublicKeys.bin Assinatura.bin 2>&1)
    [ $? -ne 0 ] && { echo -e "${RED}Erro WOTS (destinatário)${NC}"; return 1; }

    local tempo_secret_keys=$(extrair_valor "$output_remet" "SecretsKeys:")
    local tempo_public_keys=$(extrair_valor "$output_remet" "PublicKeys:")
    local tempo_masks=$(extrair_valor "$output_remet" "Tempo para gerar Masks:")
    local tempo_assinatura=$(extrair_valor "$output_remet" "Mensagem Assinada em:")
    local tempo_verificacao=$(extrair_valor "$output_dest" "Tempo Verificação:")

    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_masks=${tempo_masks:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    tempo_verificacao=${tempo_verificacao:-"0"}

    local hashes=$(extrair_hashes "$output_remet"$'\n'"$output_dest")
    local h_keygen=$(echo "$hashes" | cut -d',' -f1)
    local h_sign=$(echo   "$hashes" | cut -d',' -f2)
    local h_verif=$(echo  "$hashes" | cut -d',' -f3)

    local tamanho_secret=2144
    local tamanho_public=$(file_size_or_default "../WOTS/PublicKeys.bin" "2208")
    local tamanho_assinatura=$(file_size_or_default "../WOTS/Assinatura.bin" "2144")

    echo "WOTS,$teste_num,$tempo_secret_keys,$tempo_public_keys,$tempo_masks,$tempo_assinatura,$tempo_verificacao,$h_keygen,$h_sign,$h_verif,$tamanho_secret,$tamanho_public,$tamanho_assinatura" >> "$RESULTADO_FILE"

    echo -e "${GREEN}WOTS+ $teste_num: SK=${tempo_secret_keys}s | PK=${tempo_public_keys}s | Masks=${tempo_masks}s | Sign=${tempo_assinatura}s | Verif=${tempo_verificacao}s | Hashes=${h_keygen}/${h_sign}/${h_verif}${NC}"
}

# ── Teste HORS ───────────────────────────────────────────────
testar_hors() {
    local teste_num=$1
    echo -e "${BLUE}Testando HORS - Teste $teste_num${NC}"

    (cd ../HORS && rm -f *.txt *.bin 2>/dev/null || true)

    local output_remet=$(cd ../HORS && timeout 30 ./remet_hors "$MENSAGEM_TESTE" 2>&1)
    [ $? -ne 0 ] && { echo -e "${RED}Erro HORS (remetente)${NC}"; return 1; }

    local output_dest=$(cd ../HORS && timeout 30 ./dest_hors 2>&1)
    [ $? -ne 0 ] && { echo -e "${RED}Erro HORS (destinatário)${NC}"; return 1; }

    local tempo_secret_keys=$(extrair_valor "$output_remet" "Tempo para gerar Chaves Secretas:")
    local tempo_public_keys=$(extrair_valor "$output_remet" "Tempo para gerar Chaves Publicas:")
    local tempo_assinatura=$(extrair_valor "$output_remet" "Tempo para Assinar:")
    local tempo_verificacao=$(extrair_valor "$output_dest" "Tempo Verificação:")

    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    tempo_verificacao=${tempo_verificacao:-"0"}

    local hashes=$(extrair_hashes "$output_remet"$'\n'"$output_dest")
    local h_keygen=$(echo "$hashes" | cut -d',' -f1)
    local h_sign=$(echo   "$hashes" | cut -d',' -f2)
    local h_verif=$(echo  "$hashes" | cut -d',' -f3)

    local tamanho_secret=$(file_size_or_default "../HORS/secretKeys.bin" "32768")
    local tamanho_public=$(file_size_or_default "../HORS/publicKeys.bin" "32768")
    local tamanho_assinatura=$(file_size_or_default "../HORS/assinatura.bin" "832")

    echo "HORS,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$tempo_verificacao,$h_keygen,$h_sign,$h_verif,$tamanho_secret,$tamanho_public,$tamanho_assinatura" >> "$RESULTADO_FILE"

    echo -e "${GREEN}HORS $teste_num: SK=${tempo_secret_keys}s | PK=${tempo_public_keys}s | Sign=${tempo_assinatura}s | Verif=${tempo_verificacao}s | Hashes=${h_keygen}/${h_sign}/${h_verif}${NC}"
}

# ── Loop principal ───────────────────────────────────────────
TESTES_SEM_ARVORE=${TESTES_SEM_ARVORE:-100}

echo -e "${YELLOW}Iniciando testes (LOTS, WOTS, HORS — $TESTES_SEM_ARVORE execuções)...${NC}"
for i in $(seq 1 $TESTES_SEM_ARVORE); do
    echo -e "${YELLOW}=== EXECUÇÃO $i/$TESTES_SEM_ARVORE ===${NC}"
    testar_lamport $i
    testar_wots $i
    testar_hors $i
done

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  TESTES CONCLUÍDOS!                   ${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Resultados salvos em: $RESULTADO_FILE${NC}"

JSON_FILE="${RESULTADO_FILE%.csv}.json"
python3 export_results_to_json.py "$RESULTADO_FILE" "$JSON_FILE"
echo -e "${GREEN}Resultados em JSON: ${YELLOW}$JSON_FILE${NC}"

(cd .. && make clean > /dev/null 2>&1)
echo -e "${GREEN}Limpeza concluída!${NC}"