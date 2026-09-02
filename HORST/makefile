# Makefile para HORST (HORS with Trees)

CC = gcc
CFLAGS = -Wall -O2 -I../SHA256 -DHORST_T=1024
LDFLAGS = -lm -L../SHA256 -lsha256 -lsodium -lcrypto -Wl,-rpath,/usr/local/lib

# Arquivos fonte comuns (keys + utils + árvore)
COMMON_OBJ = keys.o utils.o horst_tree.o

# Executáveis remetente/destinatário
REMET_TARGET = remet_horst
DEST_TARGET = dest_horst

# Executável legado (menu interativo)
TARGET = testeHORST

# Target padrão: remetente e destinatário separados
all: $(REMET_TARGET) $(DEST_TARGET)

# Remetente HORST
$(REMET_TARGET): $(COMMON_OBJ) remet_horst.o ../SHA256/libsha256.a
	$(CC) $(COMMON_OBJ) remet_horst.o -o $(REMET_TARGET) $(LDFLAGS)
	@echo "remet_horst compilado!"

# Destinatário HORST
$(DEST_TARGET): $(COMMON_OBJ) dest_horst.o ../SHA256/libsha256.a
	$(CC) $(COMMON_OBJ) dest_horst.o -o $(DEST_TARGET) $(LDFLAGS)
	@echo "dest_horst compilado!"

# Compilar executável com menu (legado)
menu: $(COMMON_OBJ) testeHORST.o ../SHA256/libsha256.a
	$(CC) $(COMMON_OBJ) testeHORST.o -o $(TARGET) $(LDFLAGS)
	@echo "testeHORST (menu) compilado!"

# Compilar objetos
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

keys.o: keys.c keys.h horst_tree.h
	$(CC) $(CFLAGS) -c keys.c -o keys.o

utils.o: utils.c utils.h keys.h
	$(CC) $(CFLAGS) -c utils.c -o utils.o

horst_tree.o: horst_tree.c horst_tree.h keys.h
	$(CC) $(CFLAGS) -c horst_tree.c -o horst_tree.o

remet_horst.o: remet_horst.c keys.h utils.h
	$(CC) $(CFLAGS) -c remet_horst.c -o remet_horst.o

dest_horst.o: dest_horst.c keys.h utils.h
	$(CC) $(CFLAGS) -c dest_horst.c -o dest_horst.o

testeHORST.o: testeHORST.c keys.h utils.h
	$(CC) $(CFLAGS) -c testeHORST.c -o testeHORST.o

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

# Limpar arquivos gerados
clean:
	@echo "Limpando HORST..."
	rm -f $(COMMON_OBJ) remet_horst.o dest_horst.o testeHORST.o
	rm -f $(TARGET) $(REMET_TARGET) $(DEST_TARGET)
	rm -f *.bin mensagem.txt
	@echo "Limpeza concluída!"

.PHONY: all menu test clean
