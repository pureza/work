#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include "counter.h"
#include "buffer.h"

#define PRODUCERS 10
#define CONSUMERS 10

Counter counter;
Buffer buffer;

void *producer_main(void *args)
{
    int i;
    while (true)
    {
        int value = -1;
	int length;

        __tm_atomic 
	{
	    length = buffer.length();
   	    if (length < CAPACITY)
	    {
	        counter.increment();
	        value = counter.increment();
	        buffer.put(value);
	    } else { 
	        continue;
	    }
	}

	printf(" put %4d [%d]\n", value, length);

    }
    return NULL;
}

void *consumer_main(void *args)
{
    int i;
    while (true)
    {
	int value = -1;
        int length;

	__tm_atomic 
	{
	    length = buffer.length();
	    if (length > 0)
	    {
	        value = buffer.get();
	    } else {
	        continue;
	    }
	}

	printf(" get %4d [%d]\n", value, length);
    }

    return NULL;
}

int main()
{
    pthread_t threads[PRODUCERS + CONSUMERS];

    int i;

    // Create producers
    for (i = 0; i < PRODUCERS; i++)
    {
        pthread_create(&threads[i], NULL, &producer_main, NULL);
    }

    // Create consumers
    for (i = 0; i < CONSUMERS; i++)
    {
        pthread_create(&threads[i + PRODUCERS], NULL, &consumer_main, NULL);
    }

    // Join all threads
    for (i = 0; i < PRODUCERS + CONSUMERS; i++)
    {
	pthread_join(threads[i], NULL);
    }
}

