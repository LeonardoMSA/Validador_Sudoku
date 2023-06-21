CC=gcc

TARGET=sudoku

OBJ=sudoku.o

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET)
	rm *.o

sudoku.o: sudoku.c
	$(CC) $(CFLAGS) -c sudoku.c