#include <stdlib.h>
#include <stdio.h>

unsigned long factorial(unsigned int n)
{
	if (n == 1)
	{
		return 1;
	}

	return n * factorial(n - 1);
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Please provide a number");
		return 0;
	}
	unsigned long value = factorial(atoi(argv[1]));
	printf("Result:%lu\n", value);
	return 0;
}