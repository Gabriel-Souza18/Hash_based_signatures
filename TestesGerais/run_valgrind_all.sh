#!/usr/bin/env bash
set -u

# Script de Valgrind específico por algoritmo (fluxo interativo real de cada um).
# Executa remetente/destinatário com entradas diferentes por programa.

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT_DIR="$(cd "$SCRIPT_DIR/.." && pwd)"

LOG_DIR="$SCRIPT_DIR/Resultados_Valgrind"
mkdir -p "$LOG_DIR"

VALGRIND="valgrind"
VALGRIND_OPTS=(--track-origins=yes --leak-check=full --show-leak-kinds=all)
TIMEOUT_SECS=40

MENSAGEM_BASE="mensagem_teste_valgrind_$(date +%Y%m%d_%H%M%S)"

log_info() { echo "[INFO] $*"; }
log_warn() { echo "[WARN] $*"; }

build_modulo() {
  local dir="$1"
  log_info "Compilando modulo $dir"
  if ! (cd "$ROOT_DIR/$dir" && make clean >/dev/null 2>&1 || true; make >/dev/null); then
    log_warn "Falha ao compilar $dir"
    return 1
  fi
  return 0
}

run_valgrind_cmd() {
  local dir="$1"
  local exe="$2"
  local tag="$3"
  local input_data="$4"
  shift 4
  local args=("$@")

  local exe_path="$ROOT_DIR/$dir/$exe"
  local prefix="$LOG_DIR/${dir}_${exe}_${tag}"

  if [ ! -x "$exe_path" ]; then
    log_warn "Executavel ausente: $exe_path"
    return 1
  fi

  log_info "Valgrind -> $dir/$exe [$tag]"
  if command -v timeout >/dev/null 2>&1; then
    if [ -n "$input_data" ]; then
      (cd "$ROOT_DIR/$dir" && printf "%b" "$input_data" | timeout "${TIMEOUT_SECS}s" "$VALGRIND" "${VALGRIND_OPTS[@]}" "./$exe" "${args[@]}") >"${prefix}.stdout" 2>"${prefix}.valgrind" || true
    else
      (cd "$ROOT_DIR/$dir" && timeout "${TIMEOUT_SECS}s" "$VALGRIND" "${VALGRIND_OPTS[@]}" "./$exe" "${args[@]}") >"${prefix}.stdout" 2>"${prefix}.valgrind" || true
    fi
  else
    if [ -n "$input_data" ]; then
      (cd "$ROOT_DIR/$dir" && printf "%b" "$input_data" | "$VALGRIND" "${VALGRIND_OPTS[@]}" "./$exe" "${args[@]}") >"${prefix}.stdout" 2>"${prefix}.valgrind" || true
    else
      (cd "$ROOT_DIR/$dir" && "$VALGRIND" "${VALGRIND_OPTS[@]}" "./$exe" "${args[@]}") >"${prefix}.stdout" 2>"${prefix}.valgrind" || true
    fi
  fi
  log_info "Logs: ${prefix}.stdout | ${prefix}.valgrind"
  return 0
}

rodar_lots() {
  build_modulo "LOTS" || return 1
  run_valgrind_cmd "LOTS" "lots" "remetente" "1\n${MENSAGEM_BASE}_lots\n"
  run_valgrind_cmd "LOTS" "lots" "destinatario" "2\n"
}

rodar_wots() {
  build_modulo "WOTS" || return 1
  run_valgrind_cmd "WOTS" "wots" "remetente" "1\n${MENSAGEM_BASE}_wots\n"
  run_valgrind_cmd "WOTS" "wots" "destinatario" "2\n"
}

rodar_hors() {
  build_modulo "HORS" || return 1
  run_valgrind_cmd "HORS" "hors" "remetente" "1\n${MENSAGEM_BASE}_hors\n0\n"
  run_valgrind_cmd "HORS" "hors" "destinatario" "2\n0\n"
}

rodar_horst() {
  build_modulo "HORST" || return 1
  run_valgrind_cmd "HORST" "testeHORST" "remetente" "1\n2\n${MENSAGEM_BASE}_horst\n0\n"
  run_valgrind_cmd "HORST" "testeHORST" "destinatario" "3\n0\n"
}

rodar_mss() {
  build_modulo "MSS" || return 1

  # MSS executa apenas uma opção por execução. No remetente fazemos duas execuções:
  # 1) gerar árvore, 2) criar assinatura.
  run_valgrind_cmd "MSS" "mss" "remetente_gerar_arvore" "1\n"
  run_valgrind_cmd "MSS" "mss" "remetente_assinar" "2\n${MENSAGEM_BASE}_mss\n"
  run_valgrind_cmd "MSS" "mss" "destinatario" "3\n"
}

echo "Iniciando Valgrind com fluxo específico por algoritmo..."

rodar_lots || true
rodar_wots || true
rodar_hors || true
rodar_horst || true
rodar_mss || true

echo "Concluido. Logs em: $LOG_DIR"

echo "Limpando arquivos temporários..."
(cd "$ROOT_DIR" && make clean > /dev/null 2>&1)
echo "Limpeza concluída!"
