#include<stdio.h>
#include<stdlib.h>
#include<assert.h>
#include<sys/time.h>

#define	MY_PERIOD	2	// seconds

int main()
{
	float time_use=0;
	struct timeval start;
	struct timeval end;
	struct timeval res;
	struct timezone tz;
	int	i;
	while(1) {
		// get current time and save it to start
		gettimeofday(&start, NULL);
		printf("start time: %d sec, %d usec\n", start.tv_sec, start.tv_usec);

		// do something
		for(i=1;i<1000000;i++);

		// get current time and save it to end
		gettimeofday(&end, NULL);
		printf("end  time: %d sec, %d usec\n", end.tv_sec, end.tv_usec);

		// calculate the remaining time interval to sleep
		timersub(&end, &start, &res);
		//i = MY_PERIOD * 1000000  - (res.tv_sec * 1000000 + res.tv_usec);
		i = MY_PERIOD * 1000000;

		// go to sleep until the remaining time pass
		usleep(i);
	}
}

