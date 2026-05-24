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
TESTES=100
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
HEADER="Algoritmo,Teste,Tempo_SecretKeys,Tempo_PublicKeys,Tempo_Masks,Tempo_Assinatura,Hashes_Assinatura,Tamanho_SecretKeys,Tamanho_PublicKeys,Tamanho_Assinatura,Valgrind_Bytes,Valgrind_Erros"
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

file_size_or_default() {
    local caminho="$1"
    local padrao="$2"
    if [ -f "$caminho" ]; then
        stat -c%s "$caminho"
    else
        echo "$padrao"
    fi
}

# Função para coletar métricas de memória com valgrind
# Retorna "bytes_em_uso,erros_totais"
coletar_valgrind() {
    local executavel="$1"
    local dir="$2"
    local input="$3"

    # Executa com valgrind --tool=memcheck
    local vg_output
    vg_output=$(cd "$dir" && echo -e "$input" | timeout 60 \
        valgrind --tool=memcheck --leak-check=full --error-exitcode=0 \
        $executavel 2>&1 | tail -20)

    # Extrai bytes em uso no exit
    local bytes_uso
    bytes_uso=$(echo "$vg_output" | grep -i "in use at exit" | grep -oP '[0-9,]+(?= bytes)' | tr -d ',' | head -1)
    bytes_uso=${bytes_uso:-"0"}

    # Extrai erros totais
    local erros
    erros=$(echo "$vg_output" | grep -i "ERROR SUMMARY" | grep -oP '[0-9]+(?= errors)' | head -1)
    erros=${erros:-"0"}

    echo "${bytes_uso},${erros}"
}

# Função para testar Lamport
testar_lamport() {
    local teste_num=$1
    echo -e "${BLUE}Testando Lamport - Teste $teste_num${NC}"
    
    # Limpar arquivos anteriores do LOTS
    (cd ../LOTS && rm -f assinatura.txt mensagem.txt publicKeys.txt 2>/dev/null || true)
    
    # Teste de assinatura (opção 1)
    local output_assinatura=$(cd ../LOTS && echo -e "1\n$MENSAGEM_TESTE" | timeout 30 ./lots 2>&1)
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste Lamport $teste_num (assinatura)${NC}"
        return 1
    fi
    
    # Teste de verificação (opção 2)
    local output_verificacao=$(cd ../LOTS && echo "2" | timeout 30 ./lots 2>&1)
    
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

    # Tamanho real dos arquivos (se existirem)
    tamanho_assinatura=$(file_size_or_default "../LOTS/assinatura.txt" "$tamanho_assinatura")
    tamanho_public=$(file_size_or_default "../LOTS/publicKeys.txt" "$tamanho_public")
    tamanho_secret=$(file_size_or_default "../LOTS/secretKeys.txt" "$tamanho_secret")
    
    # Coleta valgrind (só na 1ª execução para economizar tempo)
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        vg_metrics=$(coletar_valgrind "./lots" "../LOTS" "1\n$MENSAGEM_TESTE")
    fi
    
    # Salva no CSV (Lamport não tem masks, então usa 0)
    echo "LOTS,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura,$vg_metrics" >> "$RESULTADO_FILE"
    
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
    local output_assinatura=$(cd ../WOTS && echo -e "1\n$MENSAGEM_TESTE" | timeout 30 ./wots 2>&1)
    
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

    # Tamanho real dos arquivos (se existirem)
    tamanho_assinatura=$(file_size_or_default "../WOTS/Assinatura.txt" "$tamanho_assinatura")
    tamanho_public=$(file_size_or_default "../WOTS/PublicKeys.txt" "$tamanho_public")
    tamanho_secret=$(file_size_or_default "../WOTS/SecretKeys.txt" "$tamanho_secret")
    
    # Coleta valgrind (só na 1ª execução)
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        vg_metrics=$(coletar_valgrind "./wots" "../WOTS" "1\n$MENSAGEM_TESTE")
    fi
    
    # Salva no CSV com tempos separados
    echo "WOTS,$teste_num,$tempo_secret_keys,$tempo_public_keys,$tempo_masks,$tempo_assinatura,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura,$vg_metrics" >> "$RESULTADO_FILE"
    
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
    local output_assinatura=$(cd ../HORS && ./hors_test "$MENSAGEM_TESTE" 2>&1)
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste HORS $teste_num${NC}"
        return 1
    fi
    
    # Extrai valores do HORS usando awk mais cuidadoso
    local tempo_secret_keys=$(echo "$output_assinatura" | grep "Tempo para gerar Chaves Secretas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_public_keys=$(echo "$output_assinatura" | grep "Tempo para gerar Chaves Publicas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_assinatura=$(echo "$output_assinatura" | grep "Tempo para Assinar:" | awk -F': ' '{print $2}' | awk '{print $1}')
    
    local hashes_assinatura=$(echo "$output_assinatura" | grep "Total de hashes SHA256:" | awk -F': ' '{print $2}')
    
    local tamanho_secret=$(echo "$output_assinatura" | grep "Tamanho Secretkeys:" | awk -F': ' '{print $2}')
    local tamanho_public=$(echo "$output_assinatura" | grep "Tamanho Publickeys:" | awk -F': ' '{print $2}')
    local tamanho_assinatura=$(echo "$output_assinatura" | grep "Tamanho Assinatura:" | awk -F': ' '{print $2}')
    
    # Valores padrão se não encontrados
    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    hashes_assinatura=${hashes_assinatura:-"0"}
    tamanho_secret=${tamanho_secret:-"32768"}
    tamanho_public=${tamanho_public:-"32768"}
    tamanho_assinatura=${tamanho_assinatura:-"832"}

    # Tamanho real dos arquivos (se existirem)
    (cd ../HORS && echo -e "1\n$MENSAGEM_TESTE\n0" | timeout 30 ./hors > /dev/null 2>&1)
    tamanho_assinatura=$(file_size_or_default "../HORS/assinatura.txt" "$tamanho_assinatura")
    tamanho_public=$(file_size_or_default "../HORS/publicKeys.txt" "$tamanho_public")
    tamanho_secret=$(file_size_or_default "../HORS/secretKeys.txt" "$tamanho_secret")
    
    # Coleta valgrind (só na 1ª execução)
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        vg_metrics=$(coletar_valgrind "./hors_test" "../HORS" "$MENSAGEM_TESTE")
    fi
    
    # Salva no CSV (HORS não tem masks, então usa 0)
    echo "HORS,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura,$vg_metrics" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}HORS Teste $teste_num: OK${NC}"
    echo "  SK: ${tempo_secret_keys}s | PK: ${tempo_public_keys}s | Assinatura: ${tempo_assinatura}s | Hashes: $hashes_assinatura"
}

# Função para testar HORST
testar_horst() {
    local teste_num=$1
    echo -e "${BLUE}Testando HORST - Teste $teste_num${NC}"
    
    # Limpar arquivos anteriores do HORST
    (cd ../HORST && rm -f *.bin mensagem.txt 2>/dev/null || true)
    
    # HORST é interativo, gera métricas durante execução
    # Gerar chaves (opção 1), Assinar (opção 2), Verificar (opção 3)
    local output=$(cd ../HORST && echo -e "1\n2\nmensagem_teste_horst_${teste_num}\n3\n0" | timeout 30 ./testeHORST 2>&1)
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste HORST $teste_num${NC}"
        # Valores padrão para falha
        echo "HORST,$teste_num,0,0,0,0,0,32768,32768,832" >> "$RESULTADO_FILE"
        return 1
    fi
    
    # Extrair tempos do output
    local tempo_secret_keys=$(echo "$output" | grep "Tempo para gerar Chaves Secretas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_public_keys=$(echo "$output" | grep "Tempo para gerar Chave Publica:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_assinatura=$(echo "$output" | grep "Tempo para Assinar:" | awk -F': ' '{print $2}' | awk '{print $1}')
    
    # Valores padrão se não encontrados
    tempo_secret_keys=${tempo_secret_keys:-"0"}
    tempo_public_keys=${tempo_public_keys:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    
    # Tamanho real dos arquivos (se existirem)
    local tamanho_secret=$(file_size_or_default "../HORST/seckeys.bin" "32768")
    local tamanho_public=$(file_size_or_default "../HORST/pubkey.bin" "32")
    local tamanho_assinatura=$(file_size_or_default "../HORST/assinatura.bin" "832")
    
    # Extrair hashes do output
    local hashes_keygen=$(echo "$output" | grep "Total de hashes SHA256 (keygen)" | awk -F': ' '{print $2}')
    local hashes_assinatura=$(echo "$output" | grep "Total de hashes SHA256 (assinatura)" | awk -F': ' '{print $2}')
    hashes_keygen=${hashes_keygen:-"0"}
    hashes_assinatura=${hashes_assinatura:-"0"}
    
    # Coleta valgrind (só na 1ª execução)
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        vg_metrics=$(coletar_valgrind "./testeHORST" "../HORST" "1\n2\nmensagem_valgrind\n3\n0")
    fi
    
    # Salva no CSV (hashes_assinatura = soma keygen + assinatura para linha única)
    echo "HORST,$teste_num,$tempo_secret_keys,$tempo_public_keys,0,$tempo_assinatura,$hashes_assinatura,$tamanho_secret,$tamanho_public,$tamanho_assinatura,$vg_metrics" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}HORST Teste $teste_num: OK${NC}"
    echo "  SK: ${tempo_secret_keys}s | PK: ${tempo_public_keys}s | Assinatura: ${tempo_assinatura}s"
}

# Função para testar MSS
testar_mss() {
    local teste_num=$1
    echo -e "${BLUE}Testando MSS - Teste $teste_num${NC}"
    
    # Limpar arquivos anteriores do MSS
    (cd ../MSS && rm -f *.txt *.bin 2>/dev/null || true)
    
    # MSS é interativo: Gerar árvore (1), Assinar (2), Verificar (3)
    local output=$(cd ../MSS && echo -e "1\n2\nmensagem_teste_mss_${teste_num}\n3\n0" | timeout 120 ./mss 2>&1)
    
    if [ $? -ne 0 ]; then
        echo -e "${RED}Erro no teste MSS $teste_num${NC}"
        # Valores padrão para falha
        echo "MSS,$teste_num,0,0,0,0,0,2144,2144,2144" >> "$RESULTADO_FILE"
        return 1
    fi
    
    # Extrair tempos do MSS
    local tempo_folhas=$(echo "$output" | grep "Tempo para gerar Folhas:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_arvore=$(echo "$output" | grep "Tempo para gerar Árvore:" | awk -F': ' '{print $2}' | awk '{print $1}')
    local tempo_assinatura=$(echo "$output" | grep "Tempo para Assinar:" | awk -F': ' '{print $2}' | awk '{print $1}')
    
    # Valores padrão se não encontrados
    tempo_folhas=${tempo_folhas:-"0"}
    tempo_arvore=${tempo_arvore:-"0"}
    tempo_assinatura=${tempo_assinatura:-"0"}
    
    # Tamanho real dos arquivos (se existirem)
    local tamanho_secret=$(file_size_or_default "../MSS/folhas.txt" "0")
    local tamanho_public=$(file_size_or_default "../MSS/public_key.txt" "0")
    local tamanho_assinatura=$(file_size_or_default "../MSS/assinatura.txt" "0")
    
    # Coleta valgrind (só na 1ª execução, e com timeout maior)
    local vg_metrics="0,0"
    if [ "$teste_num" -eq 1 ]; then
        vg_metrics=$(coletar_valgrind "./mss" "../MSS" "1\n2\nmensagem_valgrind\n3\n0")
    fi
    
    # Salva no CSV (tempo_folhas, tempo_arvore, 0, tempo_assinatura)
    # Usando colunas: Tempo_SecretKeys=folhas, Tempo_PublicKeys=arvore, Tempo_Masks=0, Tempo_Assinatura
    echo "MSS,$teste_num,$tempo_folhas,$tempo_arvore,0,$tempo_assinatura,0,$tamanho_secret,$tamanho_public,$tamanho_assinatura,$vg_metrics" >> "$RESULTADO_FILE"
    
    echo -e "${GREEN}MSS Teste $teste_num: OK${NC}"
    echo "  Folhas: ${tempo_folhas}s | Árvore: ${tempo_arvore}s | Assinatura: ${tempo_assinatura}s"
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
    
    # Testa HORST
    testar_horst $i
    sleep 1
    
    # Testa MSS
    testar_mss $i
    sleep 1
    
    echo
done
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