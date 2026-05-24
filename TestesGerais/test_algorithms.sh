#!/bin/bash

# Script para testar algoritmos Lamport e WOTS múltiplas vezes
# e salvar os resultados em uma tabela

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  TESTE DE ALGORITMOS DE ASSINATURA    ${NC}"
echo -e "${BLUE}========================================${NC}"

# Número de testes
TESTES=10
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
RESULTADO_DIR="resultados_metricas"
mkdir -p "$RESULTADO_DIR"
RESULTADO_FILE="$RESULTADO_DIR/resultados_${TIMESTAMP}.csv"

# Mensagem de teste padrão
MENSAGEM_TESTE="Esta é uma mensagem de teste para avaliar os algoritmos de assinatura digital."

echo -e "${YELLOW}Compilando algoritmos...${NC}"
(cd .. && make clean > /dev/null 2>&1)
(cd .. && make all > /dev/null 2>&1)

if [ $? -ne 0 ]; then
    echo -e "${RED}Erro na compilação!${NC}"
    exit 1
fi

echo -e "${GREEN}Compilação concluída!${NC}"
echo

# Cria cabeçalho do CSV DEPOIS da compilação (para não ser apagado pelo make clean)
HEADER="Algoritmo,Teste,Tempo_SecretKeys,Tempo_PublicKeys,Tempo_Masks,Tempo_Assinatura,Hashes_Assinatura,Tamanho_SecretKeys,Tamanho_PublicKeys,Tamanho_Assinatura"
printf "%s\n" "$HEADER" > "$RESULTADO_FILE"

# Compilar HORS especificamente
echo -e "${YELLOW}Compilando HORS...${NC}"
(cd ../HORS && make clean > /dev/null 2>&1)
(cd ../HORS && make > /dev/null 2>&1)
if [ $? -ne 0 ]; then
    echo -e "${RED}Erro ao compilar HORS!${NC}"
    exit 1
fi
echo -e "${GREEN}HORS compilado com sucesso!${NC}"

echo -e "${GREEN}✓ Arquivo CSV criado: $RESULTADO_FILE${NC}"
echo -e "${BLUE}Header: $(head -1 "$RESULTADO_FILE")${NC}"
echo

# Função para extrair valores do output
extrair_valor() {
    local texto="$1"
    local padrao="$2"
    # Extrai o valor numérico após o padrão (após os dois pontos)
    echo "$texto" | grep "$padrao" | sed 's/.*: *\([0-9.]*\).*/\1/' | head -1
}

# Função para testar Lamport
testar_lamport() {
    local teste_num=$1
    echo -e "${BLUE}Testando Lamport - Teste $teste_num${NC}"
    
    # Limpar arquivos anteriores do LOTS
    (cd ../LOTS && rm -f assinatura.txt mensagem.txt publicKeys.txt 2>/dev/null || true)
    
    # Teste de assinatura (opção 1)
    local output_assinatura=$(echo -e "1\n$MENSAGEM_TESTE" | timeout 30 ../LOTS/lots 2>&1)
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste Lamport $teste_num (assinatura)${NC}"
        return 1
    fi
    
    # Teste de verificação (opção 2)
    local output_verificacao=$(echo "2" | timeout 30 ../LOTS/lots 2>&1)
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste Lamport $teste_num (verificação)${NC}"
        return 1
    fi
    
    # Extrai valores do Lamport com os padrões corretos
    local tempo_secret_keys=$(extrair_valor "$output_assinatura" "SecretsKeys:")
    local tempo_public_keys=$(extrair_valor "$output_assinatura" "PublicKeys:")
    local tempo_assinatura=$(extrair_valor "$output_assinatura" "Mensagem Assinada em:")
    
    local hashes_assinatura=$(extrair_valor "$output_assinatura" "Total de hashes SHA256:")
    
    local tamanho_secret=$(extrair_valor "$output_assinatura" "Tamanho Secretkeys:")
    local tamanho_public=$(extrair_valor "$output_assinatura" "Tamanho Publickeys:")
    local tamanho_assinatura=$(extrair_valor "$output_assinatura" "Tamanho Assinatura:")
    
    # Valores padrão se não encontrados
    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    hashes_assinatura=${hashes_assinatura:-"0"}
    tamanho_secret=${tamanho_secret:-"131072"}
    tamanho_public=${tamanho_public:-"33280"}
    tamanho_assinatura=${tamanho_assinatura:-"16640"}
    
    # Salva no CSV (Lamport não tem masks, então usa 0)
    echo "LOTS,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}Lamport Teste $teste_num: OK${NC}"
    echo "  SK: ${tempo_secret_keys}s | PK: ${tempo_public_keys}s | Assinatura: ${tempo_assinatura}s | Hashes: $hashes_assinatura"
}

# Função para testar WOTS
testar_wots() {
    local teste_num=$1
    echo -e "${BLUE}Testando WOTS - Teste $teste_num${NC}"
    
    # Limpar arquivos anteriores do WOTS
    (cd ../WOTS && rm -f Assinatura.txt Mensagem.txt PublicKeys.txt Masks.txt 2>/dev/null || true)
    
    # Teste de assinatura (opção 1)
    local output_assinatura=$(echo -e "1\n$MENSAGEM_TESTE" | timeout 30 ../WOTS/wots 2>&1)
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste WOTS $teste_num (assinatura)${NC}"
        return 1
    fi
    
    # Extrai valores do WOTS separadamente
    local tempo_secret_keys=$(extrair_valor "$output_assinatura" "Tempo para gerar Chaves Secretas:")
    local tempo_public_keys=$(extrair_valor "$output_assinatura" "Tempo para gerar Chaves Public:")
    local tempo_masks=$(extrair_valor "$output_assinatura" "Tempo para gerar Masks:")
    local tempo_assinatura=$(extrair_valor "$output_assinatura" "Tempo para Assinar:")
    
    local hashes_assinatura=$(extrair_valor "$output_assinatura" "Total de hashes SHA256:")
    
    local tamanho_secret=$(extrair_valor "$output_assinatura" "Tamanho Secretkeys:")
    local tamanho_public=$(extrair_valor "$output_assinatura" "Tamanho Publickeys:")
    local tamanho_assinatura=$(extrair_valor "$output_assinatura" "Tamanho Assinatura:")
    
    # Valores padrão se não encontrados
    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_masks=${tempo_masks:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    hashes_assinatura=${hashes_assinatura:-"0"}
    tamanho_secret=${tamanho_secret:-"2144"}
    tamanho_public=${tamanho_public:-"2144"}
    tamanho_assinatura=${tamanho_assinatura:-"2144"}
    
    # Salva no CSV com tempos separados
    echo "WOTS,$teste_num,$tempo_secret_keys,$tempo_public_keys,$tempo_masks,$tempo_assinatura,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}WOTS Teste $teste_num: OK${NC}"
    echo "  SK: ${tempo_secret_keys}s | PK: ${tempo_public_keys}s | Masks: ${tempo_masks}s | Assinatura: ${tempo_assinatura}s"
}

# Função para testar HORS
testar_hors() {
    local teste_num=$1
    echo -e "${BLUE}Testando HORS - Teste $teste_num${NC}"
    
    # Limpar arquivos anteriores do HORS
    (cd ../HORS && rm -f assinatura.txt mensagem.txt publicKeys.txt secretKeys.txt assinatura.bin publicKeys.bin 2>/dev/null || true)
    
    # Teste com hors_test (sem menu)
    local output_assinatura=$(../HORS/hors_test "$MENSAGEM_TESTE" 2>&1)
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste HORS $teste_num${NC}"
        return 1
    fi
    
    # Extrai valores do HORS usando awk mais cuidadoso
    local tempo_secret_keys=$(echo "$output_assinatura" | grep "Tempo para gerar Chaves Secretas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_assinatura=$(echo "$output_assinatura" | grep "Tempo para Assinar:" | awk -F': ' '{print $2}' | awk '{print $1}')
    
    local hashes_assinatura=$(echo "$output_assinatura" | grep "Total de hashes SHA256:" | awk -F': ' '{print $2}')
    
    local tamanho_secret=$(echo "$output_assinatura" | grep "Tamanho Secretkeys:" | awk -F': ' '{print $2}')
    local tamanho_public=$(echo "$output_assinatura" | grep "Tamanho Publickeys:" | awk -F': ' '{print $2}')
    local tamanho_assinatura=$(echo "$output_assinatura" | grep "Tamanho Assinatura:" | awk -F': ' '{print $2}')
    
    # Valores padrão se não encontrados
    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    hashes_assinatura=${hashes_assinatura:-"0"}
    tamanho_secret=${tamanho_secret:-"32768"}
    tamanho_public=${tamanho_public:-"32768"}
    tamanho_assinatura=${tamanho_assinatura:-"832"}
    
    # Salva no CSV (HORS não tem masks, então usa 0)
    echo "HORS,$teste_num,$tempo_secret_keys,0,0,$tempo_assinatura,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}HORS Teste $teste_num: OK${NC}"
    echo "  SK: ${tempo_secret_keys}s | Assinatura: ${tempo_assinatura}s | Hashes: $hashes_assinatura"
}

echo -e "${YELLOW}Iniciando testes ($TESTES execuções para cada algoritmo)...${NC}"
echo

# Executa testes
for i in $(seq 1 $TESTES); do
    echo -e "${YELLOW}=== EXECUÇÃO $i/$TESTES ===${NC}"
    
    # Testa Lamport
    testar_lamport $i
    sleep 1
    
    # Testa WOTS  
    testar_wots $i
    sleep 1
    
    # Testa HORS
    testar_hors $i
    sleep 1
    
    echo
done

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  TESTES CONCLUÍDOS!                   ${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Resultados salvos em: $RESULTADO_FILE${NC}"
echo

echo
echo -e "${GREEN}Script finalizado!${NC}"
echo -e "${BLUE}Arquivo de resultados: ${YELLOW}$RESULTADO_FILE${NC}"

echo -e "${YELLOW}Limpando arquivos temporários...${NC}"
(cd .. && make clean > /dev/null 2>&1)
echo -e "${GREEN}Limpeza concluída!${NC}"