#include "stdio.h"
#include "auto_mutex.h"

auto_mutex::auto_mutex(pthread_mutex_t* mutex)
{
//	printf("Enter pid: %u, lock: 0x%x\n", pthread_self(), mutex);
	m_mutex = mutex;
	pthread_mutex_lock(m_mutex);
}

auto_mutex::~auto_mutex()
{
//	printf("Leave pid: %u, lock: 0x%x\n", pthread_self(), m_mutex);
	pthread_mutex_unlock(m_mutex);
}
