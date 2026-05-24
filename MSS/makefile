# Makefile para MSS (Merkle Signature Scheme)

CC = gcc
CFLAGS = -Wall -g -O0 -I../SHA256 -I../WOTS
LDFLAGS = -L../SHA256 -lsha256 -lm -lsodium -lcrypto -Wl,-rpath,/usr/local/lib

# Arquivos fonte
SRC = mss.c Arvore.c
OBJ = $(SRC:.c=.o)

# Objetos do WOTS que precisamos linkar
WOTS_OBJ = ../WOTS/keys.o ../WOTS/utils.o ../WOTS/prf.o

TARGET = mss

# Target padrão
all: $(TARGET)

# Compilar executável
$(TARGET): $(OBJ) $(WOTS_OBJ) ../SHA256/libsha256.a
	$(CC) $(OBJ) $(WOTS_OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "MSS compilado!"

# Compilar objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

mss.o: mss.c Arvore.h
	$(CC) $(CFLAGS) -c mss.c -o mss.o

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

# Garantir que os objetos do WOTS estão compilados
$(WOTS_OBJ):
	@echo "Compilando WOTS..."
	$(MAKE) -C ../WOTS

# Limpar arquivos gerados
clean:
	@echo "Limpando MSS..."
	rm -f $(OBJ) $(TARGET)
	rm -f *.bin
	rm -f *.txt
	@echo "Limpeza concluída!"

.PHONY: all clean
