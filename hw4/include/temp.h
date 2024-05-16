#ifndef TEMP_H
#define TEMP_H

#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <debug.h>
#include <stdio.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> 
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>

typedef enum Status
{
    Unknown = 0,
    Inactive = 1,
    Starting = 2,
    Active = 3,
    Stopping = 4,
    Exited = 5,
    Crashed = 6,
} Status;

typedef struct DEMON {
    char* name;
    char* executable;
    char** args;
    int numArgs;
    pid_t id;
    // char* status;
    int fd[2];
    // bool sentSigTerm;
    volatile sig_atomic_t statusNumber;
    // Status statusNumber;
} DEMON;

DEMON** demonList = NULL;
volatile sig_atomic_t demonListLength = 0;


const char* STATUS[] = {"unknown", "inactive", "starting", "active", "stopping", "exited", "crashed"};
const char* HELP_PRINTOUT = "Available commands:\nhelp (0 args) Print this help message\nquit (0 args) Quit the program\nregister (0 args) Register a daemon\nunregister (1 args) Unregister a daemon\nstatus (1 args) Show the status of a daemon\nstatus-all (0 args) Show the status of all daemons\nstart (1 args) Start a daemon\nstop (1 args) Stop a daemon\nlogrotate (1 args) Rotate log files for a daemon\n";
const char* LEGION_PRINTOUT = "legion> ";
char* rawUserInput = NULL;
char** fields = NULL;
int fieldCount = 0;

extern char** environ;

char **add_field(char **fields, char *start, int length);
int validArg(char** fields, int fieldCount);
// void fFreeFields(char** fields, int fieldCount);
void createAndInsertDemon(char** fields, int fieldcount, FILE* fp);
void printStatus(char* name, int no, FILE* fp);
void unregister(char* name, FILE* fp);
void statusAll(FILE* fp);
void printDemon(DEMON *demon);
DEMON* findDemon(char* name);
// void start(char* name, FILE* fp);
/**
 * 0 - unkonwn, 1 inactive, 2 starting, 3 active, 4 stopping, 5 exited, 6 crashed
 */
void setDemonStatus(DEMON* demon, int val);

void FreeDemon(DEMON* demon);
void FreeDemonList();
void FreeFields();
void FreeUserInput();

void quit();
void regist();
void unreg();
void status();
void start();
void stop(char* name);
void logrotate(char* name);


void alarm_handler(int signum, siginfo_t* info, void* context);
void child_handler(int signum, siginfo_t* info, void* context);
void sigint_handler(int signum, siginfo_t* info, void* context);

volatile sig_atomic_t currentChildId = -1;
volatile sig_atomic_t currentDemonIndex = -1;

FILE* outputFile = NULL;
FILE* inputFile = NULL;

int inputFileNo;
int outputFileNo;

volatile sig_atomic_t intRecieved = -1;

volatile sig_atomic_t childSentSigTerm = -1;

volatile sig_atomic_t childTerminated = false;
volatile sig_atomic_t exitStatus = -1;

volatile sig_atomic_t pidPid[1024] = {0};
volatile sig_atomic_t pidStatus[1024] = {0};
volatile sig_atomic_t currIndx = -1;

volatile sig_atomic_t currChildStop = -1;
















































DEMON* getDemonBasedOnPID(pid_t pid)
{
    for (int i = 0; i < demonListLength; i++)
    {
        if (demonList[i]->id == pid)
        {
            return demonList[i];
        }
    }
    return NULL;
}









int getDemonIndex(DEMON* demon)
{
    for (int i = 0; i < demonListLength; i++)
    {
        if (demon == demonList[i])
        {
            return i;
        }
    }
    return -1;
}

int isDemonUnknown(int val) {
    return val == 0 ? 1 : 0;
}

int isDemonInactive(int val) {
    return val == 1 ? 1 : 0;
}

int isDemonStarting(int val) {
    return val == 2 ? 1 : 0;
}

int isDemonActive(int val) {
    return val == 3 ? 1 : 0;
}

int isDemonStopping(int val) {
    return val == 4 ? 1 : 0;
}

int isDemonExited(int val) {
    return val == 5 ? 1 : 0;
}

int isDemonCrashed(int val) {
    return val == 6 ? 1 : 0;
}

void setDemonUnknown(DEMON* demon) {
    setDemonStatus(demon, 0); // 'unknown' status index
}

void setDemonInactive(DEMON* demon) {
    setDemonStatus(demon, 1); // 'inactive' status index
}

void setDemonStarting(DEMON* demon) {
    setDemonStatus(demon, 2); // 'starting' status index
}

void setDemonActive(DEMON* demon) {
    setDemonStatus(demon, 3); // 'active' status index
}

void setDemonStopping(DEMON* demon) {
    setDemonStatus(demon, 4); // 'stopping' status index
}

void setDemonExited(DEMON* demon) {
    setDemonStatus(demon, 5); // 'exited' status index
}

void setDemonCrashed(DEMON* demon) {
    setDemonStatus(demon, 6); // 'crashed' status index
}

#endif
