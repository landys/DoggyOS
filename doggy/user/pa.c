#include "unistd.h"

int Main()
{
	void* pAddress;
	char* str;
	int i;
	
	printf("%s", "\nTest Memory!\n");
	pAddress = malloc(256);
	str = (char*)pAddress;
	for(i = 0; i < 26; i++)
	{
		str[i] = 'a' + i;
	}
	str[26] = '\0';
	printf("the string is %s\n", str);
	
	printf("tickets = %d\n", get_ticks());
	free(pAddress);
	
	printf("\f");
	_exit(0);
}
