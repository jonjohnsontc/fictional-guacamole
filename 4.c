/*
  Threaded variant. Steps I want to hit are

  - Create structure to hold all cities (likely one that preserves lexical order
(bi-tree, etc),)
- Mutexes will exist for every city entry
- Open file
- Create enough threads to saturate all cores (16?)
- Handoff pointer to location to threaded process
- threaded process will read in the line and ingest the city data
- final print and calculate process will be handled in main thread, after all
the input has been read

I think the threaded process is going to have some sort of queue
*/
#include <stdio.h>
#include <sys/sysinfo.h>
int main(void) {
  int num_procs = get_nprocs_conf();
  printf("Number of processors configured is %d\n", num_procs);
  return 0;
}