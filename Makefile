CC=g++
CDEFINES=
SRCDIR=src
TPDIR=third_party
SOURCES=$(SRCDIR)/Dispatcher.cpp $(SRCDIR)/Mode.cpp $(SRCDIR)/precomp.cpp $(SRCDIR)/profanity.cpp $(SRCDIR)/SpeedSample.cpp $(TPDIR)/uECC.c
OBJECTS=$(patsubst %.cpp,%.o,$(patsubst %.c,%.o,$(SOURCES)))
EXECUTABLE=profanity.x64

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LDFLAGS=-framework OpenCL
	CFLAGS=-std=c++17 -Wall -mmmx -O3
	CFLAGS_C=-std=c11 -Wall -O3
else
	LDFLAGS=-s -lOpenCL -mcmodel=large
	CFLAGS=-std=c++17 -Wall -mmmx -O3 -mcmodel=large
	CFLAGS_C=-std=c11 -Wall -O3 -mcmodel=large
endif

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

$(SRCDIR)/%.o: $(SRCDIR)/%.cpp
	$(CC) -c $(CFLAGS) $(CDEFINES) $< -o $@

$(TPDIR)/%.o: $(TPDIR)/%.c
	gcc -c $(CFLAGS_C) $< -o $@

clean:
	rm -rf $(SRCDIR)/*.o $(TPDIR)/*.o $(EXECUTABLE)
