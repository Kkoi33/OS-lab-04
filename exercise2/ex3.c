#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <n>\n", argv[0]);
        return EXIT_FAILURE;
    }

    int n = atoi(argv[1]);
    if (n < 0) {
        fprintf(stderr, "n must be non-negative\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < n; i++) {
        if (fork() < 0) {
            perror("fork");
            return EXIT_FAILURE;
        }
        sleep(5);
    }

    /* wait for own children so that no zombies or orphans are left */
    while (wait(NULL) > 0)
        ;

    return EXIT_SUCCESS;
}
