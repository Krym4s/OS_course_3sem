#include "lib.h"

bool cur_bit;

int main(int argc, char** argv)
{

	if (argc != 2)
        PERROR ("There is no input file, please enter it's name.\n");

    sigset_t init_block = {0};    
    sigfillset(&init_block);

    sigprocmask(SIG_BLOCK, &init_block, NULL);

    cur_bit = 0;
    pid_t parent = getpid();
    pid_t child  = fork(); 
    if (child == -1)
        PERROR ("Child was not created.\n")

    if (parent == getpid ())
    {
        RunParent (child);
    } else
    {
        RunChild (argv[1], parent);
    } 

	return 0;
}


void RunChild(char* filename, const pid_t ppid) 
{    
    if (prctl(PR_SET_PDEATHSIG, SIGTERM) == -1)
       PERROR ("Impossible to set parent termination signal.\n");

    if (ppid != getppid())
        PERROR ("There is no parent process.\n")

    int fd = open(filename, O_RDONLY);
    if (fd == -1)
        PERROR ("Impossible to open file.\n")           

    struct sigaction wait_prnt = {}, die_prnt = {};

    wait_prnt.sa_handler = Handler_Prnt_Wait;
    sigfillset(&wait_prnt.sa_mask);
    wait_prnt.sa_flags = SA_NODEFER;

    if (sigaction(SIGUSR1, &wait_prnt, NULL) == -1)
        PERROR ("Action for SIG_USR1 did not set.\n")

    die_prnt.sa_handler = Handler_Prnt_Died;
    sigfillset(&die_prnt.sa_mask);
    die_prnt.sa_flags = SA_NODEFER;

    if (sigaction(SIGTERM, &die_prnt, NULL) == -1)
        PERROR ("Action for dead of parent did not set.\n");

    sigset_t waiting;
    sigfillset(&waiting);
    sigdelset(&waiting, SIGUSR1);
    sigdelset(&waiting, SIGTERM);

    int count_read = -1;
    while(true){
    	char cur_letter = 0;
    	count_read = read(fd, &cur_letter, 1);

    	if (count_read == 0)
    		break;

    	if (count_read == -1)
    		PERROR ("Read error.\n")

    	for (unsigned iBit = 0; iBit < nBits; iBit++){
    		char mask = 0x01 << iBit;

    		char bit = mask & cur_letter;    		
             
    		if (bit == 0){
    			if (kill(ppid, SIGUSR1) == -1)
    				PERROR("Impossible to send 0 bit to parent.\n") // send 0 to parent
    		}
    		else {
    			if (kill(ppid, SIGUSR2) == -1)
    				PERROR("Impossible to send 1 bit to parent.\n") // send 1 to parent
    		}    	                        
            sigsuspend(&waiting); // waits for parent's handling
    	}    
    }    

    close(fd);
}


void RunParent(const pid_t child_pid)
{   
	struct sigaction sig_USR1 = {}, sig_USR2 = {}, sig_CHLD_Died = {};

	sig_USR1.sa_handler       = Handler_USR1;
	sig_USR2.sa_handler       = Handler_USR2;
	sig_CHLD_Died.sa_handler  = Handler_CHLD_Died;

    sig_USR1.sa_flags = 0;
    sig_USR2.sa_flags = 0;
    sig_CHLD_Died.sa_flags = SA_NOCLDSTOP;

	sigfillset(&sig_USR1.sa_mask);    
	sigfillset(&sig_USR2.sa_mask);        
	sigfillset(&sig_CHLD_Died.sa_mask);

	if (sigaction(SIGUSR1, &sig_USR1, NULL) == -1)
		PERROR ("Impossible to set handling SIGUSR1.\n")

	if (sigaction(SIGUSR2, &sig_USR2, NULL) == -1)
		PERROR ("Impossible to set handling SIGUSR2\n")

	if (sigaction(SIGCHLD,  &sig_CHLD_Died, NULL) == -1)
		PERROR ("Impossible to set checker if child died.\n")
					
	sigset_t wait_sig;
	sigfillset(&wait_sig);
	sigdelset(&wait_sig, SIGUSR1);
	sigdelset(&wait_sig, SIGUSR2);	//unblock some signals	
	sigdelset(&wait_sig, SIGCHLD);

	while(true){

		char cur_letter = 0;

		for (unsigned iBit = 0; iBit < nBits; iBit++){
			char mask = 0x01 << iBit;           
            
			if (sigsuspend(&wait_sig) != -1)
				PERROR ("Sigsuspend did not worked.\n")

			if (cur_bit)
                cur_letter = cur_letter | mask;

            kill(child_pid, SIGUSR1);
		}

		printf("%c", cur_letter); 
        fflush(stdout);       
	}		
}


void Handler_USR1(int sig)
{
	cur_bit = 0;	
}


void Handler_USR2(int sig)
{
	cur_bit = 1;		
}

void Handler_Prnt_Died(int sig)
{
    exit(EXIT_FAILURE);
}

void Handler_CHLD_Died(int sig)
{
	exit(EXIT_FAILURE);
}

void Handler_Prnt_Wait(int sig)
{
    return;
}
