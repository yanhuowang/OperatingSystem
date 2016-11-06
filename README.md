# OperatingSystem
## 1. Measure Process States and Time:
lens.c executes a given process to tis completion, and then reports basic time measurements. (Returns the time spent on R-run, S-sleep, D-disk waiting, Z-zombie, T-traced, W-paging.)


## 2. Multi-threads:
sig.c does the following three things at the same time:

 - wait for input from user and echoes it back line-by-line
 - handle the signals: SIGINT, SIGTERM, SIGTSTP
 - heartbeat every 10 seconds


## 3. Mutexes and Semaphores

### Overview
 There are 11 aardvarks and 3 anthills. There are 100 ants in each anthill. Only 3 aardvarks can share one anthill at a time. It takes one second of real time for an aardvark to slurp up an ant, and another second for the aardvark to swallow it, during which time it is not using the anthill and another aardvark can start slurping. However, should an aardvark attempt to slurp an ant from an anthill where 3 aardvarks are already slurping, or make any other kind of mistake, including attempting to slurp from an already empty anthill or an anthill that doesn't exist, it will be 4 seconds before the aardvark is available to slurp again. 
 
 ### Objective
 Manage the aardvarks so that all of the ants are consumed in a minimal amount of real time. Each aardvark is a thread in a multi-threaded program.

