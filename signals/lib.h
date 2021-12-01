#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <sys/prctl.h>

#pragma once

#define PERROR(string) {perror (string);fflush (stderr);exit(1);}

const int nBits = 8;

void Handler_USR1(int sig);
void Handler_USR2(int sig);
void Handler_CHLD_Died(int sig);
void Handler_Prnt_Wait(int sig);
void Handler_Prnt_Died(int sig);

void RunChild(char* filename, const pid_t ppid);
void RunParent(const pid_t child_pid);