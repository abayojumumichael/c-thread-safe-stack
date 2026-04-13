CC = clang
RM = rm -f
DFLAG = -g
GFLAGS = -Wall -Wextra
CFLAGS = $(DFLAG) $(GFLAGS) -c
LFLAGS = $(DFLAG) $(GFLAGS)
LIBFLAGS = -pthread

all: TestStack TestBlockingStack

TestStack: TestStack.o Stack.o 
	$(CC) $(LFLAGS) TestStack.o Stack.o -o TestStack $(LIBFLAGS)

TestBlockingStack: TestBlockingStack.o BlockingStack.o Stack.o
	$(CC) $(LFLAGS) TestBlockingStack.o BlockingStack.o Stack.o -o TestBlockingStack $(LIBFLAGS)


Stack.o: Stack.c Stack.h
	$(CC) $(CFLAGS) Stack.c

TestStack.o: TestStack.c myassert.h
	$(CC) $(CFLAGS) TestStack.c

BlockingStack.o: BlockingStack.c BlockingStack.h
	$(CC) $(CFLAGS) BlockingStack.c

TestBlockingStack.o: TestBlockingStack.c myassert.h
	$(CC) $(CFLAGS) TestBlockingStack.c

clean:
	$(RM) TestStack TestBlockingStack *.o
