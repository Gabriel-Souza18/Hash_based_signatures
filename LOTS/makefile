# Makefile para lots OTS

CC = gcc
CFLAGS = -Wall -O2 -I../SHA256
LDFLAGS = -L../SHA256 -lsha256 -lsodium -Wl,-rpath,/usr/local/lib

# Arquivos fonte
SRC = lots.c keys.c utils.c
OBJ = $(SRC:.c=.o)
TARGET = lots

# Target padrão
all: $(TARGET)

# Compilar executável
$(TARGET): $(OBJ) ../SHA256/libsha256.a
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "lots OTS compilado!"

# Compilar objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

lots.o: lots.c utils.h keys.h
	$(CC) $(CFLAGS) -c lots.c -o lots.o

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

# Limpar arquivos gerados
clean:
	@echo "Limpando lots OTS..."
	rm -f $(OBJ) $(TARGET)
	rm -f assinatura.txt mensagem.txt publicKeys.txt
	@echo "Limpeza concluída!"

.PHONY: all clean
