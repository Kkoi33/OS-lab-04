#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 1024
#define MAX_ARGS 64
#define MAX_PATH 4096

extern char **environ;

/* Splits line into whitespace-separated tokens; returns their count. */
static int parse(char *line, char *args[])
{
    int argc = 0;
    char *token = strtok(line, " \t\n");
    while (token != NULL && argc < MAX_ARGS - 1) {
        args[argc++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[argc] = NULL;
    return argc;
}

/* execve needs a full path, so look the command up in PATH ourselves. */
static void run(char *args[])
{
    if (strchr(args[0], '/') != NULL) {
        execve(args[0], args, environ);
    } else {
        const char *env_path = getenv("PATH");
        char *paths = strdup(env_path != NULL ? env_path : "/bin:/usr/bin");
        char full[MAX_PATH];
        for (char *dir = strtok(paths, ":"); dir != NULL; dir = strtok(NULL, ":")) {
            snprintf(full, sizeof(full), "%s/%s", dir, args[0]);
            execve(full, args, environ);
        }
        free(paths);
        errno = ENOENT;
    }
    fprintf(stderr, "%s: %s\n", args[0], strerror(errno));
    _exit(127);
}

/* Collects finished background jobs so they do not stay zombies. */
static void reap_background(void)
{
    int status;
    pid_t pid;
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        printf("[done] %d\n", pid);
}

int main(void)
{
    char line[MAX_LINE];
    char *args[MAX_ARGS];

    while (1) {
        reap_background();
        printf("myshell> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("\n");
            break;
        }

        int argc = parse(line, args);
        if (argc == 0)
            continue;

        if (strcmp(args[0], "exit") == 0)
            break;

        if (strcmp(args[0], "cd") == 0) {
            const char *dir = argc > 1 ? args[1] : getenv("HOME");
            if (dir == NULL || chdir(dir) != 0)
                perror("cd");
            continue;
        }

        /* a trailing "&" is handled by this shell itself: the command runs
           in a separate process and the shell does not wait for it */
        int background = 0;
        if (strcmp(args[argc - 1], "&") == 0) {
            background = 1;
            args[--argc] = NULL;
            if (argc == 0)
                continue;
        }

        pid_t pid = fork();
        if (pid < 0) {
            perror("fork");
            continue;
        }
        if (pid == 0)
            run(args);

        if (background)
            printf("[background] %d\n", pid);
        else
            waitpid(pid, NULL, 0);
    }

    while (wait(NULL) > 0)
        ;
    return EXIT_SUCCESS;
}
