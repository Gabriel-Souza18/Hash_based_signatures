# Makefile para WOTS (Winternitz OTS)

CC = gcc
CFLAGS = -Wall -O2 -I../SHA256
LDFLAGS = -L../SHA256 -lsha256 -lsodium -lcrypto -Wl,-rpath,/usr/local/lib

# Objetos comuns
COMMON_OBJ = keys.o utils.o prf.o

# Executáveis
TARGETS = wots remet_wots dest_wots

# Target padrão
all: $(TARGETS)

# Compilar executável wots
wots: wots.o $(COMMON_OBJ) ../SHA256/libsha256.a
	$(CC) wots.o $(COMMON_OBJ) -o wots $(LDFLAGS)
	@echo "WOTS compilado!"

# Compilar remet_wots
remet_wots: remet_wots.o $(COMMON_OBJ) ../SHA256/libsha256.a
	$(CC) remet_wots.o $(COMMON_OBJ) -o remet_wots $(LDFLAGS)
	@echo "remet_wots compilado!"

# Compilar dest_wots
dest_wots: dest_wots.o $(COMMON_OBJ) ../SHA256/libsha256.a
	$(CC) dest_wots.o $(COMMON_OBJ) -o dest_wots $(LDFLAGS)
	@echo "dest_wots compilado!"

# Compilar objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

wots.o: wots.c utils.h keys.h
	$(CC) $(CFLAGS) -c wots.c -o wots.o

remet_wots.o: remet_wots.c utils.h keys.h
	$(CC) $(CFLAGS) -c remet_wots.c -o remet_wots.o

dest_wots.o: dest_wots.c utils.h keys.h
	$(CC) $(CFLAGS) -c dest_wots.c -o dest_wots.o

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

# Limpar arquivos gerados
clean:
	@echo "Limpando WOTS..."
	rm -f *.o $(TARGETS)
	rm -f Assinatura.bin Mensagem.txt PublicKeys.bin Assinatura.txt
	@echo "Limpeza concluída!"


.PHONY: all clean
