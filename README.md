# OperatingSystem
## 1. Measure time:
lens.c executes a given process to tis completion, and then reports basic time measurements. (Returns the time spent on R-run, S-sleep, D-disk waiting, Z-zombie, T-traced, W-paging.)

## 2. Multi-threads:
sig.c does the following three things at the same time:

 - wait for input from user and echoes it back line-by-line
 - handle the signals: SIGINT, SIGTERM, SIGTSTP
 - heartbeat every 10 seconds

