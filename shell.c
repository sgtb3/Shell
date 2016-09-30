#include "shell.h"

int main(void)
{
        /* History declared as static  */
        static char *history[MAXHIST];
        static int top_entry = 0;
        const char *delim = " ";
        char **arg_v = NULL;
        char *hist_entry = NULL;
        char *line = NULL;
        int arg_c = 0;
        int i = 0;

        int fds[2];

        /* Initialize a NULL history vector */
        while (i < MAXHIST)
                history[i++] = '\0';

        for (; ;) {

                /* From previous iteration and print prompt to console */
                free_memory(line, arg_v);
                print_prompt();

                /* Read the next line */
                line = read_line(stdin);
                if (line == NULL)
                        kill("error: %s\n", "main: unable to read from stdin");

                /* Avoid dangling pointers from previous iteration */
                free(history[top_entry]);

                /* Handle new line before tokenizing*/
                if (!strcmp(line, "\n")) {
                        continue;
                }

                /* Save a copy of the line before get_argc tokenizes it */
                hist_entry = dup_entry(line);
                if (hist_entry == NULL) {
                        printf("error: main() unable to allocate memory\n");
                        continue;
                }

                /* Handle pipes */
                if (strchr(line, '|') != NULL) {
                        pipe_handler(fds, line);
                        continue;
                }

                /* Get the number of arguments and allocate memory */
                arg_c = get_argc(line, delim);
                arg_v = (char **) calloc((size_t) arg_c + 1, sizeof(char *));
                arg_v[arg_c] = "\0";
                if (arg_v == NULL) {
                        printf("error: main() unable to allocate memory\n");
                        free(hist_entry);
                        continue;
                }

                /* Set arg_v to point to each string arg */
                set_argv(line, arg_v, &arg_c, delim);

                /* Handle exit command */
                if (!strcmp(arg_v[0], "exit")) {
                        free(hist_entry);
                        break;
                }

                /* Else place entry into hist and increm loc of oldest entry */
                history[top_entry] = hist_entry;
                top_entry = get_remainder(top_entry);

                /* Handle commands that require a fork and exec */
                if (!strcmp(arg_v[0], "cd")) {
                        cd_handler(arg_v, &arg_c);
                } else if (!strcmp(arg_v[0], "history")) {
                        hist_handler(history, &top_entry,
                                     hist_entry, arg_v, &arg_c);
                } else {
                        fork_handler(line, arg_v);
                }
        }

        /* Free everything and exit */
        free_memory(line, arg_v);
        hist_clear(history);
        exit(EXIT_SUCCESS);
}

char* read_line(FILE *stream)
{
        char *line = NULL;
        size_t length = 0;
        int chars_read = (int) getline(&line, &length, stream);

        if (chars_read > 0)
                return line;
        if (errno == EINVAL)
                fprintf(stderr, "error: %s\n", strerror(errno));

        free(line);
        return NULL;
}

int get_argc(char *line, const char *delim)
{
        size_t length = strlen(line);

        /* copy original line since tokenizing it will modify it */
        char *copy = (char *) malloc(sizeof(char) * length + 1);
        if (copy == NULL)
                kill("error: %s\n", "get_argc() unable to allocate memory");

        char *result = strcpy(copy, line);
        if (strcmp(line, result) != 0) {
                printf("error: strcpy() failed to properly copy %s\n", line);
                return '\0';
        }

        int argc = 0;
        char *temp = strtok(copy, delim);
        while (temp != NULL) {
                temp = strtok(NULL, delim);
                argc++;
        }
        free(copy);
        return argc;
}

void replace_nl(char **arg_v)
{
        char *cp = arg_v[0];
        while (*cp != '\0') {
                if (*cp == '\n') {
                        *cp = '\0';
                        break;
                }
                cp++;
        }
}

void set_argv(char *line, char **arg_v, int *arg_c, const char *delim)
{
        int i = 0;
        char *tok = strtok(line, delim);
        if (*arg_c == 1) {
                arg_v[i] = tok;
                replace_nl(arg_v);
        }
        arg_v[i++] = tok;

        while (tok != NULL) {
                tok = strtok(NULL, delim);
                arg_v[i++] = tok;
        }
        /* replace any new line chars in final argument */
        if (*arg_c != 1)
                replace_nl(&arg_v[*arg_c-1]);
}

void kill(char *format, const char *error_msg)
{
        fprintf(stderr, format, error_msg);
        exit(EXIT_FAILURE);
}

void free_memory(char *line, char **arg_v)
{
        free(line);
        free(arg_v);
}

char *dup_entry(const char *entry)
{
        char* dup = (char *) malloc(strlen(entry)+1);
        if (dup == NULL) {
                fprintf(stderr, "error(): dup_entry failed\n");
                return NULL;
        }
        strcpy(dup, entry);
        return dup;
}

void print_prompt()
{
        printf("$");
        fflush(stdout);
}

int get_remainder(int index)
{
        return (index+1) % MAXHIST;
}

int is_all_digits(char *line)
{
        int i;
        for (i = 0; i < strlen(line); i++) {
                if (!isdigit(line[i]))
                        return 0;
        }
        return 1;
}

void cd_handler(char **arg_v, int *arg_c)
{
        if (*arg_c != 2 || arg_v[1] == NULL)
                printf("error: cd requires two non-null arguments\n");
        else if (chdir(arg_v[1]) != 0)
                fprintf(stderr, "error: %s\n", strerror(errno));
        printf("$");
}

void fork_handler(char *line, char **arg_v)
{
        pid_t pid = fork();
        if (pid < 0) {
                free_memory(line, arg_v);
                kill("error: %s\n", strerror(errno));
        }
        if (pid != 0) {
                wait(0);
        } else {
                execv(arg_v[0], arg_v + 1);
                free_memory(line, arg_v);
                kill("error: %s\n", strerror(errno));
        }
}

void hist_clear(char **hist)
{
        int i = 0;
        while (i < MAXHIST) {
                free(hist[i]);
                hist[i] = NULL; /* Avoid dangling pointer */
                i++;
        }
}

void hist_print(char **hist, int first)
{
        int start_idx = first;/* Save the starting index */
        int curr_idx = 1;     /* History starts at 1 */
        while (1) {
                if (hist[start_idx] != NULL) {
                        printf("%d %s", curr_idx, hist[start_idx]);
                        curr_idx++;
                }
                /* Find remainder */
                if ((start_idx = get_remainder(start_idx)) == first)
                        break;
        }
}

void hist_handler(char **hist, int *first_entry, char *hist_entry,
                  char **arg_v, int *arg_c) {

        /* Print all history including last 'history' command */
        if (*arg_c == 1) {
                hist_print(hist, *first_entry);
                return;
        }

        /* Else exactly two non-null arguments required */
        if (*arg_c != 2 || arg_v[1] == NULL) {
                fprintf(stderr, "error(): invalid history arguments\n");
                return;
        }

        /* Needed for strchr() */
        if (arg_v[1][strlen(arg_v[1])] != '\0')
                arg_v[1][strlen(arg_v[1])] = '\0';

        char *opt = strchr(arg_v[1], '-');
        int num_val;

        /* Otherwise parse the second argument */
        if (opt == NULL && is_all_digits(arg_v[1])) {
                    num_val = atoi(arg_v[1]);
                    if (num_val > 0 && num_val <= MAXHIST){
                            printf("valid history number option "
                                           "executing : (%s)\n", arg_v[1]);
                            fork_handler(hist[num_val], arg_v);
                            return;
                    }
                
        } else if (!strcmp(opt + 1, "c")) {
                hist_clear(hist);
                return;
        } else {
                fprintf(stderr, "error(): invalid history argument\n");
        }

        /* Everything else is errors */
        fprintf(stderr, "error(): invalid history argument\n");
}