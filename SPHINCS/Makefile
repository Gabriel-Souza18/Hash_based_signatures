CC     = gcc
CFLAGS = -Wall -Wextra -O2 -I../SHA256 -I../WOTS
LDFLAGS = -L../SHA256 -lsha256 -lsodium -lcrypto -lm -Wl,-rpath,/usr/local/lib

SHA256_LIB = ../SHA256/libsha256.a

# ── objetos compartilhados ────────────────────────────────────────────────
WOTS_OBJS  = ../WOTS/keys.o ../WOTS/prf.o
HORST_OBJS = ../HORST/horst_tree.o
SPHINCS_BASE_OBJS = sphincs_address.o ltree.o xmss.o
SPHINCS_ALL_OBJS  = $(SPHINCS_BASE_OBJS) sphincs.o

# ── targets ───────────────────────────────────────────────────────────────
all: remet_sphincs dest_sphincs

# ── WOTS (compilados no diretório deles) ─────────────────────────────────
../WOTS/keys.o: ../WOTS/keys.c
	$(CC) $(CFLAGS) -c $< -o $@

../WOTS/prf.o: ../WOTS/prf.c
	$(CC) $(CFLAGS) -c $< -o $@

# ── HORST (compilados no diretório deles) ────────────────────────────────
../HORST/horst_tree.o: ../HORST/horst_tree.c ../HORST/horst_tree.h
	$(CC) $(CFLAGS) -I../HORST -DHORST_T=1024 -c $< -o $@

# ── módulos SPHINCS ───────────────────────────────────────────────────────
sphincs_address.o: sphincs_address.c sphincs_address.h
	$(CC) $(CFLAGS) -c $< -o $@

ltree.o: ltree.c ltree.h sphincs_address.h
	$(CC) $(CFLAGS) -c $< -o $@

xmss.o: xmss.c xmss.h ltree.h sphincs_address.h
	$(CC) $(CFLAGS) -c $< -o $@

sphincs.o: sphincs.c sphincs.h xmss.h ltree.h sphincs_address.h
	$(CC) $(CFLAGS) -I../HORST -DHORST_T=1024 -c $< -o $@

# ── executáveis de teste ──────────────────────────────────────────────────
remet_sphincs: remet_sphincs.c $(SPHINCS_ALL_OBJS) $(WOTS_OBJS) $(HORST_OBJS) $(SHA256_LIB)
	$(CC) $(CFLAGS) -I../HORST -DHORST_T=1024 remet_sphincs.c $(SPHINCS_ALL_OBJS) $(WOTS_OBJS) $(HORST_OBJS) \
	    $(SHA256_LIB) $(LDFLAGS) -o remet_sphincs

dest_sphincs: dest_sphincs.c $(SPHINCS_ALL_OBJS) $(WOTS_OBJS) $(HORST_OBJS) $(SHA256_LIB)
	$(CC) $(CFLAGS) -I../HORST -DHORST_T=1024 dest_sphincs.c $(SPHINCS_ALL_OBJS) $(WOTS_OBJS) $(HORST_OBJS) \
	    $(SHA256_LIB) $(LDFLAGS) -o dest_sphincs

# ── SHA256 (compilado no diretório dele) ──────────────────────────────────
$(SHA256_LIB):
	$(MAKE) -C ../SHA256

# ── limpeza ───────────────────────────────────────────────────────────────
clean:
	rm -f sphincs_address.o ltree.o xmss.o sphincs.o
	rm -f ../WOTS/keys.o ../WOTS/prf.o
	rm -f ../HORST/horst_tree.o
	rm -f remet_sphincs dest_sphincs
	rm -f test_address test_ltree test_xmss test_sphincs
	rm -f *.bin
.PHONY: all clean
