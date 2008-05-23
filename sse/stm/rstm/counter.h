#ifndef COUNTER_H
#define COUNTER_H

#include <stm_api.h>

class Counter : public stm::Object<Counter> 
{
    GENERATE_FIELD(int, value);

  public:

    virtual Counter* clone() const
    {
        int value = m_value;
	return new Counter(value);
    }

    Counter(int startingValue = 0) : m_value(startingValue) { }

    // increment the counter
    int increment(const stm::internal::Validator& v)
    {
        set_value(get_value(v) + 1);
	return get_value(v);
    }
};

#endif
