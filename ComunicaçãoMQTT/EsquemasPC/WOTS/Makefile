CC      = gcc
CFLAGS  = -Wall -O2 \
           -I../../../WOTS \
           -I../../../SHA256 \
           -I../..
LDFLAGS = -lmosquitto -lsodium -lcrypto

SOURCES = publisherWots.c \
          ../../Utils/IO.c \
          ../../../WOTS/keys.c \
          ../../../WOTS/prf.c \
          ../../../SHA256/sha256.c

TARGET  = publisherWots

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
