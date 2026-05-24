# Makefile para HORS (Hash to Obtain Random Subset)

CC = gcc
CFLAGS = -Wall -O2 -I../SHA256 -DHORS_T=1024
LDFLAGS = -lm -L../SHA256 -lsha256 -lsodium -lcrypto -Wl,-rpath,/usr/local/lib

# Arquivos fonte
SRC = keys.c hors.c utils.c
OBJ = $(SRC:.c=.o)
TARGET = hors
TEST_TARGET = testeHors
QUICK_TEST = hors_test

# Target padrão
all: $(TARGET) $(QUICK_TEST)

# Compilar executável principal
$(TARGET): $(OBJ) ../SHA256/libsha256.a
	$(CC) $(OBJ) -o $(TARGET) $(LDFLAGS)
	@echo "HORS compilado com HORS_T=1024!"

# Compilar teste rápido (sem menu)
$(QUICK_TEST): keys.o utils.o hors_test.o ../SHA256/libsha256.a
	$(CC) keys.o utils.o hors_test.o -o $(QUICK_TEST) $(LDFLAGS)
	@echo "hors_test compilado!"

# Compilar teste com menu
test: keys.o testeHors.o ../SHA256/libsha256.a
	$(CC) keys.o testeHors.o -o $(TEST_TARGET) $(LDFLAGS)
	@echo "testeHors compilado!"

# Compilar objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

keys.o: keys.c keys.h
	$(CC) $(CFLAGS) -c keys.c -o keys.o

utils.o: utils.c utils.h
	$(CC) $(CFLAGS) -c utils.c -o utils.o

hors_test.o: hors_test.c keys.h utils.h
	$(CC) $(CFLAGS) -c hors_test.c -o hors_test.o

testeHors.o: testeHors.c keys.h
	$(CC) $(CFLAGS) -c testeHors.c -o testeHors.o

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

# Limpar arquivos gerados
clean:
	@echo "Limpando HORS..."
	rm -f $(OBJ) testeHors.o hors_test.o $(TARGET) $(TEST_TARGET) $(QUICK_TEST)
	rm -f assinatura.txt mensagem.txt publicKeys.txt secretKeys.txt
	rm -f assinatura.bin publicKeys.bin
	@echo "Limpeza concluída!"

.PHONY: all test clean

