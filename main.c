// Make sure we have getline
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
    /* lsh            will read commands from the keyboard
       lsh script     reads them from a file instead       */
    FILE *in = stdin;
    if (argc > 1) {
        in = fopen(argv[1], "r");
        if (!in) { perror(argv[1]); return 1; }
    }
    int interactive = isatty(fileno(in));    /* only prompt for a person */

    char *line = NULL;
    size_t cap = 0;

    for (;;) {
        if (interactive) {
            printf("lsh$ ");
            fflush(stdout);
        }
        if (getline(&line, &cap, in) < 0){
            break;
        }

        // This is as lame a lexing/parsing chunk as it gets.  Won't handle quotes
        // or much anything else but spaces.  But, it illustrates what we want.
        // (For now.)
        size_t n = 0, max = 8;
        char **words = malloc(max * sizeof *words);
        for (char *w = strtok(line, " \t\n"); w; w = strtok(NULL, " \t\n")) {
            if (n + 1 >= max){
                words = realloc(words, (max *= 2) * sizeof *words);
            }
            words[n++] = w;
        }
        words[n] = NULL;

        if (n == 0) {
            free(words);
            continue;
        }
        if (strcmp(words[0], "exit") == 0) {
            free(words);
            break;
        }
        if (strcmp(words[0], "cd") == 0) {
            if (chdir(n > 1 ? words[1] : getenv("HOME")) < 0){
                perror("cd");
            }
            free(words);
            continue;
        }

        pid_t pid = fork();
        if (pid == 0) {
            execvp(words[0], words);
            perror(words[0]);
            exit(127);
        }
        int status;
        waitpid(pid, &status, 0);
        free(words);
    }
    return 0;
}
