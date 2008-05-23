#ifndef BUFFER_H
#define BUFFER_H

#include <stm_api.h>

#define CAPACITY 2

class Buffer : public stm::Object<Buffer> 
{
    GENERATE_FIELD(int, start);
    GENERATE_FIELD(int, end);
    GENERATE_ARRAY(int, contents, CAPACITY);


  public:

    virtual Buffer* clone() const
    {
        Buffer *copy = new Buffer();
        copy->set_start(m_start);
	copy->set_end(m_end);

	int i;
	for (i = 0; i < CAPACITY; i++)
	{
	    copy->set_contents(i, m_contents[i]);
	}

	return copy;
    }

    Buffer() : m_start(0), m_end(0) { }

    int get(const stm::internal::Validator& v)
    {
        int value = get_contents(get_start(v) % CAPACITY, v);
	set_start(get_start(v) + 1);
	return value;
    }

    void put(int value, const stm::internal::Validator& v)
    {
	set_contents(get_end(v) % CAPACITY, value);
	set_end(get_end(v) + 1);
    }

    int length(const stm::internal::Validator& v) const
    {
        return get_end(v) - get_start(v);
    }
};

#endif
