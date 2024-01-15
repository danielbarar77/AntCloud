#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		printf("Not enough arguments\n");
		return 0;
	}

	printf("First argument: %s\n", argv[1]);
	printf("Second argument: %s\n", argv[2]);
	printf("Third argument: %s\n", argv[3]);

	return 0;
}
