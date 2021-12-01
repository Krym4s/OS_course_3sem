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
	ALIVE_W = 5,// shows if writer is alive
	ALIVE_R = 6, // shows if reader is alive
};

const char* KEY_PATH    = "/home/krym4s/lun/shared_memory/key.key";
const int   KEY_ID      = 1;
const int   PAGE_SZ     = 4096;
const int   BUF_SZ      = 1;
const int   N_SEM       = 7;

#define PERM_FLAG 0666 | IPC_CREAT

struct sembuf block_w[2]    = {
    {BLOCK_W, 0, 0},
    {BLOCK_W, 1, SEM_UNDO}
};

struct sembuf block_r[2]    = {
    {BLOCK_R, 0, 0},
    {BLOCK_R, 1, SEM_UNDO}
};

struct sembuf unblock_w     = {BLOCK_W, -1, SEM_UNDO};
struct sembuf unblock_r     = {BLOCK_R, -1, SEM_UNDO};

struct sembuf unblock[2]    ={
    {BLOCK_W, -1, SEM_UNDO},
    {BLOCK_R, -1, SEM_UNDO}
}

struct sembuf mutex_up      = {MUTEX,    1, SEM_UNDO};
struct sembuf mutex_dw      = {MUTEX,   -1, SEM_UNDO};
struct sembuf empty_up      = {EMPTY,    1, 0};
struct sembuf empty_dw      = {EMPTY,   -1, 0};
struct sembuf full_up       = {FULL,     1, 0};
struct sembuf full_dw       = {FULL,    -1, 0};
struct sembuf finish_w      = {ALIVE_W,  1, 0};        
struct sembuf reader_ready  = {ALIVE_R, -5, SEM_UNDO};
struct sembuf finish_check  = {ALIVE_W, -1, IPC_NOWAIT};
 
struct sembuf start_writer[3]   = {
                                    {MUTEX,   1, 0},
                                    {EMPTY,   1, 0},
                                    {ALIVE_W, 5, SEM_UNDO},
                                }; 

struct sembuf start_reader[2]   = {
                                    {ALIVE_W,  -5, SEM_UNDO},
                                    {ALIVE_R,   5, SEM_UNDO}
                                }; 

#define PERROR(string) {perror (string);fflush (stderr);exit(1);}
