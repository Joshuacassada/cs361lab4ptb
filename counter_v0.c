#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include "shmSegment.h"

bool FOREVER = true;

// Wrapper for sigaction
typedef void Sigfunc(int);

Sigfunc * sigactionWrapper(int signo, Sigfunc *func)
{
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (sigaction(signo, &act, &oact) < 0)
        return (SIG_ERR);
    return (oact.sa_handler);
}

void sigHandler_A(int sig)
{
    printf("\n\n### I (%d) have been nicely asked to PAUSE. by #%3d.\n\n", getpid(), sig);
    pid_t child_process = fork();
    if (child_process == 0) {
        execlp("./divider", "divider", NULL);
        perror("execlp failed");
        exit(1);
    }
    // Send SIGSTOP to itself
    kill(getpid(), SIGSTOP);
}

void sigHandler_CONT(int sig)
{
    printf("\n\n### I (%d) have been asked to RESUME by Signal #%3d.\n\n", getpid(), sig);
    FOREVER = false;
}

int main(int argc, char *argv[])
{
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num1> <num2>\n", argv[0]);
        exit(1);
    }

    pid_t mypid = getpid();
    printf("\nHELLO! I AM THE COUNTER PROCESS WITH ID= %d\n", mypid);

    // Set up Signal Catching
    sigactionWrapper(SIGTSTP, sigHandler_A);
    sigactionWrapper(SIGCONT, sigHandler_CONT);

    // Create shared memory
    key_t key = ftok("shmSegment.h", 'R');
    int shmid = shmget(key, SHMEM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach to shared memory
    shmData *shm_ptr = (shmData *)shmat(shmid, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Set num1 and num2 in shared memory
    shm_ptr->num1 = atoi(argv[1]);
    shm_ptr->num2 = atoi(argv[2]);

    unsigned i = 0;
    while (FOREVER) {
        printf("%10X\r", i++);  // Print at least 7 digits in hexadecimal
        fflush(stdout);
    }

    printf("\nCOUNTER: Stopped Counting. The FOREVER flag must have become FALSE\n\n");
    printf("COUNTER: Found this ratio in the shared memory: %f\n", shm_ptr->ratio);
    printf("\nCOUNTER: Goodbye\n\n");

    // Detach from shared memory
    shmdt(shm_ptr);

    return 0;
}
