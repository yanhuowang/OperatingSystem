/*
* Name: Yan Huo
* Date: Oct 14, 2015
*
* sig.c does the following three things at the same time:
* 1. wait for input from user and echoes it back line-by-line
* 2. handle the signals: SIGINT, SIGTERM, SIGTSTP
* 3. heartbeat every 10 seconds
*
* Only main program can respond to signal, so I put heartbeat and read
* into two threads. And use pthread_mutex_lock to obtain thread-safe
*/

#include <stdio.h> 
#include <signal.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <pthread.h>

#define SIZE 100

char input_buf [10][SIZE]; // store the last ten lines
int line_position;
int heartbeat_time = 0;
pthread_mutex_t print_echo, print_heartbeat, print_summary;

// thread1: read from terminal
void *ptheaded_routine_read() {
    while(1) {
        fgets(input_buf[line_position], SIZE, stdin);
        input_buf[line_position][SIZE-1] = '\0';
        pthread_mutex_lock(&print_echo);
        printf("%s",input_buf[line_position]);
        pthread_mutex_unlock(&print_echo);
        line_position=(line_position+1) % 10;
    }
}

// thread2: heartbeat
void *ptheaded_routine_heartbeat() {
    struct timespec request, remain;
    int nanosleep_state = 0; // 0: completed -1: not completed  
    while(1) {          
        if (nanosleep_state == 0) {// if nanosleep completes
            pthread_mutex_lock(&print_heartbeat);
            printf("tick %d...\n", heartbeat_time);
            pthread_mutex_unlock(&print_heartbeat);
            heartbeat_time += 10;
            request.tv_sec = 10;
            request.tv_nsec = 0;
            nanosleep_state = nanosleep(&request, &remain);
        }
        else { // if nanosleep does not completes
            // actually, the thread won't be interrupted by signal
            // so we never get to this section
            request = remain;
            request.tv_sec = 0;
            request.tv_nsec = 0;
            nanosleep_state = nanosleep(&request, &remain);
        }
    }
}

// print a summary of all time spent by the program (system and user)
void print_program_summary () {
    struct rusage buf;
    getrusage(RUSAGE_SELF, &buf);
    pthread_mutex_lock(&print_summary);
    printf("total system time: %e\ntotal user time: %e\n",
        (double) buf.ru_stime.tv_sec + (double) buf.ru_stime.tv_usec 
                                                                / (double) 1e6,
        (double) buf.ru_utime.tv_sec + (double) buf.ru_utime.tv_usec 
                                                                / (double) 1e6);
    pthread_mutex_unlock(&print_summary);
    }

// print the last 10 lines of user input
void print_last_ten_lines () {
    int i = 0;
    for (i = 0; i < 10; i++) {
        printf("%s", input_buf[line_position]);
        line_position = (line_position + 1) % 10;
    }
}

// handle the signals: SIGINT, SIGTSTP and SIGTERM
void handler (int signal, siginfo_t *info, void *data) {
    switch (signal) {

        // if SIGINT, print program summary(sys and user) and continue
        case SIGINT:
            print_program_summary();
        break;
    
        // if SIGTERM, print program summary(sys and user) and exit
        case SIGTERM:
            print_program_summary();
        exit(0);
        break;

        // if SIGTSTP, print the last 10 lines of user input
        case SIGTSTP:
            print_last_ten_lines();
        break;
    }
}
main () {
    struct sigaction act, oldact;
    memset(&act, 0, sizeof(struct sigaction)); 
    act.sa_handler = (void *) handler;

    sigemptyset(&act.sa_mask); 
    sigaddset(&act.sa_mask, SIGINT); 
    sigaddset(&act.sa_mask, SIGTERM);
    sigaddset(&act.sa_mask, SIGTSTP); 
        
    act.sa_flags = 0; // no special flags
    sigaction(SIGINT, &act, &oldact); 
    sigaction(SIGTERM, &act, &oldact); 
    sigaction(SIGTSTP, &act, &oldact); ;

    pthread_t thread1, thread2; // thread1 read from terminal, thread 2 heatbeat
    pthread_create (&thread1, NULL, ptheaded_routine_read, NULL);
    pthread_create (&thread2, NULL, ptheaded_routine_heartbeat, NULL);
    pthread_join (thread1, NULL);
    pthread_join (thread2, NULL);
    return 0;
}
