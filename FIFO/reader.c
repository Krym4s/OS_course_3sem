#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "Error_handler.h"

const int fdFILE = STDOUT_FILENO;
const int a = _PC_PIPE_BUF;
const char* fifoName = "convertor";
const int FIFONAMESIZE = sizeof (pid_t) * 8 + 3;

const char* FIFONameFormat = "tr%032d\0";

int getWriter (int fdFIFO, const char* fifoname, int fdoutput);

int main(int argc, char* argv[])
{
    int fdFIFO;
    pid_t currentpid = getpid();

    if (argc != 1)
    {
        printf ("Invalid number of arguments.\n");
        return INVALID_ARGS;
    }
        
    char filename[FIFONAMESIZE];


    sprintf (filename, FIFONameFormat, currentpid);
    int error = mkfifo (filename, 0666);

    if( error && errno != EEXIST)
    {
        return ErrorCheck (FIFO_NOT_CREATED, "Fifo not created.\n");
    }

    fdFIFO = open (filename, O_RDONLY | O_NONBLOCK);

    if (fdFIFO < 0)
        return ErrorCheck (NOT_OPENED, "fifo not opened");

    ErrorCheck (getWriter (fdFIFO, filename, STDOUT_FILENO), "\nget writer function\n");

    return 0;
}

int getWriter (int fdFIFO, const char* tempfifoname, int fdoutput)
{
    int fdTransporter = 0;

    int error = mkfifo (fifoName, 0666);
    if (error && errno != EEXIST)
        return FIFO_NOT_CREATED;
    
    fdTransporter = open (fifoName, O_WRONLY);
    if (fdTransporter < 0)
        return FIFO_NOT_CREATED;

    if (write (fdTransporter, tempfifoname, FIFONAMESIZE) == -1)
        return WRITE_FAIL;

    close (fdTransporter);


    char buffer[_PC_PIPE_BUF];
    int successRead = 0;

    sleep (1);

    char flag[1];

    if (read (fdFIFO, flag, 1) == -1)
        return NO_WRITER;

    int set_fcntl = fcntl (fdFIFO, F_SETFL, O_RDONLY);
    if (set_fcntl)
        return FAIL_FCNTL;

    errno = 0;

    while (successRead = read (fdFIFO, buffer,_PC_PIPE_BUF))
    {
        if (write (fdoutput, buffer, successRead) < successRead)
        {   
            close (fdFIFO);
            unlink (tempfifoname);
            return WRITE_FAIL;
        }
            
    }
    if (successRead == -1)
    {       
        close (fdFIFO);
        unlink (tempfifoname);
        return READ_FAIL;
    }
        

    close (fdFIFO);
    unlink (tempfifoname);

    printf ("\nReaded succsessfully\n");

    return NO_ERRORS;
}