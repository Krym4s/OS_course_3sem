#include "lib.hpp"

int main(int argc, char* argv[])
{
    if (argc != 3)
        PERROR ("Please, enter arguments in format numver of children, filename.\n");

    size_t nChild;
    if (sscanf (argv[1], "%ld", &nChild) != 1)
        PERROR ("Second argument must be integer.\n");

    struct ChildInfo* chInfos = (struct ChildInfo*)calloc (nChild, sizeof(*chInfos));
    pid_t ppid = getpid();

    for (int idx = 0; idx < nChild; idx++)
    {
        if (pipe(chInfos[idx].fifoFromPrnt) == -1)
            PERROR ("Impossible to create pipe reading from parent.\n");

        if (pipe(chInfos[idx].fifoToPrnt) == -1)
            PERROR ("Impossible to create pipe writing to parent.\n");

        chInfos[idx].id = idx;

        pid_t childp = fork();
        switch (childp)
        {
        case 0:
            TrackPrntDied(ppid);
            CloseUnusedPipes(chInfos, idx);
            ChildFunction (chInfos + idx, argv[2]);
            free (chInfos);
            exit (0);
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

void TrackPrntDied(pid_t ppid)
{
    if (prctl(PR_SET_PDEATHSIG, SIGTERM) < 0)
        PERROR("Impossible to set signal of parents death.\n")

    if (ppid != getppid())
    	PERROR("Parent is not correct.\n")
}

void CloseUnusedPipes(struct ChildInfo* childInfos, const size_t nChild)
{
	for (int idx = 0; idx < nChild; idx++){
		if (close(childInfos[idx].fifoFromPrnt[WRITE]) == -1)	
			PERROR("Impossible to close pipe writing to child.\n")
		childInfos[idx].fifoFromPrnt[WRITE] = -1;	

		if (close(childInfos[idx].fifoToPrnt[READ]) == -1)	
			PERROR("Impossible to close pipe reading from child.\n")
		childInfos[idx].fifoToPrnt[READ] = -1;
	}
}

void ChildFunction(struct ChildInfo* childInfo, char* filePath)
{
    if (close(childInfo->fifoToPrnt[READ]) == -1)	
		PERROR("Child: Error in close(to_read)")
	childInfo->fifoToPrnt[READ] = -1;

	if (close(childInfo->fifoFromPrnt[WRITE]) == -1)	
		PERROR("Child: Error in close(from_write)")
	childInfo->fifoFromPrnt[WRITE] = -1;

	int fd_reader = -1;
	int fd_writer = childInfo->fifoToPrnt[WRITE];

	childInfo->pid = getpid();

	if (childInfo->id == 0)
		fd_reader = open(filePath, O_RDONLY);	
	else
		fd_reader = childInfo->fifoFromPrnt[READ];

	if (fd_reader < 0)
		PERROR("Something is wrong with fd_reader\n")

	if (fcntl(fd_writer, F_SETFL, O_WRONLY) == -1)
		PERROR("Impossible to set file flags.\n")

    if (fcntl(fd_reader, F_SETFL, O_RDONLY) == -1)
		PERROR("Impossible to set file flags.\n")

	int ret_read = -1;
	char buffer[4096];
	while (true){		
	    ret_read = read(fd_reader, buffer, 4096);
	    
	    if (ret_read < 0)
	        PERROR("Error in read.\n")
	    
	    if (ret_read == 0)
            break;

		if (write(fd_writer, buffer, ret_read) == -1)
			PERROR("Error in write.\n")		
	}

	if (close(fd_writer) == -1)	
		PERROR("Impossible to close fd_writer.\n")
	childInfo->fifoToPrnt[WRITE] = -1;

	if (close(fd_reader) == -1)	
		PERROR("Impossible to close fd_reader.\n")
	childInfo->fifoFromPrnt[READ] = -1;
}

void ParentFunction(struct ChildInfo* childInfos, const size_t numChilds)
{
	int maxFD = -1;
	fd_set fd_writers, fd_readers;

	struct Connection* connections = (Connection*)calloc(numChilds, sizeof(struct Connection));
	if (connections == NULL)
		PERROR("memory was not given.\n")

	for (int idx = 0; idx < numChilds; idx++){

		PrepareBuffer(connections, childInfos, nChild, numChilds);

		if (fcntl(connections[idx].fd_reader, F_SETFL, O_RDONLY | O_NONBLOCK) == -1)
		    PERROR("Parent: Error in fcntl(fd_reader)")

		if (fcntl(connections[idx].fd_writer, F_SETFL, O_WRONLY | O_NONBLOCK) == -1)
		    PERROR("Parent: Error in fcntl(fd_writer)")
	}	

	size_t cur_alive = 0;
	while (cur_alive < numChilds){
		FD_ZERO(&fd_writers);
		FD_ZERO(&fd_readers);

		for (size_t nChild = cur_alive; nChild < numChilds; nChild++){
			if (connections[nChild].fd_reader != -1 && connections[nChild].empty > 0)
				FD_SET(connections[nChild].fd_reader, &fd_readers);
			if (connections[nChild].fd_writer != -1 && connections[nChild].busy > 0)
				FD_SET(connections[nChild].fd_writer, &fd_writers);

			if (connections[nChild].fd_reader > maxFD)
				maxFD = connections[nChild].fd_reader;
			if (connections[nChild].fd_writer > maxFD)
				maxFD = connections[nChild].fd_writer;
		}	

		errno = 0;
		if (select(maxFD + 1, &fd_readers, &fd_writers, NULL, NULL) < 0)
			PERROR("Parent: Error in select()\n")	    
	    maxFD = -1;			    

	    for (size_t nChild = cur_alive; nChild < numChilds; nChild++){
			if (FD_ISSET(connections[nChild].fd_reader, &fd_readers) && connections[nChild].empty > 0)
				ReadToBuffer(&connections[nChild], nChild);
			
            if (FD_ISSET(connections[nChild].fd_writer, &fd_writers) && connections[nChild].busy > 0)
				WriteFromBuffer(&connections[nChild], nChild);
			

	        if (connections[nChild].fd_reader == -1 && connections[nChild].fd_writer != -1 && connections[nChild].busy == 0) {
				close(connections[nChild].fd_writer);								
				connections[nChild].fd_writer = -1;

				if (cur_alive != nChild)
	                PERROR("One of childs is dead\n")

	            cur_alive++;
				free(connections[nChild].buffer);
			}
		}
	}	
    for (size_t nChild = 0; nChild < numChilds; nChild++){
            if (waitpid(childInfos[nChild].pid, NULL, 0) == -1)
                PERROR("Impossible to wait for child.\n")
    }
        
    free(connections);
}

void ReadToBuffer(struct Connection* connection, const int id)
{
    size_t ret_read = read(connection->fd_reader, &connection->buffer[connection->iRead], connection->empty);
    if (ret_read < 0) 
        PERROR("ReadToBuffer: Error in read\n")
    
    if (ret_read == 0) {
        close(connection->fd_reader);
        connection->fd_reader = -1;
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

void PrepareBuffer(struct Connection* connections, struct ChildInfo* childInfos,
							   const size_t nChild, const size_t numChilds)
{
	connections[nChild].buf_size = CountSize(nChild, numChilds);
	connections[nChild].buffer = (char*)calloc(connections[nChild].buf_size, 1);
	if (connections[nChild].buffer == NULL)
		PERROR("Parent: Can`t create buffer\n")

	connections[nChild].iRead = 0;
	connections[nChild].iWrite = 0;

	connections[nChild].busy = 0;
	connections[nChild].empty = connections[nChild].buf_size;

	connections[nChild].fd_reader = childInfos[nChild].fifoToPrnt[READ];

	if (nChild == numChilds -1)
		connections[nChild].fd_writer = STDOUT_FILENO;
	else
		connections[nChild].fd_writer = childInfos[nChild + 1].fifoFromPrnt[WRITE];
}

void WriteFromBuffer(struct Connection* connection, const int id)
{	
    size_t ret_write = write(connection->fd_writer, &connection->buffer[connection->iWrite], connection->busy);
    if (ret_write < 0 && errno != EAGAIN) 
        PERROR ("Impossible to write from buffer.\n")
        

    if (connection->iWrite >= connection->iRead)
        connection->empty += ret_write;

    if (connection->iWrite + ret_write == connection->buf_size) {
        connection->iWrite = 0;
        connection->busy = connection->iRead;
    }
    else {
        connection->iWrite += ret_write;
        connection->busy -= ret_write;
    }
}

size_t CountSize(const unsigned nChild, const unsigned numChilds)
{
	size_t buf_size = pow(3, numChilds - nChild);

	if (buf_size > MAXBUFFSIZE)
		return MAXBUFFSIZE;
	else 
		return buf_size;
}
