#include "route.h"
#include "networking.h"

route_info_t route_queue[MAX_ROUTE_NR] = {0};
int route_index = -1;

int push_route(route_info_t route)
{
	if (route_index == MAX_ROUTE_NR)
	{
		printf("Todo queue is full!\n");
		return -1;
	}

	for (int i = 1; i <= route_index; i++)
	{
		route_queue[i + 1] = route_queue[i];
	}
	route_queue[0] = route;
	route_index++;

	return 0;
}

int pop_route(int cd, route_info_t *route_out)
{

	if (route_index == -1)
	{
		printf("Todo queue is empty!\n");
		return -1;
	}

	// find the first route of the connection in the queue
	int i = 0;
	for (i = route_index; i > 0; i--)
	{
		if (isSender(cd, *(hosts[cd].pConnection)))
		{
			*route_out = route_queue[i];
		}
	}

	// remove from queue
	for (int j = i; j < route_index - 1; j++)
	{
		route_queue
			TODO : implement this
	}

	return 0;
}