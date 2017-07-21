#ifndef AUTO_MUTEX_H_
#define AUTO_MUTEX_H_

#define __STDC_LIMIT_MACROS

#include <stdint.h>
#include <pthread.h>

class auto_mutex
{
public:
	auto_mutex(pthread_mutex_t* mutex);
	~auto_mutex();

private:
	pthread_mutex_t* m_mutex;
};

#endif //AUTO_MUTEX_H_