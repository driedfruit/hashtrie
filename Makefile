CC=gcc

CFLAGS=-g -Wall -DDEBUG_HASH
LFLAGS=

SOURCES=hash.c pushword.c
EXECUTABLE=pushword

SOURCES2=hash.c test.c
EXECUTABLE2=test

SOURCES3=hash.c stash.c stash-test.c
EXECUTABLE3=stash-test

SOURCES4=hash.c stash.c pushword-s.c
EXECUTABLE4=pushword-s


OBJECTS=$(SOURCES:.c=.o)
OBJECTS2=$(SOURCES2:.c=.o)
OBJECTS3=$(SOURCES3:.c=.o)
OBJECTS4=$(SOURCES4:.c=.o)

all: $(EXECUTABLE) $(EXECUTABLE2) $(EXECUTABLE3) $(EXECUTABLE4)
	@true

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LFLAGS) $(OBJECTS) -o $@

$(EXECUTABLE2): $(OBJECTS2)
	$(CC) $(LFLAGS) $(OBJECTS2) -o $@

$(EXECUTABLE3): $(OBJECTS3)
	$(CC) $(LFLAGS) $(OBJECTS3) -o $@

$(EXECUTABLE4): $(OBJECTS4)
	$(CC) $(LFLAGS) $(OBJECTS4) -o $@

.c.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -rf *.o *.a $(EXECUTABLE) $(EXECUTABLE2) $(EXECUTABLE3) $(EXECUTABLE4)