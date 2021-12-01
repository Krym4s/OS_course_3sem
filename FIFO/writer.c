#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "argument_handler.h"
#include "Error_handler.h"

const int a = _PC_PIPE_BUF;
const char* fifoName = "convertor";
const int FIFONAMESIZE = sizeof (pid_t) * 8 + 3;


int getReader (int fdinput);

int main (int argc, char* argv[])
{
    int fdFILE;

    if (argc != 2)
    {
        printf ("Invalid number of arguments.\n");
        return INVALID_ARGS;
    }

    if (ErrorCheck (TryOpenFiles (argv[1], &fdFILE), argv[1]))
        return NOT_OPENED;
    
    ErrorCheck (getReader (fdFILE), "getReader error");

    return 0;
}

int getReader (int fdinput)
{
    char buffer[_PC_PIPE_BUF] = {};
    char FIFOoutputName[FIFONAMESIZE];

    errno = 0;
    int error = mkfifo(fifoName, 0666);
    if (error && (errno != EEXIST))
        return ErrorCheck (error, "Transporter fifo not created");

    int fdTransporter = open (fifoName, O_RDONLY);
    if (fdTransporter < 0)
    {
        return ErrorCheck (NOT_OPENED, "Transporter fifo not opened");
    }

    if (read (fdTransporter, FIFOoutputName, FIFONAMESIZE) < FIFONAMESIZE)
        return NO_FILE;

    close (fdTransporter);

    errno = 0;
    int outputFIFOfd = open (FIFOoutputName, O_WRONLY | O_NONBLOCK);

    if (outputFIFOfd < 0)
        return ErrorCheck (NOT_OPENED, "output fifo not opened");

    char flag[1] = {'f'};
    write (outputFIFOfd, flag, 1);

    int successRead = 0;

    int set_fcntl = fcntl (outputFIFOfd, F_SETFL, O_WRONLY);
    if (set_fcntl)
        return FAIL_FCNTL;

    while ((successRead = read (fdinput, buffer, _PC_PIPE_BUF)) > 0)
    {
        if (write (outputFIFOfd, buffer, successRead) < successRead)
        {
            close  (outputFIFOfd);
            unlink (FIFOoutputName);
            return WRITE_FAIL;
        }
            
    }
    if (successRead == -1)
    {
        close  (outputFIFOfd);
        unlink (FIFOoutputName);
        return READ_FAIL;
    }
        
    close (outputFIFOfd);

     printf ("\nWrited succsessfully\n");

    return NO_ERRORS;
}

int TryOpenFiles (const char* firstFileN, int* fileDescr)
{
    if (!firstFileN)
        return INVALID_ARGS;  

    *fileDescr = open (firstFileN, O_RDONLY);
    
    if (fileDescr < 0)
        return NO_FILE;

    return NO_ERRORS;
}