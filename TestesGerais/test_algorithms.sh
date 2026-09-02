#!/bin/bash

# Script para testar algoritmos de assinatura (LOTS, WOTS+, HORS, SPHINCS)
# e salvar os resultados em tabela CSV

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  TESTE DE ALGORITMOS DE ASSINATURA    ${NC}"
echo -e "${BLUE}  (LOTS, WOTS+, HORS, SPHINCS)         ${NC}"
echo -e "${BLUE}========================================${NC}"

# Número de testes
TESTES=${TESTES:-10}
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULTADO_DIR="resultados_metricas"
mkdir -p "$RESULTADO_DIR"
RESULTADO_FILE="$RESULTADO_DIR/resultados_${TIMESTAMP}.csv"

# Mensagem de teste padrão
MENSAGEM_TESTE="Esta é uma mensagem de teste para avaliar os algoritmos de assinatura digital."

echo -e "${YELLOW}Compilando algoritmos...${NC}"
compile_err=0
(cd ../SHA256 && make > /dev/null 2>&1) || { echo -e "${RED}Erro ao compilar SHA256!${NC}"; compile_err=1; }
(cd ../LOTS   && make clean > /dev/null 2>&1; make > /dev/null 2>&1) || { echo -e "${RED}Erro ao compilar LOTS!${NC}"; compile_err=1; }
(cd ../WOTS   && make clean > /dev/null 2>&1; make > /dev/null 2>&1) || { echo -e "${RED}Erro ao compilar WOTS!${NC}"; compile_err=1; }
(cd ../HORS   && make clean > /dev/null 2>&1; make > /dev/null 2>&1) || { echo -e "${RED}Erro ao compilar HORS!${NC}"; compile_err=1; }
(cd ../SPHINCS && make clean > /dev/null 2>&1; make > /dev/null 2>&1) || { echo -e "${RED}Erro ao compilar SPHINCS!${NC}"; compile_err=1; }

if [ "$compile_err" -ne 0 ]; then
    echo -e "${RED}Erro na compilação!${NC}"
    exit 1
fi

echo -e "${GREEN}Compilação concluída!${NC}"
echo

# Cabeçalho do CSV
HEADER="Algoritmo,Teste,Tempo_SecretKeys,Tempo_PublicKeys,Tempo_Masks,Tempo_Assinatura,Tempo_Verificacao,Hashes_Assinatura,Tamanho_SecretKeys,Tamanho_PublicKeys,Tamanho_Assinatura,Valgrind_Bytes,Valgrind_Erros"
printf "%s\n" "$HEADER" > "$RESULTADO_FILE"

echo -e "${GREEN}✓ Arquivo CSV criado: $RESULTADO_FILE${NC}"
echo -e "${BLUE}Header: $(head -1 "$RESULTADO_FILE")${NC}"
echo

# Funções auxiliares
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

coletar_valgrind() {
    local executavel="$1"
    local dir="$2"
    local input="$3"
    local timeout_val="${4:-60}"
    shift 4 || shift $#
    local extra_args=("$@")

    local vg_log
    vg_log=$(mktemp /tmp/vg_XXXXXX.log)

    if [ -n "$input" ]; then
        (cd "$dir" && echo -e "$input" | timeout "$timeout_val" \
            valgrind --tool=memcheck --leak-check=full --error-exitcode=0 \
            --log-file="$vg_log" \
            $executavel "${extra_args[@]}" > /dev/null 2>&1) || true
    else
        (cd "$dir" && timeout "$timeout_val" \
            valgrind --tool=memcheck --leak-check=full --error-exitcode=0 \
            --log-file="$vg_log" \
            $executavel "${extra_args[@]}" > /dev/null 2>&1) || true
    fi

    local vg_output
    vg_output=$(cat "$vg_log" 2>/dev/null)
    rm -f "$vg_log"

    local bytes_uso
    bytes_uso=$(echo "$vg_output" | grep -i "total heap usage" | grep -oP '[0-9,]+(?= bytes allocated)' | tr -d ',' | head -1)
    bytes_uso=${bytes_uso:-"0"}

    local erros
    erros=$(echo "$vg_output" | grep -i "ERROR SUMMARY" | grep -oP '^[0-9]+' | head -1)
    erros=${erros:-"0"}

    echo "${bytes_uso},${erros}"
}

# Teste Lamport (LOTS)
testar_lamport() {
    local teste_num=$1
    echo -e "${BLUE}Testando Lamport (LOTS) - Teste $teste_num${NC}"
    
    (cd ../LOTS && rm -f *.txt *.bin 2>/dev/null || true)
    printf "%s\n" "$MENSAGEM_TESTE" > ../LOTS/mensagem.txt
    
    local output_remet=$(cd ../LOTS && timeout 30 ./remet_lots mensagem.txt publicKeys.txt assinatura.txt 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste Lamport $teste_num (remetente)${NC}"
        return 1
    fi
    
    local output_dest=$(cd ../LOTS && timeout 30 ./dest_lots mensagem.txt publicKeys.txt assinatura.txt 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste Lamport $teste_num (destinatário)${NC}"
        return 1
    fi

    local tempo_secret_keys=$(extrair_valor "$output_remet" "SecretsKeys:")
    local tempo_public_keys=$(extrair_valor "$output_remet" "PublicKeys:")
    local tempo_assinatura=$(extrair_valor "$output_remet" "Mensagem Assinada em:")
    local tempo_verificacao=$(extrair_valor "$output_dest" "Tempo Verificação:")
    local hashes_assinatura=$(extrair_valor "$output_remet" "Total de hashes SHA256:")
    
    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    tempo_verificacao=${tempo_verificacao:-"0"}
    hashes_assinatura=${hashes_assinatura:-"0"}
    
    local tamanho_secret=$(file_size_or_default "../LOTS/secretKeys.txt" "16384")
    local tamanho_public=$(file_size_or_default "../LOTS/publicKeys.txt" "33280")
    local tamanho_assinatura=$(file_size_or_default "../LOTS/assinatura.txt" "16640")
    
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        vg_metrics=$(coletar_valgrind "./remet_lots" "../LOTS" "" 60 "mensagem.txt" "publicKeys.txt" "assinatura.txt")
    fi
    
    echo "LOTS,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$tempo_verificacao,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura,$vg_metrics" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}Lamport Teste $teste_num: OK${NC}"
    echo "  SK: ${tempo_secret_keys}s | PK: ${tempo_public_keys}s | Assinatura: ${tempo_assinatura}s | Verificação: ${tempo_verificacao}s | Hashes: $hashes_assinatura"
}

# Teste WOTS
testar_wots() {
    local teste_num=$1
    echo -e "${BLUE}Testando WOTS - Teste $teste_num${NC}"
    
    (cd ../WOTS && rm -f *.txt *.bin 2>/dev/null || true)
    printf "%s\n" "$MENSAGEM_TESTE" > ../WOTS/mensagem.txt
    
    local output_remet=$(cd ../WOTS && timeout 30 ./remet_wots mensagem.txt PublicKeys.bin Assinatura.bin 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste WOTS $teste_num (remetente)${NC}"
        return 1
    fi
    
    local output_dest=$(cd ../WOTS && timeout 30 ./dest_wots mensagem.txt PublicKeys.bin Assinatura.bin 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste WOTS $teste_num (destinatário)${NC}"
        return 1
    fi
    
    local tempo_secret_keys=$(extrair_valor "$output_remet" "SecretsKeys:")
    local tempo_public_keys=$(extrair_valor "$output_remet" "PublicKeys:")
    local tempo_masks=$(extrair_valor "$output_remet" "Tempo para gerar Masks:")
    local tempo_assinatura=$(extrair_valor "$output_remet" "Mensagem Assinada em:")
    local tempo_verificacao=$(extrair_valor "$output_dest" "Tempo Verificação:")
    local hashes_assinatura=$(extrair_valor "$output_remet" "Total de hashes SHA256:")
    
    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_masks=${tempo_masks:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    tempo_verificacao=${tempo_verificacao:-"0"}
    hashes_assinatura=${hashes_assinatura:-"0"}
    
    local tamanho_secret=2144
    local tamanho_public=$(file_size_or_default "../WOTS/PublicKeys.bin" "2208")
    local tamanho_assinatura=$(file_size_or_default "../WOTS/Assinatura.bin" "2144")
    
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        vg_metrics=$(coletar_valgrind "./remet_wots" "../WOTS" "" 60 "mensagem.txt" "PublicKeys.bin" "Assinatura.bin")
    fi
    
    echo "WOTS,$teste_num,$tempo_secret_keys,$tempo_public_keys,$tempo_masks,$tempo_assinatura,$tempo_verificacao,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura,$vg_metrics" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}WOTS Teste $teste_num: OK${NC}"
    echo "  SK: ${tempo_secret_keys}s | PK: ${tempo_public_keys}s | Masks: ${tempo_masks}s | Assinatura: ${tempo_assinatura}s | Verificação: ${tempo_verificacao}s"
}

# Teste HORS
testar_hors() {
    local teste_num=$1
    echo -e "${BLUE}Testando HORS - Teste $teste_num${NC}"
    
    (cd ../HORS && rm -f *.txt *.bin 2>/dev/null || true)
    
    local output_assinatura=$(cd ../HORS && ./hors_test "$MENSAGEM_TESTE" 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste HORS $teste_num${NC}"
        return 1
    fi
    
    local tempo_secret_keys=$(echo "$output_assinatura" | grep "Tempo para gerar Chaves Secretas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_public_keys=$(echo "$output_assinatura" | grep "Tempo para gerar Chaves Publicas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_assinatura=$(echo "$output_assinatura" | grep "Tempo para Assinar:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_verificacao=$(extrair_valor "$output_assinatura" "Tempo Verificação:")
    local hashes_assinatura=$(echo "$output_assinatura" | grep "Total de hashes SHA256:" | awk -F': ' '{print $2}')
    
    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    tempo_verificacao=${tempo_verificacao:-"0"}
    hashes_assinatura=${hashes_assinatura:-"0"}
    
    local tamanho_secret=$(file_size_or_default "../HORS/secretKeys.txt" "32768")
    local tamanho_public=$(file_size_or_default "../HORS/publicKeys.txt" "32768")
    local tamanho_assinatura=$(file_size_or_default "../HORS/assinatura.txt" "832")
    
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        vg_metrics=$(coletar_valgrind "./hors_test" "../HORS" "" 60 "$MENSAGEM_TESTE")
    fi
    
    echo "HORS,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$tempo_verificacao,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura,$vg_metrics" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}HORS Teste $teste_num: OK${NC}"
    echo "  SK: ${tempo_secret_keys}s | PK: ${tempo_public_keys}s | Assinatura: ${tempo_assinatura}s | Verificação: ${tempo_verificacao}s | Hashes: $hashes_assinatura"
}

# Teste SPHINCS
testar_sphincs() {
    local teste_num=$1
    echo -e "${BLUE}Testando SPHINCS-256 - Teste $teste_num${NC}"
    
    (cd ../SPHINCS && rm -f *.bin mensagem.txt 2>/dev/null || true)
    printf "%s\n" "$MENSAGEM_TESTE" > ../SPHINCS/mensagem.txt
    
    local output_remet=$(cd ../SPHINCS && timeout 60 ./remet_sphincs mensagem.txt pubkey.bin sig.bin sk_seed.bin seckey.bin 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste SPHINCS $teste_num (remetente)${NC}"
        return 1
    fi
    
    local output_dest=$(cd ../SPHINCS && timeout 60 ./dest_sphincs mensagem.txt pubkey.bin sig.bin sk_seed.bin 2>&1)
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste SPHINCS $teste_num (destinatário)${NC}"
        return 1
    fi
    
    local tempo_secret_keys=$(extrair_valor "$output_remet" "Tempo Geração de Chaves:")
    local tempo_public_keys="0"
    local tempo_assinatura=$(extrair_valor "$output_remet" "Tempo Assinatura:")
    local tempo_verificacao=$(extrair_valor "$output_dest" "Tempo Verificação:")
    local hashes_assinatura=$(extrair_valor "$output_remet" "Total de hashes SHA256")
    
    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    tempo_verificacao=${tempo_verificacao:-"0"}
    hashes_assinatura=${hashes_assinatura:-"0"}
    
    local tamanho_secret=$(file_size_or_default "../SPHINCS/seckey.bin" "96")
    local tamanho_public=$(file_size_or_default "../SPHINCS/pubkey.bin" "64")
    local tamanho_assinatura=$(file_size_or_default "../SPHINCS/sig.bin" "36840")
    
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        vg_metrics=$(coletar_valgrind "./remet_sphincs" "../SPHINCS" "" 120 "mensagem.txt" "pubkey.bin" "sig.bin" "sk_seed.bin" "seckey.bin")
    fi
    
    echo "SPHINCS,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$tempo_verificacao,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura,$vg_metrics" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}SPHINCS Teste $teste_num: OK${NC}"
    echo "  Keygen: ${tempo_secret_keys}s | Sign: ${tempo_assinatura}s | Verificação: ${tempo_verificacao}s | Hashes: $hashes_assinatura"
}

echo -e "${YELLOW}Iniciando testes ($TESTES execuções para cada algoritmo)...${NC}"
echo

for i in $(seq 1 $TESTES); do
    echo -e "${YELLOW}=== EXECUÇÃO $i/$TESTES ===${NC}"
    
    testar_lamport $i
    sleep 1
    
    testar_wots $i
    sleep 1
    
    testar_hors $i
    sleep 1

    testar_sphincs $i
    sleep 1
    
    echo
done

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  TESTES CONCLUÍDOS!                   ${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Resultados salvos em: $RESULTADO_FILE${NC}"

# Exporta automaticamente para JSON
JSON_FILE="${RESULTADO_FILE%.csv}.json"
python3 export_results_to_json.py "$RESULTADO_FILE" "$JSON_FILE"
echo -e "${GREEN}Resultados em JSON:   ${YELLOW}$JSON_FILE${NC}"
echo

(cd .. && make clean > /dev/null 2>&1)
echo -e "${GREEN}Limpeza concluída!${NC}"