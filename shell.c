#include "shell.h"

int main(void)
{
    /* history buffer */
    static char *history[MAXHIST] = { 0 };
    static int top_idx = 0;

    char *hist_entry = NULL;
    char *line = NULL;
    int prev_cmd_idx = -1;
    int i = 0;

    for (; ;) {

        printf("\n$ ");

        /* avoid dangling pointers from previous iteration */
        free(history[top_idx]);
        history[top_idx] = NULL;

        /* read the next line */
        line = read_line(stdin);
        if (line == NULL)
            err_kill("error: %s\n", "main: unable to read from stdin");

        /* continue if new line */
        if (!strcmp(line, "\0"))
            goto free;

        /* handle exit */
        if (!strcmp(line, "exit")) {
            free(line);
            break;
        }

        /* save a copy of the line for history before tokenizing it */
        hist_entry = dup_entry(line);
        if (hist_entry == NULL)
            goto free;

        /* place entry into hist and increm loc of oldest entry */
        history[top_idx] = hist_entry;
        top_idx = (top_idx + 1) % MAXHIST;

        /* handle commands */
        execute_cmds(line, history, hist_entry, &top_idx, &prev_cmd_idx);

    free:
        free(line);
    }

    /* free history and exit */
    clear_history(history);
    return 0;
}

int execute_cmds(char *line, char **history, char *hist_entry, int *top_idx,
                 int *prev_cmd_idx)
{
    char *toks[_POSIX_ARG_MAX] = { 0 };
    char *line_copy = NULL;
    int pipe_count = 0;
    int arg_c = 0;

    /* save a copy of input line before tokenizing it */
    line_copy = dup_entry(line);
    arg_c = tokenize_line(line, toks, " \t");

    /* handle history */
    if (!strcmp(toks[0], "history")) {
        history_handler(history, top_idx, hist_entry, line_copy,
                        toks, arg_c, prev_cmd_idx);
        goto free;
    }

    /* handle pipes */
    pipe_count = get_pipe_count(line_copy);
    if (pipe_count) {
        pipe_handler(line_copy, toks, arg_c);
        goto free;
    }

    /* handle change directory */
    if (!strcmp(toks[0], "cd")) {
        cd_handler(toks, arg_c);
        goto free;
    }

    /* fork and exec all other commands */
    if (fork_handler(line_copy, -1, -1)) {
        free(line_copy);
        return 1;
    }

free:
    free(line_copy);
    return 0;
}

void clear_history(char **history)
{
    int i = 0;
    while (i < MAXHIST) {
        if (history[i] != NULL) {
            free(history[i]);
            history[i] = NULL;     /* avoid dangling pointer */
        }
        i++;
    }
}

void print_history(char **history, int top_idx)
{
    int start_idx = top_idx;       /* save the starting index */
    int curr_idx = 0;              /* history starts at 0 */

    while (1) {
        if (history[start_idx] != NULL) {
            printf("%d %s\n", curr_idx, history[start_idx]);
            curr_idx++;
        }
        /* find remainder */
        if ((start_idx = (start_idx + 1) % MAXHIST) == top_idx)
            break;
    }
}

void history_handler(char **history, int *top_idx, char *hist_entry,
                     char *line, char **arg_v, int arg_c, int *prev_cmd_idx)
{
    /* print all history including last 'history' command */
    if (arg_c == 1) {
        if (*prev_cmd_idx != -1)
            free(hist_entry);
        print_history(history, *top_idx);
        return;
    }

    /* else exactly two non-null arguments required */
    if (arg_c != 2)
        goto usage;

    /* check for clear option */
    if (!strcmp(arg_v[1], "-c")) {
        clear_history(history);
        /* reset back to 0 */
        *top_idx = 0;
        return;
    }

    /* check if history index is all digits */
    if (check_numerical(arg_v[1]))
        goto usage;

    /* check if index is within valid range */
    int index = atoi(arg_v[1]);

    if (index < 0 || index >= MAXHIST || index > *top_idx)
        goto usage;

    /* else re-execute previous command from history */
    char *prev = dup_entry(history[index]);
    if (!prev) {
        free(prev);
        return;
    }
    execute_cmds(prev, history, hist_entry, top_idx, prev_cmd_idx);

    /* reset index */
    *prev_cmd_idx = -1;

    free(prev);
    return;

usage:
    printf(HIST_USAGE);
}

void cd_handler(char **arg_v, int arg_c)
{
    if (arg_c != 2 || arg_v[1] == NULL)
        printf("error: cd requires two non-null arguments\n");
    else if (chdir(arg_v[1]) != 0)
        PRINT_ERRNO
}

int fork_handler(char *line, int readfd, int writefd)
{
    char *buff[_POSIX_ARG_MAX];
    pid_t pid;

    if (tokenize_line(line, buff, " \t") < 0)
        return 1;

    if ((pid = fork()) < 0)
        goto error;

    /* parent waits for child */
    if (pid > 0) {
        wait(0);
        return 0;
    }

    /* else child - duplicate fds */
    if ((readfd != -1) && (dup2(readfd, STDIN_FILENO) < 0))
        goto error;
    if ((writefd != -1) && (dup2(writefd, STDOUT_FILENO) < 0))
        goto error;

    execv(buff[0], buff);
    /* execv returns only if it failed */
    err_kill("error: %s\n", strerror(errno));

error:
    PRINT_ERRNO
    return 1;
}

void pipe_handler(char *line, char **line_toks, int arg_c)
{
    char *cmds[_POSIX_CHILD_MAX];
    int exit_status = 0;
    int writefd = -1;
    int readfd = -1;
    int pipes[2];
    int i;

    if (line_toks == '\0')
        return;

    if (*line_toks[0] == '|' || *line_toks[arg_c-1] == '|') {
        fprintf(stderr, "error: improper pipe usage\n");
        goto close;
    }

    for (i = 0; i < arg_c - 1; i++) {
        if (*line_toks[i] == '|' && *line_toks[i+1] == '|') {
            fprintf(stderr, "error: improper pipe usage\n");
            goto close;
        }
        if (!line_toks[i])
            break;
    }

    /* retokenize the line around all pipe symbols */
    tokenize_line(line, cmds, "|");

    int c = 0;
    char *cmd = cmds[c];

    /* link each cmd */
    while (cmd != NULL) {

        if (c == 0)
            readfd = -1;        /* input end of pipe reads */
        else
            readfd = pipes[0];  /* else cmd's stdin == stdout of prev cmd */

        if (cmds[c+1] == NULL) {
            /* output end of pipe writes */
            writefd = -1;
        } else {
            /* not last cmd; collect stdout in new pipe for next cmd to read */
            if (pipe(pipes) < 0) {
                PRINT_ERRNO
                goto close;
            }
            writefd = pipes[1];
        }

        /* fork and exec piped commands */
        if (fork_handler(cmd, readfd, writefd))
            goto close;

        /*
         * close write fd to send EOF to read end of pipe, else processes will
         * wait assuming more input is incoming.
         */
        if (readfd != -1) {
            close(readfd);
            readfd = -1;
        }
        if (writefd != -1) {
            close(writefd);
            writefd = -1;
        }
        cmd = cmds[++c];
    }

close:
    if (readfd != -1)
        close(readfd);

    if (writefd != -1)
        close(writefd);

    /* wait until children exit */
    while (1) {
        if (wait(&exit_status) == -1) {
            /* break if no remaining children to wait for, else error */
            if (errno == ECHILD)
                break;
            PRINT_ERRNO
        }
    }
}

int get_pipe_count(char *line)
{
    char *temp = dup_entry(line);
    if (!temp) {
        return 0;
    }
    char *tok_buff[_POSIX_CHILD_MAX];

    /* returns number of commands */
    int pipes = tokenize_line(temp, tok_buff, "|");
    free(temp);

    /* return 1 less than number of commands */
    return pipes-1;
}

int tokenize_line(char *line, char **arg_v, char *delm)
{
    int i = 0;
    arg_v[i] = strtok(line, delm);
    while (arg_v[i]) {
        if (++i >= _POSIX_ARG_MAX) {
            fprintf(stderr, "error: token count > than max allowable\n");
            return -1;
        }
        arg_v[i] = strtok(NULL, delm);
    }
    return i;
}

char* read_line(FILE *stream)
{
    char *line = NULL;
    size_t length = 0;

    int chars_read = (int) getline(&line, &length, stream);
    if (chars_read > 0 && line[0]) {
        line[chars_read-1] = '\0';
        return line;
    }

    if (errno == EINVAL)
        PRINT_ERRNO

    free(line);
    return NULL;
}

char *dup_entry(const char *entry)
{
    if (!entry)
        return NULL;

    char *dup = (char *) malloc(strlen(entry) + 1);
    if (dup == NULL) {
        fprintf(stderr, "error(): dup_entry failed\n");
        return NULL;
    }
    strcpy(dup, entry);
    return dup;
}

int check_numerical(char *str)
{
    int i;
    for (i = 0; i < strlen(str); i++) {
        if (!isdigit(str[i]))
            return 1;
    }
    return 0;
}

void err_kill(char *format, const char *error_msg)
{
    fprintf(stderr, format, error_msg);
    exit(EXIT_FAILURE);
}
