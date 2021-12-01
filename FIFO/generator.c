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

int main (int argc, char* argv[])
{
    int fd = open (argv[1], O_WRONLY | O_CREAT | );
    int n = atoi (argv[2]);
    char c = argv[3][0];

    char* buffer = (char*) calloc (1000, sizeof (char));
    for (int i = 0; i < 1000 ; i ++)
    {
        buffer[i] = c;
    }
    for (int i = 0; i < n; i ++)
    {
        write (fd, buffer, 1000);
    }
    
    return 0;
}