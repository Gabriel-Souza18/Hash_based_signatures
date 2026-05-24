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
(cd .. && make clean > /dev/null 2>&1 && make all > /dev/null 2>&1)
if [ $? -ne 0 ]; then
    echo -e "${RED}Erro ao compilar MSS!${NC}"
    exit 1
fi
echo -e "${GREEN}MSS compilado com sucesso!${NC}"

# ── Cabeçalho CSV ─────────────────────────────────────────────────────────────
# Colunas do MSS:  Tempo_SecretKeys = tempo das folhas
#                  Tempo_PublicKeys = tempo da árvore Merkle
#                  Tempo_Masks      = 0 (não aplicável)
#                  Tempo_Assinatura = tempo de assinar
HEADER="Algoritmo,Teste,Tempo_SecretKeys,Tempo_PublicKeys,Tempo_Masks,Tempo_Assinatura,Hashes_Assinatura,Tamanho_SecretKeys,Tamanho_PublicKeys,Tamanho_Assinatura,Valgrind_Bytes,Valgrind_Erros"
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

# Coleta métricas de memória com valgrind (memcheck)
# Retorna "bytes_em_uso,erros_totais"
coletar_valgrind() {
    local executavel="$1"
    local dir="$2"
    local input="$3"
    local timeout_val="${4:-120}"

    local vg_output
    vg_output=$(cd "$dir" && echo -e "$input" | timeout "$timeout_val" \
        valgrind --tool=memcheck --leak-check=full --error-exitcode=0 \
        $executavel 2>&1 | tail -20)

    local bytes_uso
    bytes_uso=$(echo "$vg_output" | grep -i "in use at exit" | grep -oP '[0-9,]+(?= bytes)' | tr -d ',' | head -1)
    bytes_uso=${bytes_uso:-"0"}

    local erros
    erros=$(echo "$vg_output" | grep -i "ERROR SUMMARY" | grep -oP '[0-9]+(?= errors)' | head -1)
    erros=${erros:-"0"}

    echo "${bytes_uso},${erros}"
}

# ── Função: testar HORST ──────────────────────────────────────────────────────
testar_horst() {
    local teste_num=$1
    echo -e "${BLUE}Testando HORST - Teste $teste_num/${TESTES}${NC}"

    # Limpar arquivos anteriores
    (cd ../HORST && rm -f *.bin mensagem.txt 2>/dev/null || true)

    # Gerar chaves (1), Assinar (2), Verificar (3), Sair (0)
    local output
    output=$(cd ../HORST && echo -e "1\n2\nmensagem_teste_horst_${teste_num}\n3\n0" \
             | timeout 30 ./testeHORST 2>&1)

    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste HORST $teste_num${NC}"
        echo "HORST,$teste_num,0,0,0,0,0,32800,32,9152,0,0" >> "$RESULTADO_FILE"
        return 1
    fi

    # Tempos
    local tempo_sk tempo_pk tempo_sign
    tempo_sk=$(echo "$output"   | grep "Tempo para gerar Chaves Secretas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_pk=$(echo "$output"   | grep "Tempo para gerar Chave Publica:"   | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_sign=$(echo "$output" | grep "Tempo para Assinar:"               | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_sk=${tempo_sk:-"0"}
    tempo_pk=${tempo_pk:-"0"}
    tempo_sign=${tempo_sign:-"0"}

    # Hashes (fase assinatura — após reset feito em gerarChaves)
    local hashes_sign
    hashes_sign=$(echo "$output" | grep "Total de hashes SHA256 (assinatura)" | awk -F': ' '{print $2}')
    hashes_sign=${hashes_sign:-"0"}

    # Tamanhos
    local tam_sk tam_pk tam_sign
    tam_sk=$(file_size_or_default "../HORST/seckeys.bin" "32800")
    tam_pk=$(file_size_or_default "../HORST/pubkey.bin"  "32")
    tam_sign=$(file_size_or_default "../HORST/assinatura.bin" "9152")

    # Valgrind apenas na 1ª execução
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        echo -e "${YELLOW}  → Coletando métricas Valgrind (HORST)...${NC}"
        vg_metrics=$(coletar_valgrind "./testeHORST" "../HORST" "1\n2\nmensagem_valgrind\n3\n0" 60)
    fi

    echo "HORST,$teste_num,$tempo_sk,$tempo_pk,0,$tempo_sign,$hashes_sign,$tam_sk,$tam_pk,$tam_sign,$vg_metrics" >> "$RESULTADO_FILE"

    echo -e "${GREEN}  ✓ HORST $teste_num: SK=${tempo_sk}s | PK=${tempo_pk}s | Sign=${tempo_sign}s | Hashes=${hashes_sign}${NC}"
}

# ── Função: testar MSS ────────────────────────────────────────────────────────
testar_mss() {
    local teste_num=$1
    echo -e "${BLUE}Testando MSS  - Teste $teste_num/${TESTES}${NC}"

    # Limpar arquivos anteriores
    (cd ../MSS && rm -f *.txt *.bin 2>/dev/null || true)

    # Gerar árvore (1), Assinar (2), Verificar (3), Sair (0)
    local output
    output=$(cd ../MSS && echo -e "1\n2\nmensagem_teste_mss_${teste_num}\n3\n0" \
             | timeout 120 ./mss 2>&1)

    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste MSS $teste_num${NC}"
        echo "MSS,$teste_num,0,0,0,0,0,0,0,0,0,0" >> "$RESULTADO_FILE"
        return 1
    fi

    # Tempos
    local tempo_folhas tempo_arvore tempo_sign
    tempo_folhas=$(echo "$output" | grep "Tempo para gerar Folhas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_arvore=$(echo "$output" | grep "Tempo para gerar Árvore:" | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_sign=$(echo "$output"   | grep "Tempo para Assinar:"      | awk -F': ' '{print $2}' | awk '{print $1}')
    tempo_folhas=${tempo_folhas:-"0"}
    tempo_arvore=${tempo_arvore:-"0"}
    tempo_sign=${tempo_sign:-"0"}

    # Tamanhos de arquivo
    local tam_folhas tam_pubkey tam_sign_file
    tam_folhas=$(file_size_or_default "../MSS/folhas.txt"      "0")
    tam_pubkey=$(file_size_or_default  "../MSS/public_key.txt" "0")
    tam_sign_file=$(file_size_or_default "../MSS/assinatura.txt" "0")

    # Valgrind apenas na 1ª execução (timeout 300s para comportar geração de 1024 folhas)
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        echo -e "${YELLOW}  → Coletando métricas Valgrind (MSS — pode demorar ~60s)...${NC}"
        vg_metrics=$(coletar_valgrind "./mss" "../MSS" "1\n2\nmensagem_valgrind\n3\n0" 300)
    fi

    # Colunas: Tempo_SecretKeys=folhas, Tempo_PublicKeys=árvore, Tempo_Masks=0, Hashes=0 (não instrumentado)
    echo "MSS,$teste_num,$tempo_folhas,$tempo_arvore,0,$tempo_sign,0,$tam_folhas,$tam_pubkey,$tam_sign_file,$vg_metrics" >> "$RESULTADO_FILE"

    echo -e "${GREEN}  ✓ MSS  $teste_num: Folhas=${tempo_folhas}s | Árvore=${tempo_arvore}s | Sign=${tempo_sign}s${NC}"
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
echo

# Limpeza opcional (comentar se quiser manter os binários)
# (cd .. && make clean > /dev/null 2>&1)
echo -e "${GREEN}Script finalizado!${NC}"
