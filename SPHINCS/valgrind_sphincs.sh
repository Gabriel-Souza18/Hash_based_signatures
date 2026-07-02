#!/usr/bin/env bash
set -e

make

# Arquivos para o teste
MSG_FILE="msg_valgrind_test.txt"
PK_FILE="pk_valgrind.bin"
SIG_FILE="sig_valgrind.bin"
SK_SEED_FILE="sk_seed_valgrind.bin"
SK_FILE="sk_valgrind.bin"

echo "Criando mensagem de teste..."
echo "Mensagem para assinatura SPHINCS via Valgrind" > "$MSG_FILE"

echo ""
echo "1. Rodando Remetente (Keygen & Sign) com Valgrind Memcheck"
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
  ./remet_sphincs "$MSG_FILE" "$PK_FILE" "$SIG_FILE" "$SK_SEED_FILE" "$SK_FILE"

echo ""
echo "2. Rodando Destinatário (Verify) com Valgrind Memcheck"
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
  ./dest_sphincs "$MSG_FILE" "$PK_FILE" "$SIG_FILE" "$SK_SEED_FILE"

# Limpeza 
rm -f "$MSG_FILE" "$PK_FILE" "$SIG_FILE" "$SK_SEED_FILE" "$SK_FILE"

echo ""
echo "Informação Adicional"
echo "Para analisar o consumo de pico de memória (Heap profiling) usando a ferramenta Massif, rode:"
echo "  valgrind --tool=massif ./remet_sphincs msg.txt pk.bin sig.bin sk_seed.bin sk.bin"
echo "Depois execute: ms_print massif.out.<PID>"
