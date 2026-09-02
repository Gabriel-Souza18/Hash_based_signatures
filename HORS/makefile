# Makefile para HORS (Hash to Obtain Random Subset)

CC = gcc
CFLAGS = -Wall -O2 -I../SHA256 -DHORS_T=1024
LDFLAGS = -lm -L../SHA256 -lsha256 -lsodium -lcrypto -Wl,-rpath,/usr/local/lib

# Arquivos fonte comuns (keys + utils)
COMMON_OBJ = keys.o utils.o

# Executáveis remetente/destinatário
REMET_TARGET = remet_hors
DEST_TARGET = dest_hors

# Executáveis legado (menu interativo e testes)
TARGET = hors
TEST_TARGET = testeHors
QUICK_TEST = hors_test

# Target padrão: remetente e destinatário separados
all: $(REMET_TARGET) $(DEST_TARGET)

# Remetente HORS
$(REMET_TARGET): $(COMMON_OBJ) remet_hors.o ../SHA256/libsha256.a
	$(CC) $(COMMON_OBJ) remet_hors.o -o $(REMET_TARGET) $(LDFLAGS)
	@echo "remet_hors compilado!"

# Destinatário HORS
$(DEST_TARGET): $(COMMON_OBJ) dest_hors.o ../SHA256/libsha256.a
	$(CC) $(COMMON_OBJ) dest_hors.o -o $(DEST_TARGET) $(LDFLAGS)
	@echo "dest_hors compilado!"

# Compilar executável com menu (legado)
menu: $(COMMON_OBJ) hors.o ../SHA256/libsha256.a
	$(CC) $(COMMON_OBJ) hors.o -o $(TARGET) $(LDFLAGS)
	@echo "HORS (menu) compilado!"

# Compilar teste rápido (sem menu)
$(QUICK_TEST): $(COMMON_OBJ) hors_test.o ../SHA256/libsha256.a
	$(CC) $(COMMON_OBJ) hors_test.o -o $(QUICK_TEST) $(LDFLAGS)
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

remet_hors.o: remet_hors.c keys.h utils.h
	$(CC) $(CFLAGS) -c remet_hors.c -o remet_hors.o

dest_hors.o: dest_hors.c keys.h utils.h
	$(CC) $(CFLAGS) -c dest_hors.c -o dest_hors.o

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
	rm -f $(COMMON_OBJ) hors.o remet_hors.o dest_hors.o testeHors.o hors_test.o
	rm -f $(TARGET) $(REMET_TARGET) $(DEST_TARGET) $(TEST_TARGET) $(QUICK_TEST)
	rm -f assinatura.txt mensagem.txt publicKeys.txt secretKeys.txt
	rm -f assinatura.bin publicKeys.bin
	@echo "Limpeza concluída!"

.PHONY: all menu test clean
