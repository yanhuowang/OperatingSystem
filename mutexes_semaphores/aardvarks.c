#include "anthills.h" 
#include <semaphore.h>

#define TRUE 1
#define FALSE 0

int initialized=FALSE; // semaphores and mutexes are not initialized 

// define mutexes and semaphores (global variables).
sem_t sem_remain_positions [ANTHILLS];
sem_t sem_remain_ants [ANTHILLS]; 

// actually sleep 1.1s
void sleepOneSecond () {
    struct timespec deadline;
    clock_gettime(CLOCK_MONOTONIC, &deadline);
    // add 1.1 second
    deadline.tv_sec += 1;
    deadline.tv_nsec += 100000000;
    // normalize the time to account for the second boundary
    if (deadline.tv_nsec >= 1000000000) {
        deadline.tv_nsec -= 1000000000;
        deadline.tv_sec++;
    }
    clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &deadline, NULL);
}

// sleep for 1 second, then sem_post, (other aardvarks can eat)
void *ptheaded_routine_timer (void *x_void_ptr) {
    int *anthill_ptr = (int *) x_void_ptr;
    int anthill = *anthill_ptr;
    sleepOneSecond ();
    sem_post (&sem_remain_positions[anthill]);
}

int my_slurp (aname, anthill) {
    int result = -1;
    int anthill_num = anthill;
    if (sem_trywait(&sem_remain_positions[anthill_num]) == 0) {
        if (sem_trywait(&sem_remain_ants[anthill_num]) == 0) {
            pthread_t timer;
            pthread_create (&timer, NULL, ptheaded_routine_timer, &anthill_num);
            result = slurp (aname, anthill_num);
            pthread_join (timer, NULL);
            if (result == FALSE) {
                sem_post (&sem_remain_ants[anthill]);
            }
        } 
        else { 
	    sem_post (&sem_remain_positions[anthill_num]);
        }
    }
    return result;  
}

// first thread initializes mutexes 
void *aardvark(void *input) { 
    char aname = *(char *)input; 
    // first caller need to initialize the mutexes!
    if (!initialized++) { // this succeeds only for one thread
	// initialize your mutexes and semaphores
        int i = 0;
        for (i = 0; i < ANTHILLS; i++) {
            sem_init (&sem_remain_positions[i], 0, AARDVARKS_PER_HILL);
            sem_init (&sem_remain_ants[i], 0, ANTS_PER_HILL);
        }
    } 

    // now be an aardvark
    while (chow_time()) { 
        // try to slurp
        int i = 0;
        for (i = 0; i < ANTHILLS; i++) {
            my_slurp(aname, i);
        }
    }
    return NULL; 
} 
