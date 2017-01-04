
###Shell
A small C program written to learn the inner workings of the shell. Tested with 
`valgrind` for memory leaks on Ubuntu Linux 16.04.01.

###Usage
`make`
`./shell_sh`

###Contents

#####Makefile
* `make` will create the `shell_sh` binary.
* `make clean` will clean all auxiliary files.
* `make test` will test the binary with the `testinput.txt` file.
* `make valtest` will test for memory leaks using `valgrind`.

#####testinput.txt
* Sample commands to test the `shell_sh` binary.

#####shell.c
* The main executable.
* Built-in functionality for displaying and saving command history, 
changing directories, and piping commands.

#####shell.h
* Header file with documentation.

