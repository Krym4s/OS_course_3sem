enum ErrorTypes
{
    NO_ERRORS,
    INVALID_ARGS,
    NO_FILE,
    NOT_OPENED,
    WRITE_FAIL,
    READ_FAIL,
    FIFO_NOT_CREATED,
    FAIL_FCNTL,
    NO_WRITER
};

int ErrorCheck (int error, const char* errmessage);