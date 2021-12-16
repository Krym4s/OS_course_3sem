#include "lib.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
        PERROR ("Please, enter arguments in format numver of children, filename.\n");

    size_t nChild;
    if (sscanf (argv[1], "%ld", &nChild) != 1)
        PERROR ("Second argument must be integer.\n");

    pid_t ppid;

    struct ChildInfo* chInfos = (struct ChildInfo*)calloc (nChild, sizeof(*chInfos)); 

    for (unsigned idx = 0; idx < nChild; idx++)
    {
        chInfos[idx].id = idx;

        if (pipe(chInfos[idx].fifoFromPrnt) == -1)
            PERROR ("Impossible to create pipe reading from parent.\n");

        if (pipe(chInfos[idx].fifoToPrnt) == -1)
            PERROR ("Impossible to create pipe writing to parent.\n");

        pid_t childpid = fork();

        switch (childpid)
        {
            case 0:
            TrackPrntDied (getppid());
            CloseChildExtraPipes (chInfos, idx);
            ChildFunction (chInfos + idx, argv[2], nChild);
            free (chInfos);
            exit(0);
            break;

            default:
            if (close(chInfos[idx].fifoFromPrnt[READ]) == -1)
				PERROR("Enable read.\n")
            chInfos[idx].fifoFromPrnt[READ] = -1;

            if (close(chInfos[idx].fifoToPrnt[WRITE]) == -1)	
                PERROR("Parent: Error in close(to_write)")
            chInfos[idx].fifoToPrnt[WRITE] = -1;
            continue;
        }
    }

    ParentFunction(chInfos, nChild);
	free(chInfos);

    return 0;
}

void CloseChildExtraPipes (struct ChildInfo* chInfos, unsigned idx)
{
    for (unsigned i = 0; i < idx; i ++)
    {
        if(close(chInfos[i].fifoToPrnt[READ]) == -1)
            PERROR("Impossible to close fifoToPrnt to read.\n");
        chInfos[i].fifoToPrnt[READ] = -1;

        if (close(chInfos[i].fifoFromPrnt[WRITE]) == -1)
            PERROR("Impossible to close fifoFromPrnt to write.\n");    
        chInfos[i].fifoFromPrnt[WRITE] = -1;
    }

    if(close(chInfos[idx].fifoToPrnt[READ]) == -1)    
        PERROR("Impossible to close fifoToPrnt to read.\n");        
    chInfos[idx].fifoToPrnt[READ] = -1;

    if (close(chInfos[idx].fifoFromPrnt[WRITE]) == -1)
            PERROR("Impossible to close fifoFromPrnt to write.\n");
    chInfos[idx].fifoFromPrnt[WRITE] = -1;

}

void ChildFunction (struct ChildInfo* childInfo, char* filePath, const unsigned nChild)
{   
    int input, output;
    if (childInfo->id == 0)
    {
        input = open (filePath, O_RDONLY);
        if (close (childInfo->fifoFromPrnt[READ]) == -1)
            PERROR ("Impossible to close input.\n")
    }
    else    
		input = childInfo->fifoFromPrnt[READ];

    if (childInfo->id == nChild - 1)
    {
        output = STDOUT_FILENO;
        if (close (childInfo->fifoToPrnt[WRITE]) == -1)
            PERROR ("Impossible to close output.\n")
    }
    else
        output = childInfo->fifoToPrnt[WRITE];
    
    if (input == -1)
        PERROR ("There is no input descriptor.\n")

    if (output == -1)
        PERROR ("Ther is no output descriptor.\n")

    if (fcntl(output, F_SETFL, O_WRONLY) == -1)
		PERROR("Impossible to set file flags.\n")

    if (fcntl(input, F_SETFL, O_RDONLY) == -1)
		PERROR("Impossible to set file flags.\n")

    int read_success = -1;
    char buffer[buff_sz];

    while (true)
    {
        read_success = read (input, buffer, buff_sz);
        if (read_success == -1)
            PERROR ("Error in reading to child buffer.\n")

        if (read_success == 0) 
            break;
        
        if (write (output, buffer, read_success) == -1)
            PERROR ("Error in writing to parent pipe.\n")
    }

    if (close (input) == -1)
        PERROR("Impossible to close child input file descriptor.\n")
    input = childInfo->fifoFromPrnt[READ] = -1;    

    if (close (output) == -1)
        PERROR("Impossible to close child output file descriptor.\n")
    output = childInfo->fifoToPrnt[WRITE] = -1;
}

void ParentFunction (struct ChildInfo* childInfos, const unsigned nChild)
{
    int maxFD = -1;
	fd_set output, input;
    struct Connection* connections = (Connection*)calloc(nChild - 1, sizeof(struct Connection));
	if (connections == NULL)
		PERROR("memory was not given.\n")

	for (int idx = 0; idx < nChild - 1; idx++){

		PrepareBuffer(connections, childInfos, idx, nChild);

		if (fcntl(connections[idx].input, F_SETFL, O_RDONLY | O_NONBLOCK) == -1)
		    PERROR("Impossible to set read to unblock.\n")
        
		if (fcntl(connections[idx].output, F_SETFL, O_WRONLY | O_NONBLOCK) == -1)
		    PERROR("Impossible to set write to unblock.\n")
	}	

	size_t beg_idx = 0;
    int connect = nChild - 1;
	while (beg_idx < connect){
		FD_ZERO(&output);
		FD_ZERO(&input);

		for (size_t idx = beg_idx; idx < connect; idx++){

			if (connections[idx].input != -1 && connections[idx].empty > 0)
				FD_SET(connections[idx].input, &input);
			if (connections[idx].output != -1 && connections[idx].busy > 0)
				FD_SET(connections[idx].output, &output);

			if (connections[idx].input > maxFD)
				maxFD = connections[idx].input;
			if (connections[idx].output > maxFD)
				maxFD = connections[idx].output;
		}	

		if (select(maxFD + 1, &input, &output, NULL, NULL) < 0)
			PERROR("Error in select\n")	    
	    maxFD = -1;			    

	    for (size_t idx = beg_idx; idx < connect; idx++){
			if (FD_ISSET(connections[idx].input, &input) && connections[idx].empty > 0)
				ReadToBuffer(&connections[idx], idx);
            if (FD_ISSET(connections[idx].output, &output) && connections[idx].busy > 0)
				WriteFromBuffer(&connections[idx], idx);
			
	        if (connections[idx].input == -1 && connections[idx].output != -1 && connections[idx].busy == 0) {
				close(connections[idx].output);								
				connections[idx].output = -1;

				if (beg_idx != idx)
	                PERROR("One of childs is dead\n")

	            beg_idx++;
				free(connections[idx].buffer);
			}
		}
	}	
    for (size_t idx = 0; idx < nChild; idx++){
            if (waitpid(childInfos[idx].pid, NULL, 0) == -1)
                PERROR("Impossible to wait for child.\n")
    }
        
    free(connections);
}

void WriteFromBuffer(struct Connection* connection, const int id)
{
    errno = 0;
    int ret_write = write(connection->output, &connection->buffer[connection->iWrite], connection->busy);
    if (ret_write < 0 && errno != EAGAIN) 
        PERROR ("Impossible to write from buffer.\n")
        

    if (connection->iWrite >= connection->iRead)
        connection->empty += ret_write;

    if (connection->iWrite + ret_write == connection->buf_size) {
        connection->busy = connection->iRead;
        connection->iWrite = 0; 
    }
    else {
        connection->busy -= ret_write;
        connection->iWrite += ret_write; 
    }
}


void PrepareBuffer(struct Connection* connections, struct ChildInfo* childInfos,
							   const unsigned idx, const unsigned nChilds)
{
    connections[idx].buf_size = CountSize(idx, nChilds);
	connections[idx].buffer = (char*)calloc(1, connections[idx].buf_size);
	if (connections[idx].buffer == NULL)
		PERROR("Impossible to allocate memory in parent process.\n")

	connections[idx].iRead = 0;
	connections[idx].iWrite = 0;

	connections[idx].busy = 0;
	connections[idx].empty = connections[idx].buf_size;

	connections[idx].input = childInfos[idx].fifoToPrnt[READ];
	connections[idx].output = childInfos[idx + 1].fifoFromPrnt[WRITE];
}

unsigned CountSize(const unsigned idx, const unsigned nChilds)
{
    unsigned buff_sz = pow (3, nChilds - idx) * 81;
    return (MAXBUFFSIZE > buff_sz) ? buff_sz : MAXBUFFSIZE;
}

void ReadToBuffer(struct Connection* connection, const int id)
{
    int ret_read = read(connection->input, &connection->buffer[connection->iRead], connection->empty);
    if (ret_read < 0) 
        PERROR("Error in read\n")
    
    if (ret_read == 0) {
        close(connection->input);
        connection->input = -1;
        return;
    }

    if (connection->iRead >= connection->iWrite)
        connection->busy += ret_read;

    if (connection->iRead + ret_read == connection->buf_size) {
        connection->iRead = 0;
        connection->empty = connection->iWrite;
    }
    else {
        connection->iRead += ret_read;
        connection->empty -= ret_read;
    }
}

void TrackPrntDied(pid_t ppid)
{
    if (prctl(PR_SET_PDEATHSIG, SIGTERM) < 0)
        PERROR("Impossible to set signal of parents death.\n")

    if (ppid != getppid())
    	PERROR("Parent is not correct.\n")
}
