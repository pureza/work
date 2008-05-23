#ifndef COUNTER_H
#define COUNTER_H

class Counter 
{
  int value;

  public:

    Counter() : value(0) { }

    // increment the counter
     __declspec(tm_callable)
    int increment()
    {
        return ++value;
    }
};

#endif

