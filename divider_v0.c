#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include "shmSegment.h"

int main()
{
    pid_t mypid = getpid();
    printf("\n\nHELLO! I am the newly-born DIVIDER with PID= %d\n", mypid);

    // Find shared memory region
    key_t key = ftok("shmSegment.h", 'R');
    int shmid = shmget(key, SHMEM_SIZE, 0666);
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

    // Compute ratio
    if (shm_ptr->num2 != 0) {
        shm_ptr->ratio = (double)shm_ptr->num1 / shm_ptr->num2;
    } else {
        shm_ptr->ratio = 0;  // or handle division by zero as appropriate
    }

    printf("\nDIVIDER: Dividing    %d by   %d\n", shm_ptr->num1, shm_ptr->num2);
    printf("\nDIVIDER: will Signal my parent to CONTINUE");

    // Detach from shared memory
    shmdt(shm_ptr);

    // Awaken my Parent
    pid_t parent = getppid();
    kill(parent, SIGCONT);

    return 0;
}