#!/usr/bin/env bash
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
source "$SCRIPT_DIR/run_valgrind_all.sh" >/dev/null 2>&1 || true
# We can just extract the rodar_sphincs part:
MENSAGEM_BASE="mensagem_teste_valgrind_$(date +%Y%m%d_%H%M%S)"
rodar_sphincs
