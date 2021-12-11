#include "instr.h"

int main(int argc, char** argv)
{
    key_t key = ftok(KEY_PATH, KEY_ID);
    if (key == -1)
        PERROR ("Key was not generated.\n")

    int semid = semget(key, N_SEM, PERM_FLAG);
    if (semid == -1)
        PERROR("Semaphore set was not created.\n")

    int shmid = shmget(key, PAGE_SZ, PERM_FLAG);
    if (shmid == -1)
        PERROR ("Share memory did not get.\n")

    char* memp = (char*) shmat(shmid, NULL, 0);
    if (memp == NULL)
        PERROR ("Shared Memory was not attached.\n")

    char* buffer = (char*) calloc(PAGE_SZ, 1);
    if (buffer == NULL)
        PERROR ("Buffer was not allocated.\n");

    if (semop (semid, block_r, 2) == -1)
        PERROR ("cannot block reader.\n")

    if (semctl (semid, FULL, SETVAL, 0) == -1)
        PERROR ("Cannot set full to 0.\n")

    if (semctl (semid, EMPTY, SETVAL, 1) == -1)
        PERROR ("Cannot set empty to 1.\n")

    if (semop (semid, connect_r, 5) == -1)
        PERROR ("Impossible to find writer.\n")

    //exit(0);

    while (true)
    {
        if (semop (semid, full_dw, 3) == -1)   // checks if shm is full
            PERROR ("Impossible to check fullness of shm.\n")
            
        char n_wr = *memp;

        if (n_wr == 0)
        {   exit(0);  
            if (semop (semid, finish_r, 4) == -1)
                PERROR ("Impossible to finish.\n")
            break; 
        }

        if (semop (semid, check, 4) == -1)
            PERROR ("Other process died.\n")
    
        memcpy (buffer, memp + 1, n_wr);
        if (write  (0, buffer, n_wr) == -1)
            PERROR ("WRITE ERROR.\n");
        memset (memp, '\0', PAGE_SZ);
    
        if (semop (semid, empty_up, 3) == -1)
            PERROR ("Impossible to set emptiness.\n")

    }

    if (semctl (semid, 0, SETALL, defaultSem) == -1)
        PERROR ("Impossible to set default values.\n")

    if (shmdt(memp) == -1)
        PERROR ("Impossible to detach from shared memory.\n")    

    return 0;
}
