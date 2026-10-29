CC = gcc
CFLAGS = -Wall -O2 -Wno-unused-result -Wno-deprecated-declarations \
	-I../../../HORS -I../../../SHA256 -I../..
LDFLAGS = -lmosquitto -lsodium -lcrypto -lm

SOURCES = publisherHors.c \
	../../Utils/IO.c \
	../../../HORS/keys.c \
	../../../SHA256/sha256.c
TARGET = publisherHors

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $(SOURCES) $(LDFLAGS)

clean:
	rm -f $(TARGET)

.PHONY: all clean
