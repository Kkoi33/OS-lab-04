#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

static double elapsed_ms(clock_t start)
{
    return (double)(clock() - start) * 1000.0 / CLOCKS_PER_SEC;
}

int main(void)
{
    clock_t start = clock();

    pid_t first = fork();
    if (first < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (first == 0) {
        /* first child: timer starts at the first instruction after fork */
        clock_t child_start = clock();
        printf("Child 1: PID = %d, PPID = %d\n", getpid(), getppid());
        printf("Child 1: execution time = %.3f ms\n", elapsed_ms(child_start));
        return EXIT_SUCCESS;
    }

    /* only the main process reaches this point, so both children share the same parent */
    pid_t second = fork();
    if (second < 0) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (second == 0) {
        clock_t child_start = clock();
        printf("Child 2: PID = %d, PPID = %d\n", getpid(), getppid());
        printf("Child 2: execution time = %.3f ms\n", elapsed_ms(child_start));
        return EXIT_SUCCESS;
    }

    waitpid(first, NULL, 0);
    waitpid(second, NULL, 0);

    printf("Main: PID = %d, PPID = %d\n", getpid(), getppid());
    printf("Main: execution time = %.3f ms\n", elapsed_ms(start));
    return EXIT_SUCCESS;
}
