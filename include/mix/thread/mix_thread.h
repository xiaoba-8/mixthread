/*
 * mix_thread.h
 *
 *  Created on: 2019-4-13
 *      Author: xiaoba-8
 */

#ifndef THREAD_H_
#define THREAD_H_

#ifdef _WIN32
#else
#include <pthread.h>
#endif
#include "i_runnable.h"

#ifdef _WIN32

typedef unsigned threadfunc_t;
typedef void * threadparam_t;
#define STDPREFIX __stdcall
#else

typedef void * threadfunc_t;
typedef void * threadparam_t;
#define STDPREFIX
#endif


namespace mix
{

class mix_thread : public root_thread
{
public:
	mix_thread(bool detach = false);
	virtual ~mix_thread();

	static threadfunc_t STDPREFIX StartThread(threadparam_t);

	virtual void Run() = 0;

#ifdef _WIN32
	HANDLE GetThread() { return m_thread; }
	unsigned int GetThreadId() { return m_dwThreadId; }
#else
	pthread_t GetThread() { return m_threadId; }
	unsigned long int GetThreadId() { return m_threadId; }
#endif

	bool IsRunning();
	void SetRunning(bool x);
	bool IsDeleteOnFinish();
	void SetDeleteOnFinish(bool x = true);
	bool IsDestructor();

	virtual bool Start();

	virtual void Stop();

	virtual unsigned long int GetId();

	static unsigned long int GetCurrentThreadId();

	virtual void Join();
	virtual void Join(unsigned long millisTime);

	virtual void SetPriority(int priority);
	virtual int GetPriority();

	virtual void Pause();
	virtual void Resume();

protected:

#ifdef _WIN32
	HANDLE m_thread;
	unsigned m_dwThreadId;
#else
	static void CancelHandler(int sig);
	void InitCancelHandler();

	pthread_attr_t m_attr;
#endif

	mix_thread(const mix_thread& ) : root_thread("mix_thread") {}
	mix_thread& operator=(const mix_thread& ) { return *this; }
	bool m_running;
	bool m_bDeleteOnFinish;
	bool m_bDestructor;
	bool m_joined;
	bool m_detach;
};

}

#endif /* THREAD_H_ */
