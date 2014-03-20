#include "unistd.h"

int Main()
{
	int i = 0;
	while(i < 100){
		printf("C");
		milli_delay(500);
		i++;
	}
	printf("\f");
	_exit(0);
	
}

