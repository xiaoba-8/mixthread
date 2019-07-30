/*
 * DefaultMutex.cpp
 *
 *  Created on: 2019-2-8
 *      Author: xiaoba-8
 */

#include <stdio.h>
#include <unistd.h>

#include <string.h>
#include <mix/mutex/thread_mutex.h>

namespace mix
{

#ifdef _WIN32

thread_mutex::thread_mutex()
{
	m_mutex = CreateMutex(NULL,FALSE,NULL);
}

thread_mutex::~thread_mutex()
{
	CloseHandle(m_mutex);
}

void thread_mutex::lock()
{
	WaitForSingleObject(m_mutex,INFINITE);
}

void thread_mutex::unlock()
{
	ReleaseMutex(m_mutex);
}

#else

thread_mutex::thread_mutex() : m_mutex(PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP)
{
}
thread_mutex::~thread_mutex()
{
	pthread_mutex_destroy(&m_mutex);
}

void thread_mutex::lock()
{
	pthread_mutex_lock(&m_mutex);
}

void thread_mutex::unlock()
{
	pthread_mutex_unlock(&m_mutex);
}

//void DefaultMutex::reset()
//{
//	pthread_mutex_t mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;
//	memcpy(&m_mutex, &mutex, sizeof(pthread_mutex_t));
//}
//
//void DefaultMutex::CleanUp(void *arg)
//{
//	DefaultMutex *pMutex = (DefaultMutex *)arg;
//	pMutex->unlock();
//}

#endif

}
