shell_sh: shell.o shell.h
	gcc -g -Wall shell.o -o shell_sh

binTest: binTest.o
	gcc -g -Wall binTest.o -o binTest

.PHONY: test
test:
	valgrind --leak-check=yes ./shell_sh

.PHONY: fulltest
fulltest:
	valgrind -v --leak-check=full --show-leak-kinds=all ./shell_sh

.PHONY: clean
clean:
	rm -f *.o a.out core shell_sh binTest

.PHONY: all
all: clean shell_sh binTest 

