#!/usr/bin/env bash
set -e


make

# Arquivos temporários para o teste
MSG_FILE="msg_valgrind_test.txt"
PK_FILE="pk_valgrind.txt"
SIG_FILE="sig_valgrind.txt"

echo "Criando mensagem de teste..."
echo "Mensagem para assinatura LOTS via Valgrind" > "$MSG_FILE"

echo ""
echo " 1. Rodando Remetente (Keygen & Sign) com Valgrind Memcheck "
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
  ./remet_lots "$MSG_FILE" "$PK_FILE" "$SIG_FILE"

echo ""
echo " 2. Rodando Destinatário (Verify) com Valgrind Memcheck "
valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all \
  ./dest_lots "$MSG_FILE" "$PK_FILE" "$SIG_FILE"

# Limpeza dos arquivos temporários gerados pelo teste
rm -f "$MSG_FILE" "$PK_FILE" "$SIG_FILE"

echo ""
echo " Informação Adicional "
echo "Para analisar o consumo de pico de memória (Heap profiling) usando a ferramenta Massif, rode:"
echo "  valgrind --tool=massif ./remet_lots msg.txt pk.txt sig.txt"
echo "Depois execute: ms_print massif.out.<PID>"
