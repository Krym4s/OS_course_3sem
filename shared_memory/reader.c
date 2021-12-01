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

    if (semop (semid, start_reader, 2) == -1)   // checks if ALIVE_W is set
        PERROR ("Reader cannot start working.\n")  // sets ALIVE_R

    while (true)
    {
        if (semop (semid, &full_dw, 1) == -1)   // checks if shm is full
            PERROR ("Impossible to check fullness of shm.\n")

        if (semop (semid, &mutex_dw, 1) == -1)  // blocks other process interruptions 
            PERROR ("Impossible to block other process interruptions.\n")

        char n_wr = *memp;
        
        memcpy (buffer, memp + 1, n_wr);
        write  (0, buffer, n_wr);
        memset (memp, '\0', PAGE_SZ);
        
        if (semop (semid, &mutex_up, 1) == -1)  // unblocks other process interruptions
            PERROR ("Impossible to unblock for interruptions.\n")

        if (semop (semid, &empty_up, 1) == -1)  // signals to writer to start writing to shm
            PERROR ("Impossible to set emptiness.\n")

        if (n_wr == 0)
            break;  
    }     
    

    if (semop (semid, &unblock_r, 1) == -1)  
        PERROR ("Impossible to unblock reader.\n")

    if (semop (semid, &unblock_w, 1) == -1)
        PERROR ("Impossible to unblock writer.\n")

    if (semctl (semid, 0, IPC_RMID, NULL) == -1)
        PERROR("Impossible to remove semaphores.\n")

    if (shmdt(memp) == -1)
        PERROR ("Impossible to detach from shared memory.\n")    

    if (shmctl (shmid, IPC_RMID, NULL) == -1)
        PERROR ("Impossible to remove shared memory.\n")


    return 0;
}