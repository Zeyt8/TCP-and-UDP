PROJECT=server
PROJECT2=subscriber
SOURCES=server.c queue.c list.c ue_vector.c
SOURCES2=subscriber.c
LIBRARY=nope
INCPATHS=
LIBPATHS=.
LDFLAGS=
CFLAGS=-c -Wall
CC=gcc

# Automatic generation of some important lists
OBJECTS=$(SOURCES:.c=.o)
OBJECTS2=$(SOURCES2:.c=.o)
INCFLAGS=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS=$(foreach TMP,$(LIBPATHS),-L$(TMP))

# Set up the output file names for the different output types
BINARY=$(PROJECT)
BINARY2=$(PROJECT2)

all: $(SOURCES) $(BINARY) $(SOURCES2) $(BINARY2)

$(BINARY): $(OBJECTS)
	$(CC) $(LIBFLAGS) $(OBJECTS) $(LDFLAGS) -o $@

$(BINARY2): $(OBJECTS2)
	$(CC) $(LIBFLAGS) $(OBJECTS2) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(INCFLAGS) $(CFLAGS) -fPIC $< -o $@

distclean: clean
	rm -f $(BINARY) $(BINARY2)

clean:
	rm -f $(OBJECTS) $(OBJECTS2)