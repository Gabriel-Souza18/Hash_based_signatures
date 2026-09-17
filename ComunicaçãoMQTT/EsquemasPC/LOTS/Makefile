CC = gcc
CFLAGS = -Wall -O2 -I../../../LOTS -I../../../SHA256 -I../..
LDFLAGS = -lmosquitto -lsodium -lcrypto

# Arquivos fonte
SOURCES = publisherLots.c ../../Utils/IO.c ../../../LOTS/keys.c ../../../SHA256/sha256.c
TARGET = publisherLots

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
