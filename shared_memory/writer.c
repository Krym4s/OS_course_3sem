#include "instr.h"

int main(int argc, char** argv)
{
    if (argc != 2)
        PERROR ("Wrong number of arguments.\n")

    key_t key = ftok(KEY_PATH, KEY_ID);
    if (key == -1)
        PERROR ("Key was not generated.\n")

    int fd = open (argv[1], O_RDONLY);
    if (fd == -1)
        PERROR("File cannot be opened.\n")

    int semid = semget(key, N_SEM, PERM_FLAG);
    if (semid == -1)
        PERROR("Semaphore set was not created.\n")

    int shmid = shmget(key, PAGE_SZ, PERM_FLAG);
    if (shmid == -1)
        PERROR ("Share memory did not get.\n")

    char* memp = (char*) shmat(shmid, NULL, 0);
    if (memp == NULL)
        PERROR ("Shared Memory was not attached.")

    if (semop (semid, block_w, 2) == -1)
        PERROR ("cannot block writer.\n")

    if (semop (semid, connect_w, 5) == -1)
        PERROR ("Impossible to find reader.\n")

    if (semop (semid, check, 4) == -1)
        PERROR ("Other process died.\n")

    errno = 0;      
    char success_read = 1;

    while (success_read)
    {   
        if (semop (semid, empty_dw, 3) == -1) // checks if shm is emmpty
            PERROR ("Impossible to check emptiness of shm.\n")

        if ((success_read = read (fd, memp + 1, BUF_SZ)) == -1)
            PERROR ("Read from file error.\n")

        *memp = success_read;
        memset(memp + success_read + 1, '\0', BUF_SZ - success_read);

        if (semop (semid, check, 4) == -1)
            PERROR ("Other process died.\n")


        if (semop (semid, full_up, 3) == -1)
            PERROR ("Impossible to set fullness.\n")

        if (success_read == 0)
            break;
    }

    if (shmdt(memp) == -1)
        PERROR ("Impossible to detach from shared memory.\n")    

    close (fd);

    return 0;
}

