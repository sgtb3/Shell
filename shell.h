#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#ifndef _SHELL_H_
#define _SHELL_H_

#define MAXHIST 100

/**
 * read_line() - A wrapper for the GNU call to getline().
 * @arg: The file stream to read from.
 *
 * Calls getline to read the next line from stream. The line
 * pointer and length variable are set to NULL and 0,
 * respectively, allowing getline to allocate the buffer and
 * buffer size.
 *
 * Return: On success, a char pointer of the line read from stream
 *         is returned. The caller must ensure line is freed.
 *         If getline fails for any reason, the errno value
 *         is printed to standard error and a a pointer to NULL
 *         is returned and line is freed.
 */
char *read_line(FILE *stream);


/**
 * dup_entry() - Duplicates a string by allocating heap mem.
 * @arg entry : The string entry to be cloned.
 *
 * Caller must deallocate memory.
 *
 * Return: A heap allocated clone of entry.
 */
char *dup_entry(const char *entry);

/**
 * is_all_digits() - Checks if a string is all numerical digits.
 * @arg line : The string to be parsed
 *
 * Return: 1 if line contains all digits, 0 otherwise.
 */
int is_all_digits(char *line);

/**
 * get_argc() - Parses the line, counting the number of arguments.
 * @arg1 line  : The line to be parsed.
 * @arg2 delim : The line delimiter.
 *
 * Uses strtok to tokenize the line and thus needs a separate copy
 * allocated before the call to strtok. Keeps a running count on the
 * number of tokens.
 *
 * Return: The int token/argument count.
 */
int get_argc(char *line, const char *delim);

/**
 * get_remainder() - Returns the remainder after performing
 *                   division modulus MAXHIST.
 * @arg index : The history index.
 *
 * Return: The reminder.
 */
int get_remainder(int index);

/**
 * print_prompt() - Prints the prompt and flushes the buffer.
 */
void print_prompt();

/**
 * cd_handler() - Changes the present directory to dir.
 * @arg1 arg_v : The argument vector, with the 2nd arg being the new directory.
 * @arg2 arg_c : The argument count.
 *
 */
void cd_handler(char **arg_v, int *arg_c);

/**
 * free_memory() - Releases the allocated memory.
 * @arg1 line  : The line read from console.
 * @arg2 arg_v : The tokenized argument vector.
 *
 */
void free_memory(char *line, char **arg_v);

/**
 * kill() - Kills the program.
 * @arg error_msg : The message to be printed.
 *
 * Calls fprintf to print error_msg to standard error, followed
 * by a call to exit with an exit_code value of 1 indicating
 * abnormal termination.
 *
 */
void kill(char *format, const char *error_msg);

/**
 * set_argv() - Parses arguments from line into provided arg vector.
 * @arg1 arg_count : The arg count.
 * @arg2 line      : The line to be parsed.
 * @arg3 argv      : The array buffer to hold the (char *) pointers.
 * @arg4 delim     : The line delimiter.
 *
 * The line argument is modified by strtok and the resulting arg pointers
 * are stored in the argument vector.
 *
 */
void set_argv(char *line, char **arg_v, int *arg_c, const char *delim);

/**
 * replace_nl() - Searches for and replace new line char with NULL.
 * @arg arg_v : The string to be searched.
 *
 */
void replace_nl(char **arg_v);

/**
 * handle_fork() - Handles the forking process.
 * @arg1 arg_v : The argument vector.
 * @arg2 line  : The string containing the command and arguments to
 *               be executed.
 *
 */
void fork_handler(char *line, char **arg_v);

/**
 * clear_history() - Clears the history vector by freeing pointer value.
 * @arg1 hist  : A reference to history vector.
 * @arg2 count : The current number of items in history.
 *
 */
void hist_clear(char **hist);

/**
 * hist_print() - Prints the entire contents of the history.
 * @arg1 hist  : A reference to the history vector.
 * @arg2 first : The top most history entry.
 *
 */
void hist_print(char **hist, int first);

/**
 * print_history() - Displays history based on parameters provided.
 * @arg1 hist     : A reference to the history vector.
 * @arg2 hist_idx : A reference to the current history count.
 * @arg3 arg_v    : A reference to the argument vector.
 * @arg4 arg_c    : A reference to the argument count.
 *
 */
void hist_handler(char **hist, int *first_entry, char *hist_entry,
                  char **arg_v, int *arg_c);

/**
 * Future implementation.
 * pipe_handler() - Handles the actions to create a pipe
 * for redirecting input/output between two processes.
 * @arg2 fds[] : An array of size 2 to hold the file descriptors.
 * @arg2 line  : The command.
 *
 * Parses the line, set the file descriptors and them forks.
 *
 */
/* void pipe_handler(int *fds, char *line); */

#endif