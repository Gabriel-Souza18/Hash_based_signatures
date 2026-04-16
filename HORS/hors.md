# Valor padrão (1024)
make -C HORS

# Alterar durante compilação (ex: 256)
cd HORS && make clean && make CFLAGS="-Wall -O2 -I../SHA256 -DHORS_T=256"