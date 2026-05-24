CC = gcc
CFLAGS = -Wall -O2 -I../SHA256 -DHORST_T=1024
LDFLAGS = -lm -L../SHA256 -lsha256 -lsodium -lcrypto -Wl,-rpath,/usr/local/lib

# Arquivos
SRCS = keys.c utils.c horst_tree.c testeHORST.c
OBJS = $(SRCS:.c=.o)
TARGET = testeHORST

all: $(TARGET)

$(TARGET): $(OBJS) ../SHA256/libsha256.a
	$(CC) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Garantir que a biblioteca SHA256 está compilada
../SHA256/libsha256.a:
	@echo "Compilando biblioteca SHA256..."
	$(MAKE) -C ../SHA256

test: all
	./$(TARGET)

clean:
	rm -f $(OBJS) $(TARGET)
	rm -f *.bin
.PHONY: all test clean
