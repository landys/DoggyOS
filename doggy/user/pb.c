#include "unistd.h"

int Main()
{
	printf("\nTest get_ticket.\n");
	printf("Ticket=%d.\n", get_ticks());

	_exit(0);
}

