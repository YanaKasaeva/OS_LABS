CC = gcc
CFLAGS = -Wall -Wextra -O2 -std=c99

TARGET = run
OBJ = main.o freelist.o buddy.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)

main.o: main.c freelist.h buddy.h allocator.h
	$(CC) $(CFLAGS) -c main.c

freelist.o: freelist.c freelist.h allocator.h
	$(CC) $(CFLAGS) -c freelist.c

buddy.o: buddy.c buddy.h allocator.h
	$(CC) $(CFLAGS) -c buddy.c

run_program: $(TARGET)
	./$(TARGET)

clean:
	rm -f *.o $(TARGET)