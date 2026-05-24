#!/bin/bash

# Script master para executar todos os testes e consolidar resultados

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")

# Cores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
BLUE='\033[0;34m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() { echo -e "${BLUE}[INFO]${NC} $*"; }
log_success() { echo -e "${GREEN}[SUCCESS]${NC} $*"; }
log_warn() { echo -e "${YELLOW}[WARN]${NC} $*"; }
log_error() { echo -e "${RED}[ERROR]${NC} $*"; }

echo -e "${BLUE}========================================${NC}"
echo -e "${BLUE}  MASTER TEST SUITE                    ${NC}"
echo -e "${BLUE}========================================${NC}"
echo

# ===== 1. Executar testes de algoritmos =====
log_info "Executando test_algorithms.sh..."
if bash "$SCRIPT_DIR/test_algorithms.sh"; then
    log_success "test_algorithms.sh concluído"
else
    log_error "Falha em test_algorithms.sh"
    exit 1
fi

echo
sleep 2

# ===== 2. Executar testes MSS e HORST =====
log_info "Executando test_mss_horst.sh (20 iterações — MSS ~3 min, HORST ~1 min)..."
if bash "$SCRIPT_DIR/test_mss_horst.sh"; then
    log_success "test_mss_horst.sh concluído"
else
    log_error "Falha em test_mss_horst.sh"
    exit 1
fi

echo
sleep 2

# ===== 3. Executar testes Valgrind =====
log_info "Executando run_valgrind_all.sh..."
if bash "$SCRIPT_DIR/run_valgrind_all.sh"; then
    log_success "run_valgrind_all.sh concluído"
else
    log_error "Falha em run_valgrind_all.sh"
    exit 1
fi

echo
sleep 2

# ===== 4. Coletar bytes de Valgrind =====
log_info "Coletando dados de Valgrind..."
python3 "$SCRIPT_DIR/collect_valgrind_metrics.py" "$SCRIPT_DIR/Resultados_Valgrind" > "$SCRIPT_DIR/resultados_metricas/valgrind_bytes_${TIMESTAMP}.csv"
if [ $? -eq 0 ]; then
    log_success "Dados Valgrind coletados"
else
    log_warn "Falha ao coletar dados Valgrind"
fi

echo
sleep 1

# ===== 5. Consolidar resultados em tabela =====
log_info "Consolidando resultados em tabela..."
python3 "$SCRIPT_DIR/consolidate_results.py" "$SCRIPT_DIR/resultados_metricas" > "$SCRIPT_DIR/resultados_metricas/consolidated_results_${TIMESTAMP}.csv"
if [ $? -eq 0 ]; then
    log_success "Resultados consolidados em CSV"
else
    log_error "Falha ao consolidar resultados"
    exit 1
fi

echo
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}  TESTES CONCLUÍDOS COM SUCESSO!       ${NC}"
echo -e "${GREEN}========================================${NC}"
echo -e "${BLUE}Resultados em:${NC}"
echo "  - Métricas: $SCRIPT_DIR/resultados_metricas/"
echo "  - Valgrind: $SCRIPT_DIR/Resultados_Valgrind/"
echo -e "${YELLOW}Arquivo consolidado: $SCRIPT_DIR/resultados_metricas/consolidated_results_${TIMESTAMP}.csv${NC}"
echo
