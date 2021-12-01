#include "Error_handler.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int ErrorCheck (int error, const char* errmessage)
{
      
    switch (error)
    {
        case NO_ERRORS:     
            break;

        case INVALID_ARGS:  
            printf ("Invalid number of arguments or types of arguments. Enter %s", errmessage); 
            break;

        case NO_FILE:       
            printf ("There is no %s file.\n", errmessage);
            break;

        case NOT_OPENED:    
            printf ("File %s cannot be opened.\n", errmessage);
            break;

        case WRITE_FAIL:    
            printf ("Information cannot be written to %s.\n", errmessage);
            break;

        case READ_FAIL:     
            printf ("Information cannot be read from %s.\n", errmessage);
            break;

        case FIFO_NOT_CREATED:
            printf ("Cannot create fifo %s.\n", errmessage);
            break;

        case FAIL_FCNTL:
            printf ("Cannot change FIFO flag.\n");
            break;

        case NO_WRITER:
            printf ("There is no writer.\n");
            break;

        default:
            printf ("Unknowm error: %s.\n", errmessage);

    }
    return NO_ERRORS;
}