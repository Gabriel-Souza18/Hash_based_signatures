CC = gcc
CFLAGS = -Wall -O2 -Wno-unused-result -Wno-deprecated-declarations \
	-I../../../MSS \
	-I../../../WOTS \
	-I../../../SHA256 \
	-I../..
LDFLAGS = -lmosquitto -lsodium -lcrypto -lm

SOURCES = publisherMss.c \
	../../Utils/IO.c \
	ArvoreMss.c \
	../../../WOTS/keys.c \
	../../../WOTS/prf.c \
	../../../SHA256/sha256.c
TARGET = publisherMss

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
