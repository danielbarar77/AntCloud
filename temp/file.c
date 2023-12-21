#include <stdio.h>

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		printf("Not enough arguments");
		return 0;
	}
	int a = atoi(argv[1]);
	int b = atoi(argv[2]);

	printf("Rezultatul este: %d", a + b);

	return 0;
}