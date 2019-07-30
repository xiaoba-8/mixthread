/*
 * thread_mutex.h
 *
 *  Created on: 2019-2-8
 *      Author: xiaoba-8
 */

#ifndef DEFAULTMUTEX_H_
#define DEFAULTMUTEX_H_


#ifdef _WIN32
	#include <winsock2.h>
	#include <windows.h>
#else // using pthread
	#include <pthread.h>
#endif // WIN32

namespace mix
{

class thread_mutex
{
public:
	thread_mutex();
	virtual ~thread_mutex();
	void lock();
	void unlock();

private:
#ifdef _WIN32
	HANDLE m_mutex;
#else
	pthread_mutex_t m_mutex;
#endif
};

}

#endif /* DEFAULTMUTEX_H_ */
