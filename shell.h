#ifndef _SHELL_H_
#define _SHELL_H_

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <limits.h>

#define MAXHIST 100
#define PRINT_ERRNO fprintf(stderr, "error: %s\n", strerror(errno));
#define HIST_USAGE  "\nUsage: \
					\n\thistory    : display up to 100 previous entries (0-99) \
					\n\thistory -c : clear history \
            		\n\thistory n  : execute command at index n \n"

/**
 * execute_cmds() - Calls the appropirate command handler.
 * @arg1 line         : Input line.
 * @arg2 history      : History buffer.
 * @arg3 hist_entry   : Current history entry.
 * @arg4 top_idx      : Current topmost history index.
 * @arg5 prev_cmd_idx : Buffer containing index of a previous command in
 * 						history to be executed again. -1 otherwise.
 * 
 * Return: 0 on success, 1 otherwise.
 */
int execute_cmds(char *line, char **history, char *hist_entry, int *top_idx, 
				 int *prev_cmd_idx);

/**
 * clear_history() - Clears the history vector by freeing pointer value.
 * @arg hist : History buffer.
 */
void clear_history(char **history);

/**
 * print_history() - Prints the all history entries.
 * @arg1 hist    : History buffer.
 * @arg2 top_idx : Topmost history entry.
 */
void print_history(char **history, int top_idx);

/**
 * history_handler() - Displays history based on parameters provided.
 * @arg1 history      : History buffer.
 * @arg2 first_entry  : Index of top history entry.
 * @arg3 hist_entry   : Current history entry.
 * @arg4 line         : Current input line.
 * @arg5 arg_c        : Argument count.
 * @arg6 prev_cmd_idx : Buffer containing the prev command in history to be 
 * 						executed, or -1.
 */
void history_handler(char **history, int *first_entry, char *hist_entry, 
				     char *line, char **, int arg_c, int *prev_cmd_idx);

/**
 * cd_handler() - A wrapper for the chdir call to change the cwd.
 * @arg1 arg_v : Argument vector with the 2nd arg being the new directory.
 * @arg2 arg_c : Argument count.
 */
void cd_handler(char **arg_v, int arg_c);

/**
 * fork_handler() - Handles the forking and execution of a process, and the 
 * duplication of file descriptors if necessary.
 * @arg1 line    : Current input line.
 * @arg2 readfd  : Read file descriptor, or -1.
 * @arg3 writefd : Write file descriptor, or -1.
 * 
 * Return: 0 on success, 1 otherwise.
 */
int fork_handler(char *line, int readfd, int writefd);

/**
 * pipe_handler() - Handles the actions to create a pipe for redirecting 
 * input/output between multiple command processes.
 * @arg1 line  : Current input line.
 * @arg2 arg_v : Tokenized argument vector.
 * @arg3 arg_c : Argument count.
 * 
 * Tokenizes the line around any pipes, sets the file descriptors, and calls
 * the fork handler.
 */
void pipe_handler(char *line, char **arg_v, int arg_c);

/**
 * get_pipe_count() - Returns the number of pipes in line.
 * @arg1 line : Currnent input line.
 * 
 * Return: Number of pipes in line.
 */
int get_pipe_count(char *line);

/**
 * tokenize_line() - Tokenizes str and saves tokens in arg_v.
 * @arg1 line  : Line to be tokenized.
 * @arg2 arg_v : Buffer to hold the tokens.
 * @arg3 delm  : Delimiters.
 * 
 * Return: The number of tokens on success, -1 otherwise.
 */
int tokenize_line(char *line, char **arg_v, char *delm);

/**
 * read_line() - A wrapper for the call to getline() to read the next line
 * from file stream. Called must deallocate memory.
 * @arg stream : The file stream to read from.
 */
char *read_line(FILE *stream);

/**
 * dup_entry() - Duplicates a string by allocating heap memory. Caller must 
 * deallocate memory.
 * @arg entry : String to be cloned.
 * 
 * Return: A heap allocated clone of entry, else NULL.
 */
char *dup_entry(const char *entry);

/**
 * check_numerical() - Checks if a string is all numerical digits.
 * @arg str : String to be parsed.
 *
 * Return: 0 if line contains all digits, 1 otherwise.
 */
int check_numerical(char *str);

/**
 * err_kill() - Kills the program and displays error_msg to stderr.
 * @arg error_msg : The message to be printed.
 */
void err_kill(char *format, const char *error_msg);

#endif