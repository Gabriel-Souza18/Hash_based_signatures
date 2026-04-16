# Makefile para HORS (Hash to Obtain Random Subset)

CC = gcc
CFLAGS = -Wall -O2 -I../SHA256 -DHORS_T=1024
LDFLAGS = -L../SHA256 -lsha256 -lsodium -Wl,-rpath,/usr/local/lib

# Arquivos fonte
SRC = keys.c hors.c
OBJ = $(SRC:.c=.o)
TARGET = hors

# Target padrão
all: $(TARGET)

# Compilar executável
$(TARGET): $(OBJ) ../SHA256/libsha256.a
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "HORS compilado com HORS_T=1024!"

# Compilar objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

keys.o: keys.c keys.h
	$(CC) $(CFLAGS) -c keys.c -o keys.o

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

# Limpar arquivos gerados
clean:
	@echo "Limpando HORS..."
	rm -f $(OBJ) $(TARGET)
	rm -f assinatura.txt mensagem.txt publicKeys.txt secretKeys.txt
	@echo "Limpeza concluída!"

.PHONY: all clean

