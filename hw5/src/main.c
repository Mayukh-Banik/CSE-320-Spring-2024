#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "debug.h"
#include "server.h"
#include "globals.h"
#include "Custom.h"
#include "csapp.h"

static void terminate(int);
void sighup_handler(int signum, siginfo_t* info, void* context);
volatile sig_atomic_t KMS = 0;
/*
 * "Charla" chat server.
 *
 * Usage: charla <port>
 */
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen.
    int portValue = -1, opt;
    char* portStr = NULL;
    while ((opt = getopt(argc, argv, "dqp:h:")) != -1)
    {
        switch (opt)
        {
            case 'p':
                errno = 0;
                char* endptr;
                portValue = (int) strtol(optarg, &endptr, 10);
                if (errno != 0 || endptr == optarg)
                {
                    fprintf(stderr, "Error converting port number\n");
                    if (userSpecifiedHostName != NULL) 
                    {
                        free(userSpecifiedHostName);
                    }
                    exit(EXIT_FAILURE);
                }
                portStr = strdup(optarg);
                break;
            case 'h':
                userSpecifiedHostName = strdup(optarg);
                break;
            case 'd':
                DEBUG_MODE = true;
                break;
            case 'q':
                QUIET_MODE = true;
                break;
            default:
                break;
        }
    }
    if (portValue < 0) 
    {
        fprintf(stderr, "Port value >= 0 has to be specified.\n");
        exit(EXIT_FAILURE);
    }
    if (userSpecifiedHostName == NULL)
    {
        userSpecifiedHostName = strdup("127.0.0.1");
    }
    // Sighup handler
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = sighup_handler;  
    if (sigaction(SIGHUP, &sa, NULL) == -1)
    {
        fprintf(stderr, "Sigaction error");
        exit(EXIT_FAILURE);
    }
    // Perform required initializations of the client_registry and
    // player_registry.
    user_registry = ureg_init();
    client_registry = creg_init();
    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function charla_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.
    int listenfd, *connfdp;
    socklen_t clientlen;
    struct sockaddr_storage clientaddr;
    pthread_t tid;
    listenfd = Open_listenfd(portStr);
    while (KMS == 0) 
    {
        clientlen = sizeof(struct sockaddr_storage);
        connfdp = Malloc(sizeof(int));
        *connfdp = accept(listenfd, (SA*) &clientaddr, &clientlen);
        if (KMS != 0)
        {
            close(listenfd);
            free(connfdp);
            break;
        }
        Pthread_create(&tid, NULL, chla_client_service, connfdp);
    }
    free(portStr);
    free(userSpecifiedHostName);
    terminate(EXIT_SUCCESS);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    // Shut down all existing client connections.
    // This will trigger the eventual termination of service threads.
    creg_shutdown_all(client_registry);

    // Finalize modules.
    creg_fini(client_registry);
    ureg_fini(user_registry);

    debug("%ld: Server terminating", pthread_self());
    exit(status);
}

void sighup_handler(int signum, siginfo_t* info, void* context)
{
    KMS = 1;
}
