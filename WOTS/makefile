# Makefile para WOTS (Winternitz OTS)

CC = gcc
CFLAGS = -Wall -O2 -I../SHA256
LDFLAGS = -L../SHA256 -lsha256 -lsodium -Wl,-rpath,/usr/local/lib

# Arquivos fonte
SRC = wots.c keys.c utils.c prf.c
OBJ = $(SRC:.c=.o)
TARGET = wots

# Target padrão
all: $(TARGET)

# Compilar executável
$(TARGET): $(OBJ) ../SHA256/libsha256.a
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "WOTS compilado!"

# Compilar objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

wots.o: wots.c utils.h keys.h
	$(CC) $(CFLAGS) -c wots.c -o wots.o

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

# Limpar arquivos gerados
clean:
	@echo "Limpando WOTS..."
	rm -f $(OBJ) $(TARGET)
	rm -f Assinatura.txt Mensagem.txt PublicKeys.txt Masks.txt
	@echo "Limpeza concluída!"

.PHONY: all clean
