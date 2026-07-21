# Makefile para LOTS (Lamport OTS)

CC = gcc
CFLAGS = -Wall -O2 -I../SHA256
LDFLAGS = -L../SHA256 -lsha256 -lsodium -lcrypto -Wl,-rpath,/usr/local/lib

# Arquivos fonte e objetos comuns
COMMON_SRC = keys.c utils.c
COMMON_OBJ = $(COMMON_SRC:.c=.o)

# Executáveis
TARGETS = remet_lots dest_lots

# Target padrão
all: $(TARGETS)

# Compilar remet_lots
remet_lots: remet_lots.o $(COMMON_OBJ) ../SHA256/libsha256.a
	$(CC) remet_lots.o $(COMMON_OBJ) -o remet_lots $(LDFLAGS)
	@echo "remet_lots compilado!"

# Compilar dest_lots
dest_lots: dest_lots.o $(COMMON_OBJ) ../SHA256/libsha256.a
	$(CC) dest_lots.o $(COMMON_OBJ) -o dest_lots $(LDFLAGS)
	@echo "dest_lots compilado!"

# Compilar objetos comuns e individuais
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

remet_lots.o: remet_lots.c utils.h keys.h
	$(CC) $(CFLAGS) -c remet_lots.c -o remet_lots.o

dest_lots.o: dest_lots.c utils.h keys.h
	$(CC) $(CFLAGS) -c dest_lots.c -o dest_lots.o

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

# Limpar arquivos gerados
clean:
	@echo "Limpando LOTS..."
	rm -f *.o $(TARGETS) lots
	rm -f assinatura.txt mensagem.txt publicKeys.txt
	@echo "Limpeza concluída!"

.PHONY: all clean
