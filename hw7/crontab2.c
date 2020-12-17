#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

void main(){
	time_t t;
	struct tm *tm;

	t=time(NULL);
	tm=localtime(&t);
	printf("손준호 starting job by crontab2 at %s\n", asctime(tm));
}
