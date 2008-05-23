#include <stdlib.h>
#include <pthread.h>
#include <stm_api.h>
#include "counter.h"
#include "buffer.h"

#define PRODUCERS 10
#define CONSUMERS 10

#define ITERATIONS 100

stm::sh_ptr<Counter> counter;
stm::sh_ptr<Buffer> buffer;

void *producer_main(void *args)
{
    stm::init("Polka", "", false);
    
    int i;
    for (i = 0; i < ITERATIONS; i++)
    { 
        int value = -1;
	bool space_available = false;
    
        BEGIN_TRANSACTION;

        do {
	    stm::rd_ptr<Buffer> rd_buffer(buffer);

	    if (rd_buffer->length(rd_buffer.v()) < CAPACITY)
	        space_available = true;

	    if (space_available)
	    {
                stm::wr_ptr<Buffer> wr_buffer(rd_buffer);
                stm::wr_ptr<Counter> wr_counter(counter);
    
    	        value = wr_counter->increment(wr_counter.v());

		// Give other threads the chance to see inconsistence state if there are any bugs.
		usleep(100);
	        value = wr_counter->increment(wr_counter.v());

                wr_buffer->put(value, wr_buffer.v());
	    } else {
	        pthread_yield();
	    }
	} while (!space_available);

        END_TRANSACTION;
    }

    stm::shutdown();

    return NULL;
}

void *consumer_main(void *args)
{
      stm::init("Polka", "", false);
      
      int i;
      for (i = 0; i < ITERATIONS; i++)
      {
	  bool elements_available = false;
          int value = -1;

          BEGIN_TRANSACTION;

          do {
	      stm::rd_ptr<Buffer> rd_buffer(buffer);

	      if (rd_buffer->length(rd_buffer.v()) > 0)
	          elements_available = true;

	      if (elements_available)
	      {
                  stm::wr_ptr<Buffer> wr_buffer(rd_buffer);
                  value = wr_buffer->get(wr_buffer.v());
	      } else {
	          pthread_yield();
	      }
	  } while (!elements_available);

	  END_TRANSACTION;

	  printf(" get %d\n", value);
      }


      stm::shutdown();

      return NULL;
}

int main()
{
    stm::init("Polka", "", false);

    counter = stm::sh_ptr<Counter>(new Counter(0));
    buffer = stm::sh_ptr<Buffer>(new Buffer());

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
