#define _POSIX_C_SOURCE 199309 
#include <stdio.h> 
#include <stdlib.h> 
#include <sys/types.h>                                                                                    
#include <sys/stat.h>                                                                                     
#include <unistd.h>                                                                                       
#include <unistd.h>
#include <time.h>
#include <sys/time.h> 
#include <stdarg.h>
#include <sys/wait.h>

#define FALSE 0;
#define TRUE 1;
int done = FALSE;

// return the process state of the child
// R-run, S-sleep, D-disk waiting, Z-zombie, T-traced, W-paging
// return X if there is an error
char processState (pid_t childPid) {
    char* p;
        char path[100], line[100];
        char state;
        FILE* statusf;
        sprintf(path, "/proc/%ld/status", childPid);
        statusf = fopen(path,"r");

        if (!statusf){
            return 'x';
       }

        while(fgets(line, 100, statusf)) {
           // printf("%s\n", line[0]);
        if(strncmp(line, "State:", 6) != 0) 
            continue;
        p = line + 7;
        while(isspace(*p))
        ++p;
        state = p[0];
        break;
        }
        fclose(statusf);
        return state;
}

// make the process sleep for 1/100 second
void sleepTenMilliseconds () {
    struct timespec deadline;
    clock_gettime(CLOCK_MONOTONIC, &deadline);
    // add 10 millisecond
    deadline.tv_nsec += 10000000;
    // normalize the time to account for the second boundary
    if (deadline.tv_nsec >= 1000000000) {
        deadline.tv_nsec -= 1000000000;
        deadline.tv_sec++;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL);
}

// return the time between two timespec
double timeDifference (struct timespec start, struct timespec finish) {
    long seconds = finish.tv_sec - start.tv_sec;
    long ns = finish.tv_nsec - start.tv_nsec;
    if (start.tv_nsec > finish.tv_nsec) { // clock underflow
        --seconds;
        ns += 1000000000;
    }
    return (double) seconds + (double) ns / (double) 1000000000;
}


// add the timeDifference to the corresponding state
// if s2 == 'X' (error, e.g. the child already exited), do nothing
void addTimeToState (char state, double timeDiff, double* R, double* S, double* D, double* Z, double* T, double* W) {
    switch (state) {
        case 'R':
            *R += timeDiff;
            break;
        case 'S':
            *S += timeDiff;
            break;
        case 'D':
            *D += timeDiff;
            break;
        case 'Z':
            *Z += timeDiff;
            break;
        case 'T':
            *T += timeDiff;
            break;
        case 'W':
            *W += timeDiff;
            break;
    }
}

void usage(const char *program) { 
    printf("%s usage: %s program_to_measure\n", program, program); 
    exit(1); 
} 

void reaper (int sig) {
    int status;
    struct rusage usage;
    wait3 (&status, 0, &usage);
    done = TRUE;

}

int main(int argc, char **argv) {
    double R=0.0, S=0.0, D=0.0, Z=0.0, T=0.0, W=0.0, E=0.0; 
    double O=0.0; // extra credit 
 
    struct stat existence_check; 
    // check that file to watch exists 
    if (argc!=2 || stat(argv[1], &existence_check)!=0) usage(argv[0]); 
    // now we know the file exists.

    // run the program now 
    pid_t childPid;
    int status;
    signal (SIGCHLD, reaper);

    struct timespec t1, t2, t3; // record the time
    char s1, s2, s3; // record the state

    if ((childPid = fork())) { // parent
        clock_gettime(CLOCK_REALTIME, &t1);
        s1 = processState(childPid);
        sleepTenMilliseconds();
        clock_gettime(CLOCK_REALTIME, &t2);
        s2 = processState(childPid);

        // add the first slice to state s1
        double firstTimeSlice = timeDifference(t1, t2) / (double)2;
        addTimeToState(s1, firstTimeSlice, &R, &S, &D, &Z, &T, &W);
        sleepTenMilliseconds();
        
        while (!done) { // child still alive
                clock_gettime(CLOCK_REALTIME, &t3);
                s3 = processState(childPid);
                //  timeDifference = (t2-t1)/2-(t3-t2)/2 = (t3-t1)/2
                double timeDiff = timeDifference(t1, t3) / (double)2;       
                addTimeToState(s2, timeDiff, &R, &S, &D, &Z, &T, &W);
                t1 = t2;
                t2 = t3;
                s2 = s3;  
                sleepTenMilliseconds();          
        }
    }

    else { // child
         execl(argv[1], NULL);
    }

    if (done) {
        // when child exited
        // add the last slice to state s3
        double lastTimeSlice = timeDifference(t1, t2) / (double)2;
        addTimeToState(s3, lastTimeSlice, &R, &S, &D, &Z, &T, &W);

        E = R + S + D + Z + T + W;
        printf("R = %e\n", R); 
        printf("S = %e\n", S); 
        printf("D = %e\n", D); 
        printf("Z = %e\n", Z); 
        printf("T = %e\n", T); 
        printf("W = %e\n", W); 
        printf("E = %e\n", E); 
        signal (SIGCHLD, SIG_DFL);

    }
      
}
