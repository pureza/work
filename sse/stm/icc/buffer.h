#ifndef BUFFER_H
#define BUFFER_H

#define CAPACITY 10

class Buffer
{
    int start;
    int end;
    int contents[CAPACITY];

  public:

    Buffer() : start(0), end(0) { }

     __declspec(tm_callable)
    int get()
    {
        int value = contents[start % CAPACITY];
        start++;
        return value;
    }

     __declspec(tm_callable)
    void put(int value)
    {
        contents[end % CAPACITY] = value;
        end++;
    }

    __declspec(tm_callable)
    int length() const
    {
        return end - start;
    }
};

#endif
