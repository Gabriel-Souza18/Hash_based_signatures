# Makefile para biblioteca SHA256

CC = gcc
CFLAGS = -Wall -O2 -fPIC
AR = ar
ARFLAGS = rcs

# Arquivos fonte e objeto
SRC = sha256.c
OBJ = sha256.o
LIB = libsha256.a

# Target padrão: compilar biblioteca estática
all: $(LIB)

# Criar biblioteca estática
$(LIB): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^
	@echo "Biblioteca SHA256 criada: $(LIB)"

# Compilar arquivo objeto
$(OBJ): $(SRC) sha256.h
	$(CC) $(CFLAGS) -c $(SRC) -o $(OBJ)

# Limpar arquivos gerados
clean:
	@echo "Limpando biblioteca SHA256..."
	rm -f $(OBJ) $(LIB)
	@echo "Limpeza concluída!"

.PHONY: all clean
