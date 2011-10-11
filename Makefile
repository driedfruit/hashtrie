CC=gcc

CFLAGS=-g
LFLAGS=

SOURCES=hash.c pushword.c
EXECUTABLE=pushword

SOURCES2=hash.c test.c
EXECUTABLE2=test

OBJECTS=$(SOURCES:.c=.o)
OBJECTS2=$(SOURCES2:.c=.o)

all: $(SOURCES) $(EXECUTABLE) $(EXECUTABLE2)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@

$(EXECUTABLE2): $(OBJECTS2)
	$(CC) $(LFLAGS) $(OBJECTS2) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *.a $(EXECUTABLE) 