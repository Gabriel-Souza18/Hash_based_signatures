# Makefile para TesteHash

CC = gcc
CFLAGS = -Wall -O2 -I../SHA256
LDFLAGS = -L../SHA256 -lsha256

# Arquivos fonte
SRC = main.c dicionario.c
OBJ = $(SRC:.c=.o)
TARGET = testeHash

# Target padrão
all: $(TARGET)

# Compilar executável
$(TARGET): $(OBJ) ../SHA256/libsha256.a
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "TesteHash compilado!"

# Compilar objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

main.o: main.c dicionario.h
	$(CC) $(CFLAGS) -c main.c -o main.o

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

# Limpar arquivos gerados
clean:
	@echo "Limpando TesteHash..."
	rm -f $(OBJ) $(TARGET)
	@echo "Limpeza concluída!"

.PHONY: all clean
