#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>

enum SEMS {
	MUTEX = 0,  // allows working with shared memory only for one process
	EMPTY = 1,  // shows if shared memory is empty
	FULL = 2,   // shows if shared memory is full
    BLOCK_W = 3,// enable only one writer
    BLOCK_R = 4,// enable only one reader
};

const char* KEY_PATH    = "/home/krym4s/lun/shared_memory/key.key";
const int   KEY_ID      = 1;
const int   PAGE_SZ     = 4096;
const int   BUF_SZ      = 100;
const int   N_SEM       = 5;

#define PERM_FLAG 0666 | IPC_CREAT

struct sembuf block_w[2]    = {
    {BLOCK_W, 0, 0},
    {BLOCK_W, 1, SEM_UNDO}
};

struct sembuf block_r[2]    = {
    {BLOCK_R, 0, 0},
    {BLOCK_R, 1, SEM_UNDO}
};

struct sembuf full_begin[2]  = {
    {FULL, 0, 0},
    {FULL, 1, SEM_UNDO}
};

struct sembuf empty_begin[2] = {
    {EMPTY, 0, 0},
    {EMPTY, 1, SEM_UNDO}
};

struct sembuf finish = {FULL, 1, SEM_UNDO};

unsigned short sems[7] = {1,0,0,0,0};


struct sembuf unblock_w     = {BLOCK_W, -1, 0};
/*struct sembuf unblock_r[2]     = {
    {BLOCK_R, -1, SEM_UNDO},
    {EMPTY,   -1, SEM_UNDO}
};*/

struct sembuf unblock_r = {BLOCK_R, -1, 0};

struct sembuf mutex_up      = {MUTEX,    1, 0};
struct sembuf mutex_dw      = {MUTEX,   -1, 0};
struct sembuf empty_up      = {EMPTY,    1, SEM_UNDO};
struct sembuf empty_dw      = {EMPTY,   -1, SEM_UNDO};
struct sembuf full_up       = {FULL,     1, 0};
struct sembuf full_dw       = {FULL,    -1, SEM_UNDO};
 
#define PERROR(string) {perror (string);fflush (stderr);exit(1);}