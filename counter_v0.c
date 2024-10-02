#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include "shmSegment.h"

bool FOREVER = true;

//------------------------------------------------------------
/* Wrapper for sigaction */
typedef void Sigfunc(int);

Sigfunc *sigactionWrapper(int signo, Sigfunc *func) {
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;

    if (sigaction(signo, &act, &oact) < 0)
        return (SIG_ERR);

    return (oact.sa_handler);
}

// Shared memory pointer
shmData *shmPtr;

//------------------------------------------------------------
// SIGTSTP handler now forks and execs the divider
void sigHandler_A(int sig) {
    fflush(stdout);
    printf("\n\n### I (%d) received Signal #%3d and will fork a new process.\n\n", getpid(), sig);

    pid_t child_pid = fork();

    if (child_pid == 0) {
        // Child process
        execlp("./divider", "divider", NULL);
        perror("execlp failed"); // If execlp fails
        exit(1);
    }
}

//------------------------------------------------------------
// SIGCONT handler calculates and prints the ratio
void sigHandler_CONT(int sig) {
    fflush(stdout);
    printf("\n\n### I (%d) have been asked to RESUME by Signal #%3d.\n\n", getpid(), sig);

    if (shmPtr != NULL) {
        if (shmPtr->num2 != 0) {
            shmPtr->ratio = (double)shmPtr->num1 / shmPtr->num2;  // Calculate the ratio
            printf("Ratio from shared memory: %f\n", shmPtr->ratio);
        } else {
            printf("Error: Division by zero.\n");
        }
    }
    
    FOREVER = false;
}

//------------------------------------------------------------

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <num1> <num2>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Create shared memory
    key_t key = ftok("shmSegment.h", 'R'); // Use the provided header file
    int shmid = shmget(key, SHMEM_SIZE, IPC_CREAT | 0666);
    
    if (shmid < 0) {
        perror("shmget failed");
        return EXIT_FAILURE;
    }

    // Attach to the shared memory
    shmPtr = (shmData *)shmat(shmid, NULL, 0);
    if (shmPtr == (void *)-1) {
        perror("shmat failed");
        return EXIT_FAILURE;
    }

    // Set values from command-line arguments
    shmPtr->num1 = atoi(argv[1]);
    shmPtr->num2 = atoi(argv[2]);
    shmPtr->ratio = 0.0;  // Initialize ratio

    // Set up signal catching
    sigactionWrapper(SIGTSTP, sigHandler_A);
    sigactionWrapper(SIGCONT, sigHandler_CONT);

    unsigned i = 0;
    while (FOREVER)
        printf("%10X\r", i++);

    printf("\nCOUNTER: Stopped Counting. The FOREVER flag must have become FALSE\n\n");
    printf("\nCOUNTER: Goodbye\n\n");

    // Detach from shared memory and remove it
    shmdt(shmPtr);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
