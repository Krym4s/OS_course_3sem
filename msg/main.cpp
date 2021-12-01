#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <unistd.h>
#include <cstdio>
#include <errno.h>

struct msgtype
{
    long    type;
    int     np;
};

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        printf ("Not enough arguments, please enter number of clild processes\n");
        return 0;
    }

    unsigned np;  
    if (sscanf (argv[1], "%u", &np) == 0)
    {
        printf ("argument is not a number\n");
        return 0;
    }

    int ask, confirm;
    int msgkey1 = 111;
    int msgkey2 = 222;

    ask = msgget(msgkey1, IPC_CREAT | 0666);
    if (ask == -1)
    {
        printf ("error msgqueue1\n");
        return 0;
    }
        

    confirm = msgget(msgkey2, IPC_CREAT | 0666);
    if (confirm == -1)
    {
        printf ("error msgqueue2\n");
        return 0;
    }
        
    msgtype msg;

    pid_t parent = getpid();
    pid_t tempPID;

    int pNumber = 0;
    for (int i = 0; i < np; i ++)
    {    
        pNumber = i + 1;   
        tempPID = fork();
        if (tempPID == -1)
            printf ("process number %d was not created\n", i + 1);

        if (getpid() != parent)
            break;
    }

    msgtype mrcv;

    int error = 0;

    if (getpid() != parent)
    {
        error = msgrcv(ask, (void*)&mrcv, sizeof (mrcv), pNumber, MSG_NOERROR);
        if (error < 0)
        {
            printf ("ask message was not recieved\n");
            return 0;
        }
        
        printf ("parent pid: %d pid:%d number of child:%d\n",parent, getpid(), pNumber);
        
        //fflush (stdout);

        msg.type = pNumber;

        error = msgsnd(confirm, (void*)&msg, sizeof (msg.np), 0);
        if (error < 0)
        {
            printf ("confirm message was not sent\n");
            return 0;
        }
    }
    else
    {
        for (int i = 0; i < np; i++)
        {
            msg.type = msg.np = i + 1;
            error = msgsnd(ask, (void*)&msg, sizeof (msg.np), 0);
            if (error < 0)
            {
                printf ("ask message was not sent\n");
                return 0;
            }
            error = msgrcv(confirm, (void*)&mrcv, sizeof (mrcv), i + 1, 0);
            if (error < 0)
            {
                printf ("confirm message was not recieved\n");
                return 0;
            }
        }
    }
}

