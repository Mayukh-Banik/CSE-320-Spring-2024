/*
 * Legion: Command-line interface
 */
#include "legion.h"
#include "temp.h"

void run_cli(FILE *in, FILE *out)
{
    struct stat st = {0};
    if (stat(LOGFILE_DIR, &st) == -1)    
    {
        mkdir(LOGFILE_DIR, 0755);
    }
    if (in == NULL || out == NULL)
    {
        fprintf(stderr, "Error");
        fflush(stderr);
        exit(1);
    }
    if ((inputFileNo = fileno(in)) == -1)
    {
        sf_error("Can't get in file number");
        return;
    }
    if ((outputFileNo = fileno(out)) == -1)
    {
        sf_error("Can't get out file number");
        return;
    }
    
    outputFile = out;
    inputFile = in;

    // signal(SIGALRM, alarm_handler);
    // signal(SIGCHLD, child_handler);

    struct sigaction sa;

    // Setup for SIGALRM
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_RESTART | SA_SIGINFO;
    sa.sa_sigaction = alarm_handler;  
    sigaction(SIGALRM, &sa, NULL);

    
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO | SA_RESTART;  
    sa.sa_sigaction = child_handler;  
    sigaction(SIGCHLD, &sa, NULL);

    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO | SA_RESTART;  
    sa.sa_sigaction = sigint_handler;  
    sigaction(SIGINT, &sa, NULL);

    // sigset_t mask;
    // sigemptyset(&mask);
    // sigaddset(&mask, SIGALRM);
    // sigprocmask(SIGALRM, &mask, NULL);

    // sigaddset(&mask, SIGCHLD);
    // sigprocmask(SIGCHLD, &mask, NULL);
    

    int bytesWritten = 0;
    size_t userInputLength = 0;
    long long int readCheckSum = 0;

    while (1)
    {
        while (currIndx >= 0)
        {
            sigset_t tm, tm2;
            sigfillset(&tm);
            sigdelset(&tm, SIGINT);
            sigprocmask(SIG_BLOCK, &tm, &tm2);
            DEMON* temp = getDemonBasedOnPID(pidStatus[currIndx]);
            if (temp == NULL)
            {
                currIndx = currIndx - 1;
                continue;
            }
            if (WIFEXITED(pidStatus[currIndx]))
            {
                sf_term(temp->name, pidPid[currIndx], pidStatus[currIndx]);
                temp->statusNumber = Exited;
            }
            else
            {
                sf_crash(temp->name, pidPid[currIndx], WTERMSIG(pidPid[currIndx]));
                temp->statusNumber = Crashed;
            }
            currIndx = currIndx - 1;
            sigprocmask(SIG_SETMASK, &tm, NULL);
        }
        currIndx = -1;
        if (intRecieved == 0)
        {
            for (int i = 0; i < demonListLength; i++)
            {
                if (demonList[i] != NULL)
                {
                    if (isDemonActive(demonList[i]->statusNumber))
                    {
                        stop(demonList[i]->name);
                    }
                }
            }
            quit();
            return;
        }
        sf_prompt();
        bytesWritten = write(outputFileNo, LEGION_PRINTOUT, 8);
        if (bytesWritten == -1)
        {
            free(rawUserInput);
        }
        readCheckSum = getline(&rawUserInput, &userInputLength, in);
        if (readCheckSum == -1)
        {
            FreeUserInput();
            FreeFields();
            sf_error("Error getting user input");
            return;
        }
        if (readCheckSum < 4)
        {
            free(rawUserInput);
            userInputLength = 0;
            rawUserInput = NULL;
            sf_error("Invalid argument");
            continue;
        }
        rawUserInput[readCheckSum - 1] = rawUserInput[readCheckSum - 1] == '\n' ? '\0' : rawUserInput[readCheckSum - 1];
        fieldCount = 0;
        char* field_start = rawUserInput;
        int in_quotes = false;
        for (char* p = rawUserInput; *p; p++)
        {
            if (*p == '\'' && in_quotes == 0) 
            {
                in_quotes = 1;
                field_start = p + 1;
            } 
            else if (*p == '\'' && in_quotes == 1) 
            {
                fields = add_field(fields, field_start, p - field_start);
                in_quotes = 0;
                field_start = p + 1;
            } 
            else if ((*p == ' ' && in_quotes == 0)) 
            {
                if (p > field_start) 
                {
                    fields = add_field(fields, field_start, p - field_start);
                }
                field_start = p + 1;
            }
        }
        if (*field_start != '\'' && *field_start != '\n' && *field_start != '\0') 
        {
            fields = add_field(fields, field_start, strlen(field_start));
        }
        int argNumber = validArg(fields, fieldCount);

        switch (argNumber)
        {
            case 0:
                write(outputFileNo, HELP_PRINTOUT, 368);
                break;
            case 1:
                if (demonList != NULL)
                {
                    for (int i = 0; i < demonListLength; i++)
                    {
                        if (demonList[i] != NULL)
                        {
                            if (isDemonActive(demonList[i]->statusNumber))
                            {
                                stop(demonList[i]->name); 
                            }
                        }
                    }
                }
                quit();
                return;
            case 2:
                regist();
                break;
            case 3:
                unregister(fields[1], out);
                break;
            case 4:
                status();
                break;
            case 5:
                statusAll(out);
                break;
            case 6:
                start();
                break;
            case 7:
                stop(fields[1]);
                break;
            case 8:
                logrotate(fields[1]);
                break;
            default:
                fprintf(out, "Error in argument\n");
                fflush(out);
                sf_error("Invalid Argument");
                fprintf(out, "Error executing commands\n");
                fflush(out);
                break;
        }
        FreeFields();
    }
    FreeUserInput();
}

void logrotate(char* name)
{
    DEMON* demon = findDemon(name);
    if (demon == NULL)
    {
        sf_error("Demon of that name doesn't exist");
        return;
    }
    ssize_t stringLength = strlen(LOGFILE_DIR) + strlen("/") + strlen(demon->name) + strlen(".log.") + sizeof(char) + 1;
    char* maxFile = calloc(stringLength, 1);
    char* oldFile = calloc(stringLength, 1);
    sprintf(maxFile, "%s/%s.log.%c", LOGFILE_DIR, demon->name, LOG_VERSIONS + '0' - 1);
    if (access(maxFile, F_OK) == 0)
    {
        unlink(maxFile);
    }
    // for (int i = LOG_VERSIONS - 1; i > 0; i--)
    // {
    //     char newVersion = i + '0';
    //     char oldVersion = newVersion - 1;
    //     oldFile[stringLength - 2] = oldVersion;
    //     maxFile[stringLength - 2] = newVersion;
    //     // sprintf(oldFile, "%s/%s.log.%c", LOGFILE_DIR, demon->name, oldVersion);
    //     // sprintf(maxFile, "%s/%s.log.%c", LOGFILE_DIR, demon->name, newVersion);
    //     if (access(oldFile, F_OK) == 0)
    //     {
    //         rename(oldFile, maxFile);
    //     }
    // }

    sf_logrotate(demon->name);
    stop(demon->name);
    setDemonInactive(demon);
    for (int i = LOG_VERSIONS - 1; i > 0; i--)
    {
        char newVersion = i + '0';
        char oldVersion = newVersion - 1;
        // oldFile[stringLength - 2] = oldVersion;
        // maxFile[stringLength - 2] = newVersion;
        sprintf(oldFile, "%s/%s.log.%c", LOGFILE_DIR, demon->name, oldVersion);
        sprintf(maxFile, "%s/%s.log.%c", LOGFILE_DIR, demon->name, newVersion);
        if (access(oldFile, F_OK) == 0)
        {
            rename(oldFile, maxFile);
        }
    }
    free(maxFile);
    free(oldFile);
    start();
    // fields[1] = realloc(fields[1], strlen(demon.))
}


void stop(char* name)
{
    // char* name = fields[1];
    DEMON* demon = findDemon(name);
    // int demonIndex = getDemonIndex(demon);
    if (demon == NULL)
    {
        sf_error("Can't find demon of that name");
        return;
    }
    if (isDemonExited(demon->statusNumber) || isDemonCrashed(demon->statusNumber))
    {
        setDemonInactive(demon);
        sf_reset(demon->name);
        return;
    }
    if (!isDemonActive(demon->statusNumber))
    {
        sf_error("Demon isn't active");
        return;
    }
    setDemonStopping(demon);
    childSentSigTerm = demon->id;
    // currentChildId = demon->id;
    currChildStop = demon->id;
    kill(demon->id, SIGTERM);
    sf_stop(demon->name, demon->id);

    sigset_t mask;
    sigfillset(&mask);
    sigdelset(&mask, SIGALRM);
    sigdelset(&mask, SIGCHLD);
    alarm(CHILD_TIMEOUT);
    
    sigsuspend(&mask);
    if (childTerminated == true)
    {
        sf_term(demon->name, demon->id, exitStatus);
        setDemonExited(demon);
    }
    else
    {
        sf_kill(demon->name, demon->id);
        setDemonCrashed(demon);
    }
    demon->id = 0;
    childTerminated = false;    
    childSentSigTerm = 0;
}

void setDemonStatus(DEMON* demon, int val)
{
    demon->statusNumber = val;
    // demon->status = realloc(demon->status, strlen(STATUS[val]) + 1);
    // strcpy(demon->status, STATUS[val]);
}

void start()
{
    char* name = fields[1];
    FILE* fp = outputFile;
    DEMON* demon = findDemon(name);
    currentDemonIndex = getDemonIndex(demon);
    if (demon == NULL)
    {
        sf_error("Demon of that name doesn't exist");
        return;
    }

    sf_start(demon->name);
    if (demon == NULL)
    {
        fprintf(fp, "Demon %s doesn't exist", name);
        fflush(fp);
        sf_error("Demon doesn't exist");
        fprintf(fp, "Error executing command: start");
        fflush(fp);
    }
    if (!isDemonInactive(demon->statusNumber))
    // if (strcmp(demon->status, STATUS[1]))
    {
        sf_error("Demon isn't inactive");
    }
    setDemonStarting(demon);
    // setDemonStatus(demon, 2);

    // int demon->fd[2];
    pid_t cpid;
    if (pipe(demon->fd) == -1)
    {
        sf_error("Error creating pipe");
        setDemonStatus(demon, 1);
        return;
    }
    sigset_t set;
    sigemptyset(&set);
    sigaddset(&set, SIGCHLD);
    cpid = fork();
    if (cpid == -1)
    {
        sf_error("Forking error");
        setDemonInactive(demon);
        // setDemonStatus(demon, 1);
        return;
    }
    if (cpid == 0)
    {
        close(demon->fd[0]);
        char version = '0';
        char logFilePath[strlen(LOGFILE_DIR) + strlen("/") + strlen(name) + strlen(".log.") + sizeof(char) + 1];
        sprintf(logFilePath, "%s/%s.log.%c", LOGFILE_DIR, name, version);
        int logFileDescriptor = open(logFilePath, O_CREAT | O_APPEND | O_WRONLY, 0777);

        if (logFileDescriptor == -1)
        {
            sf_error("opening log file");
            exit(EXIT_FAILURE);
        }

        if (dup2(logFileDescriptor, STDOUT_FILENO) == -1)
        {
            sf_error("Error dup2 log file");
            exit(EXIT_FAILURE);
        }
        close(logFileDescriptor);

        if (dup2(demon->fd[1], SYNC_FD) == -1)
        {
            sf_error("Error dup2 pipe to parent");
            exit(EXIT_FAILURE);
        }
        close(demon->fd[1]);

        char *path_env = getenv("PATH");
        char new_path[strlen(DAEMONS_DIR) + strlen(path_env) + 2];
        sprintf(new_path, "%s:%s", DAEMONS_DIR, path_env);
        setenv("PATH", new_path, 1);
        char* daemon_args[demon->numArgs + 2];
        daemon_args[0] = demon->executable;
        for (int i = 0; i < demon->numArgs; i++)
        {
            daemon_args[i + 1] = demon->args[i];
        }

        setpgid(0, 0);
        daemon_args[demon->numArgs + 1] = NULL;
        struct sigaction sa;
        memset(&sa, 0, sizeof(sa));
        sa.sa_handler = SIG_DFL; // Set to default signal handling

        // Reset handlers for SIGALRM, SIGCHLD, and SIGINT
        sigaction(SIGALRM, &sa, NULL);
        sigaction(SIGCHLD, &sa, NULL);
        sigaction(SIGINT, &sa, NULL);
        execvpe(demon->executable, daemon_args, environ);

        // sf_error("Child crashed"); 
        exit(EXIT_FAILURE);
    }
    else
    {
        sigprocmask(SIG_BLOCK, &set, NULL);
        close(demon->fd[1]);
        demon->id = cpid;
        currentChildId = cpid;
        char syncSignal[1];
        alarm(CHILD_TIMEOUT);
        if (read(demon->fd[0], syncSignal, 1))
        {
            alarm(0);
            setDemonActive(demon);
            // setDemonStatus(demon, 3);
            sf_active(demon->name, cpid);
            currentChildId = -1;
            currentDemonIndex = -1;
        }
        else if (currentChildId > -1)
        {
            alarm(0);
            sf_kill(demon->name, cpid);
            // sf_error()
            setDemonCrashed(demon);
            currentChildId = -1;
            currentDemonIndex = -1;
            // exit(2);
        }
        close(demon->fd[0]);
        sigprocmask(SIG_UNBLOCK, &set, NULL);
        // exit(2);
    }
}

void alarm_handler(int signum, siginfo_t* info, void* context)
{
    kill(currentChildId, SIGKILL);
}

void child_handler(int signum, siginfo_t* info, void* context)
{
    int oe = errno;
    // alarm(0);
    int status;
    // int a = info->si_pid;
    pid_t child_pid;

    while ((child_pid = waitpid(-1, &status, WNOHANG)) > 0)
    {
        if (currChildStop == child_pid)
        {
            childTerminated = true;
        }
        currIndx++; 
        pidPid[currIndx] = child_pid; 
        pidStatus[currIndx] = status;
    }
    errno = oe;
}

void sigint_handler(int signum, siginfo_t* info, void* context)
{
        intRecieved = 0;
    sigset_t set;
    sigfillset(&set);
    sigprocmask(SIG_SETMASK, &set, NULL);
    for (int i = 0; i < demonListLength; i++)
    {
        if (demonList[i] != NULL)
        {
            if (isDemonActive(demonList[i]->statusNumber))
            // if (isDemonActive(demonList[i]->status))
            {
                stop(demonList[i]->name);
            }
        }
    }
    sf_fini();
    _exit(0);

}

DEMON* findDemon(char* name)
{
    for (int i = 0; i < demonListLength; i++)
    {
        if (!strcmp(demonList[i]->name, name))
        {
            return demonList[i];
        }
    }
    return NULL;
}

void unregister(char* name, FILE* fp)
{
    for (int i = 0; i < demonListLength; i++)
    {
        if (!strcmp(demonList[i]->name, name) && isDemonInactive(demonList[i]->statusNumber))
        {
            sf_unregister(name);
            FreeDemon(demonList[i]);
            demonList[i] = NULL;
            return;
        }
        else if (!strcmp(demonList[i]->name, name) && !isDemonInactive(demonList[i]->statusNumber))
        {
            fprintf(fp, "Daemon %s is not inactive\n", name);
            fflush(fp);
            sf_error("Unregister");
            fprintf(fp, "Error executing command: unregister\n");
            fflush(fp);
            return;
        }
    }
    fprintf(fp, "Daemon %s is not registered\n", name);
    fflush(fp);
    sf_error("Unregister");
    fprintf(fp, "Error executing command: unregister\n");
    fflush(fp);
}

void printStatus(char* name, int no, FILE* fp)
{
    for (int i = 0; i < demonListLength; i++)
    {
        if (demonList[i] != NULL && !strcmp(demonList[i]->name, name))
        {
            int y = strlen(demonList[i]->name) + strlen(STATUS[demonList[i]->statusNumber]) + 4;
            char* yy = calloc(1, y);
            snprintf(yy, y, "%s\t%d\t%s", demonList[i]->name, demonList[i]->id, STATUS[demonList[i]->statusNumber]);
            sf_status(yy);
            free(yy);
            // sf_status(demonList[i]->name);
            // fprintf(fp, "%s\t%d\t%s\n", demonList[i]->name, demonList[i]->id, demonList[i]->status);
            // fflush(fp);
            return;
        }
    }
    fprintf(fp, "Daemon %s is not registered\n", name);
    fflush(fp);
    sf_error("Unregister");
    fprintf(fp, "Error executing command: status\n");
}

void statusAll(FILE* fp)
{
    for (int i = 0; i < demonListLength; i++)
    {
        if (demonList[i] != NULL)
        {
            int y = strlen(demonList[i]->name) + strlen(STATUS[demonList[i]->statusNumber]) + 4;
            char* yy = calloc(1, y);
            snprintf(yy, y, "%s\t%d\t%s", demonList[i]->name, demonList[i]->id, STATUS[demonList[i]->statusNumber]);
            sf_status(yy);
            free(yy);
            // sf_status("lazy\t0\tactive");
            // sf_status(demonList[i]->name);
            // fprintf(fp, "%s\t%d\t%s\n", demonList[i]->name, demonList[i]->id, demonList[i]->status);
            // fflush(fp);
            return;
        }
    }
}

void createAndInsertDemon(char** fields, int fds, FILE* fp)
{
    if (demonListLength != 0)
    {
        for (int i = 0; i < demonListLength; i++)
        {
            if (!strcmp(fields[1], demonList[i]->name))
            {
                fprintf(fp, "Demon of that name is already registered");
                fflush(fp);
                sf_error("Same name as a previous demon");
                return;
            }
        }
    }
    DEMON* demon = malloc(sizeof(DEMON));
    demon->name = malloc(strlen(fields[1]) + 1);
    strcpy(demon->name, fields[1]);
    demon->executable = malloc(strlen(fields[2]) + 1);
    strcpy(demon->executable, fields[2]);
    demon->args = NULL;
    if (fds > 3)
    {
        demon->numArgs = fds - 3;
        demon->args = malloc(sizeof(char*) * demon->numArgs);
        for (int i = 0; i < demon->numArgs; i++)
        {
            demon->args[i] = malloc(strlen(fields[i + 3]) + 1);
            strcpy(demon->args[i], fields[i + 3]);
        }
    }
    demon->id = 0;
    // demon->status = malloc(strlen(STATUS[1]) + 1);
    // strcpy(demon->status, STATUS[1]);
    for (int i = 0; i < demonListLength; i++)
    {
        if (demonList[i] == NULL)
        {
            demonList[i] = demon;
            return;
        }
    }
    demonList = realloc(demonList, sizeof(DEMON*) * (demonListLength + 1));
    demonList[demonListLength] = demon;
    demonListLength++;
    // demon->sentSigTerm = false;
    demon->statusNumber = Inactive;
}

void regist()
{
    sf_register(fields[1], fields[2]);
    createAndInsertDemon(fields, fieldCount, outputFile);
}

void unreg()
{
    unregister(fields[1], outputFile);
}

void status()
{
    if (fields[1] == NULL)
    {
        sf_error("Demon can't be found");
    }
    DEMON* demon = findDemon(fields[1]);
    if (demon == NULL)
    {
        sf_error("Can't find Demon of that name");
    }
    printStatus(fields[1], outputFileNo, outputFile);
    // int y = strlen(demon->name) + strlen(demon->status) + 4;
    // char* yy = calloc(1, y);
    // snprintf(yy, y, "%s\t%d\t%s", demonList[i]->name, demonList[i]->id, demonList[i]->status);
    // sf_status(yy);
    // free(yy);
}

void FreeFields()
{
    for (int i = 0; i < fieldCount; i++)
    {
        free(fields[i]);
        fields[i] = NULL;
    }
    free(fields);
    fields = NULL;
    fieldCount = 0;
}

int validArg(char** fields, int fieldCount)
{
    if (!strcmp("help", fields[0]))
    {
        return 0;
    }
    else if (!strcmp("quit", fields[0]))
    {
        return 1;
    }
    else if (!strcmp("register", fields[0]))
    {
        if (fieldCount < 3)
        {
            return -1;
        }
        return 2;
    }
    else if (!strcmp("unregister", fields[0]))
    {
        if (fieldCount != 2)
        {
            return -1;
        }
        return 3;
    }
    else if (!strcmp("status", fields[0]))
    {
        if (fieldCount != 2)
        {
            return -1;
        }
        return 4;
    }
    else if (!strcmp("status-all", fields[0]))
    {
        return 5;
    }
    else if (!strcmp("start", fields[0]))
    {
        if (fieldCount != 2)
        {
            return -1;
        }
        return 6;
    }
    else if (!strcmp("stop", fields[0]))
    {
        if (fieldCount != 2)
        {
            return -1;
        }
        return 7;
    }
    else if (!strcmp("logrotate", fields[0]))
    {
        if (fieldCount != 2)
        {
            return -1;
        }
        return 8;
    }
    return -1;
}

char **add_field(char **fields, char *start, int length) 
{
    fields = realloc(fields, (fieldCount + 1) * sizeof(char*));
    fields[fieldCount] = calloc(1, length + 1);
    strncpy(fields[fieldCount], start, length);
    fields[fieldCount][length] = 0;
    fieldCount++;
    return fields;
}

void FreeDemon(DEMON* demon)
{
    free(demon->name);
    demon->name = NULL;
    free(demon->executable);
    demon->executable = NULL;
    for (int i = 0; i < demon->numArgs; i++)
    {
        free(demon->args[i]);
        demon->args[i] = NULL;
    }
    free(demon->args);
    demon->args = NULL;
    // free(demon->status);
    // demon->status = NULL;
    free(demon);
}

void FreeDemonList()
{
    for (int i = 0; i < demonListLength; i++)
    {
        if (demonList[i] != NULL)
        {
            FreeDemon(demonList[i]);
            demonList[i] = NULL;
        }
    }
    free(demonList);
    demonList = NULL;
}

void FreeUserInput()
{
    free(rawUserInput);
    rawUserInput = NULL;
}

void quit()
{
    FreeFields();
    FreeUserInput();
    FreeDemonList();
}

void printDemon(DEMON *demon) 
{
    debug("\nName: %s", demon->name);
    debug("Executable: %s", demon->executable);
    debug("Args:");
    if (demon->numArgs > 0 && demon->args != NULL) {
        for (int i = 0; i < demon->numArgs; i++) {
            debug("  %d: %s", i, demon->args[i]);
        }
    } else {
        debug("  No arguments");
    }
    debug("Number of Args: %d", demon->numArgs);
    debug("ID: %d", demon->id);
    // debug("Status: %s", demon->status);
    // debug("Status number")
}










