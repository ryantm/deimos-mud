
# C compiler to use
CC = gcc

# Any special flags you want to pass to the compiler
MYFLAGS = -Wall

OBJFILES = test.o
LIBS = -lcrypt

CFLAGS = -ggdb -O3 $(MYFLAGS)

all: $(OBJFILES) 
	$(CC) -o test $(OBJFILES) $(LIBS)
