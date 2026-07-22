# Makefile para MSS (Merkle Signature Scheme)

CC = gcc
CFLAGS = -Wall -g -O0 -I../SHA256 -I../WOTS
LDFLAGS = -L../SHA256 -lsha256 -lm -lsodium -lcrypto -Wl,-rpath,/usr/local/lib

# Objetos comuns
COMMON_OBJ = Arvore.o

# Objetos do WOTS que precisamos linkar
WOTS_OBJ = ../WOTS/keys.o ../WOTS/utils.o ../WOTS/prf.o

TARGETS = remet_mss dest_mss

# Target padrão
all: $(TARGETS)

# Compilar remet_mss
remet_mss: remet_mss.o $(COMMON_OBJ) $(WOTS_OBJ) ../SHA256/libsha256.a
	$(CC) remet_mss.o $(COMMON_OBJ) $(WOTS_OBJ) -o remet_mss $(LDFLAGS)
	@echo "remet_mss compilado!"

# Compilar dest_mss
dest_mss: dest_mss.o $(COMMON_OBJ) $(WOTS_OBJ) ../SHA256/libsha256.a
	$(CC) dest_mss.o $(COMMON_OBJ) $(WOTS_OBJ) -o dest_mss $(LDFLAGS)
	@echo "dest_mss compilado!"

# Compilar objetos
%.o: %.c %.h
	$(CC) $(CFLAGS) -c $< -o $@

remet_mss.o: remet_mss.c Arvore.h
	$(CC) $(CFLAGS) -c remet_mss.c -o remet_mss.o

dest_mss.o: dest_mss.c Arvore.h
	$(CC) $(CFLAGS) -c dest_mss.c -o dest_mss.o

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
	rm -f *.o $(TARGETS) mss
	rm -f *.bin
	rm -f *.txt
	@echo "Limpeza concluída!"

.PHONY: all clean
