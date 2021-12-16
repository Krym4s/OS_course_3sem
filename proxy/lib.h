#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <math.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/prctl.h>
#include <math.h>
#include <poll.h>


#pragma once

enum MODES{
    READ = 0,
    WRITE = 1,
};

const size_t MAXBUFFSIZE = 10000;

#define DEBUG_MODE

#define PERROR(string) {perror (string);fflush (stderr);exit(1);}

struct ChildInfo {
    unsigned id;
    pid_t pid;

    int fifoToPrnt[2];
    int fifoFromPrnt[2];
};

struct Connection {
    int input;
    int output;

    size_t buf_size;
    char* buffer;

    size_t iRead;
    size_t iWrite;    

    size_t busy;
    size_t empty;
};

const unsigned buff_sz = 4096;

void CloseChildExtraPipes (struct ChildInfo* chInfos, unsigned idx);
void TrackPrntDied(pid_t ppid);
void ChildFunction (struct ChildInfo* childInfo, char* filePath, const unsigned nChild);
void ParentFunction (struct ChildInfo* childInfo, const unsigned nChild);
void PrepareBuffer(struct Connection* connections, struct ChildInfo* childInfos,
							   const unsigned idx, const unsigned nChilds);
unsigned CountSize(const unsigned idx, const unsigned nChilds);
void ReadToBuffer(struct Connection* connection, const int id); 
void WriteFromBuffer(struct Connection* connection, const int id);                         
