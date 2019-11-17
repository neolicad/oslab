#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <sys/wait.h>

#define HZ	100

void cpuio_bound(int last, int cpu_time, int io_time);

int main(int argc, char * argv[])
{
  pid_t child_pid, wpid;
  int status = 0;
  if ((child_pid = fork()) == 0) {
    cpuio_bound(20, 1, 0);
    exit(0);
  }
  printf("Created child with pid: %d\n", child_pid);
  if ((child_pid = fork()) == 0) {
    cpuio_bound(20, 9, 1);
    exit(0);
  }
  printf("Created child with pid: %d\n", child_pid);
  if ((child_pid = fork()) == 0) {
    cpuio_bound(20, 1, 1);
    exit(0);
  }
  printf("Created child with pid: %d\n", child_pid);
  if ((child_pid = fork()) == 0) {
    cpuio_bound(20, 1, 9);
    exit(0);
  } 
  printf("Created child with pid: %d\n", child_pid);
  if ((child_pid = fork()) == 0) {
    cpuio_bound(20, 0, 1);
    exit(0);
  }
  printf("Created child with pid: %d\n", child_pid);
  while((wpid = wait(&status)) > 0) {
    printf("Reaped child with pid: %d\n", wpid);
  }
  return 0;
}

void cpuio_bound(int last, int cpu_time, int io_time)
{
	struct tms start_time, current_time;
	clock_t utime, stime;
	int sleep_time;

	while (last > 0)
	{
		/* CPU Burst */
		times(&start_time);
		do
		{
			times(&current_time);
			utime = current_time.tms_utime - start_time.tms_utime;
			stime = current_time.tms_stime - start_time.tms_stime;
		} while ( ( (utime + stime) / HZ )  < cpu_time );
		last -= cpu_time;

		if (last <= 0 )
			break;

		/* IO Burst */
		sleep_time=0;
		while (sleep_time < io_time)
		{
			sleep(1);
			sleep_time++;
		}
		last -= sleep_time;
	}
}
