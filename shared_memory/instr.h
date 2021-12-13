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
	EMPTY = 0,  // shows if shared memory is empty
	FULL = 1,   // shows if shared memory is full
    BLOCK_W = 2,// enable only one writer
    BLOCK_R = 3,// enable only one reader
};

const char* KEY_PATH    = "/home/krym4s/lun/shared_memory/key.key";
const int   KEY_ID      = 1;
const int   PAGE_SZ     = 4096;
const int   BUF_SZ      = 100;
const int   N_SEM       = 4;

#define PERM_FLAG 0666 | IPC_CREAT

struct sembuf block_w[2]    = {
    {BLOCK_W, 0, 0},
    {BLOCK_W, 1, SEM_UNDO},
};

struct sembuf block_r[2]    = {
    {BLOCK_R, 0, 0},
    {BLOCK_R, 1, SEM_UNDO},
};

struct sembuf connect_w[5] = {
    {BLOCK_W, -2, 0},
    {BLOCK_R, 1, SEM_UNDO},
    {BLOCK_W, 2, 0},
    {FULL,  1, 0},
    {FULL, -1, SEM_UNDO}
};
struct sembuf connect_r[5] = {
    {BLOCK_W, -1, 0},
    {BLOCK_W,  1, 0},
    {BLOCK_W, 1, SEM_UNDO},
    {EMPTY,  1, 0},
    {EMPTY, -1, SEM_UNDO}
};

struct sembuf full_begin[2]  = {
    {FULL, 0, 0},
    {FULL, 1, SEM_UNDO}
};

struct sembuf empty_begin[2] = {
    {EMPTY, 0, 0},
    {EMPTY, 1, SEM_UNDO}
};

struct sembuf writer_end[4] = 
{
    {FULL,  1, SEM_UNDO},
    {FULL, -1, 0},
    {BLOCK_R, -1, SEM_UNDO},
    {BLOCK_W, -1, SEM_UNDO}
};

struct sembuf reader_end[4] =
{
    {EMPTY,  1, SEM_UNDO},
    {EMPTY, -1, 0},
    {BLOCK_R, -1, SEM_UNDO},
    {BLOCK_W, -1, SEM_UNDO}
};


struct sembuf check[4] = {
    {BLOCK_R, -2, IPC_NOWAIT},
    {BLOCK_W, -2, IPC_NOWAIT},
    {BLOCK_R,  2, 0},
    {BLOCK_W,  2, 0}
};

struct sembuf finish = {FULL, 1, SEM_UNDO};

unsigned short defaultSem[5] = {0,0,0,0};

struct sembuf full_up[3] = {
    {FULL,   1, SEM_UNDO},
    {EMPTY,  1, SEM_UNDO},
    {EMPTY, -1, 0}
};

struct sembuf empty_up[3] = {
    {EMPTY,  1, SEM_UNDO},
    {FULL,   1, SEM_UNDO},
    {FULL,  -1, 0}
};

struct sembuf finish_r[4] = {
    {EMPTY,  1, SEM_UNDO},
    {EMPTY, -1, 0},
    {FULL,   1, SEM_UNDO},
    {FULL,  -1, 0}
};


struct sembuf unblock_w     = {BLOCK_W, -1, 0};

struct sembuf unblock_r = {BLOCK_R, -1, 0};

//struct sembuf empty_up      = {EMPTY,    1, SEM_UNDO};
struct sembuf empty_dw[3]      = {
    {EMPTY,   -1, SEM_UNDO},
    {FULL,     1, 0},
    {FULL,    -1, SEM_UNDO}
};
//struct sembuf full_up       = {FULL,     1, 0};
struct sembuf full_dw[3]       = {
    {FULL,    -1, SEM_UNDO},
    {EMPTY,    1, 0},
    {EMPTY,   -1, SEM_UNDO}
}; 


#define PERROR(string) {perror (string);fflush (stderr);exit(1);}
