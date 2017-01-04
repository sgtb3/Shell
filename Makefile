.PHONY: all clean test valtest

TESTFILE = testinput.txt
EXE = shell_sh

shell: shell.o shell.h
	gcc -g -Wall -Werror shell.o -o $(EXE)

test: all
	./$(EXE) < testinput.txt

valtest: all
	valgrind --leak-check=yes ./$(EXE) < $(TESTFILE)

clean:
	rm -f *.o a.out $(EXE)

all: clean shell 

