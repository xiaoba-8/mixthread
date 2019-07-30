/*
 * mix_thread.cpp
 *
 *  Created on: 2019-4-13
 *      Author: xiaoba-8
 */

#include <stdio.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

#include <iostream>

#include <mix/thread/mix_thread.h>
#include <mix/utils/thread_exception.h>
#include <mix/thread_errno.h>

namespace mix
{

mix_thread::mix_thread(bool detach)
:root_thread("mix_thread")
,m_running(false)
,m_bDeleteOnFinish(false)
,m_bDestructor(false)
,m_joined(false)
,m_detach(detach)
{
#ifdef _WIN32

#else

	pthread_attr_init(&m_attr);

	if (detach)
		pthread_attr_setdetachstate(&m_attr,PTHREAD_CREATE_DETACHED);
	else
		pthread_attr_setdetachstate(&m_attr,PTHREAD_CREATE_JOINABLE);

#endif
}


mix_thread::~mix_thread()
{
	m_bDestructor = true;
	if (m_running)
	{
		SetRunning(false);
	}
#ifdef _WIN32
	if (m_threadId)
		::CloseHandle(m_threadId);
#else
	pthread_attr_destroy(&m_attr);
	Join();
#endif
}


threadfunc_t STDPREFIX mix_thread::StartThread(threadparam_t zz)
{
	mix_thread *p = (mix_thread *)zz;

#ifndef _WIN32
	int oldstate;
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, &oldstate);
	int oldtype;
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, &oldtype);
#endif


	if (p -> m_running)
	{
		p -> Run();
	}
	else
	{
		printf("thread over\n");
	}
	p -> SetRunning(false); // if return
	if (p -> IsDeleteOnFinish() && !p -> IsDestructor())
	{
		//std::cout << "Destroy Thread Class :" << p->GetRunableName() << std::endl;
		delete p;
	}
#ifdef _WIN32
	_endthreadex(0);
#else
	pthread_exit(0);
#endif
	return (threadfunc_t)NULL;
}

bool mix_thread::Start()
{
	m_joined = false;
	m_running = true;
#ifdef _WIN32
	m_threadId = (HANDLE)_beginthreadex(NULL, 0, &StartThread, this, 0, &m_dwThreadId);
#else
	if (pthread_create(&m_threadId, &m_attr, StartThread, this) != 0)
	{
		if (errno == EAGAIN)
		{

		}
		printf("----------%d [%s]\n", errno, strerror(errno));
		perror("Thread: create failed");
		SetRunning(false);

		return false;
	}
	else
	{
		return true;
	}

#endif
}

void mix_thread::Stop()
{
	SetRunning(false);

#ifdef _WIN32
	TerminateThread(m_threadId, 0);
#else

	if (pthread_cancel(m_threadId) == -1)
	{
		perror("Thread: cancel failed");
	}

#endif
}

unsigned long int mix_thread::GetId()
{
	return m_threadId;
}

bool mix_thread::IsRunning()
{
 	return m_running;
}


void mix_thread::SetRunning(bool x)
{
 	m_running = x;
}


bool mix_thread::IsDeleteOnFinish()
{
	return m_bDeleteOnFinish;
}


void mix_thread::SetDeleteOnFinish(bool x)
{
	m_bDeleteOnFinish = x;
}


bool mix_thread::IsDestructor()
{
	return m_bDestructor;
}


void mix_thread::Pause()
{
	root_thread::Pause();
}

void mix_thread::Resume()
{
	root_thread::Resume();
}

unsigned long int mix_thread::GetCurrentThreadId()
{
#ifdef _WIN32
	return ::GetCurrentThreadId();
#else
	return pthread_self();
#endif
}

void mix_thread::Join()
{
#ifdef _WIN32

#else
	if (m_threadId > 0 && !m_joined && !m_detach)
	{
		m_joined = true;
		pthread_join(m_threadId, NULL);
	}
#endif
}

void mix_thread::Join(unsigned long millisTime)
{
#ifdef _WIN32

#else
	if (m_threadId > 0 && !m_detach)
	{
		if (millisTime == 0)
			pthread_join(m_threadId, NULL);
		else
		{
			unsigned long k = 0;
			while (IsRunning() && k < millisTime)
			{
				usleep(10000);
				k += 10;
			}

			if (!IsRunning())
			{
				pthread_join(m_threadId, NULL);
			}
		}
	}
#endif
}

#ifndef _WIN32
void mix_thread::InitCancelHandler()
{
	struct sigaction act, oact;
	act.sa_handler = &mix_thread::CancelHandler;
	sigemptyset(&act.sa_mask);
	act.sa_flags = 0;
	act.sa_restorer = NULL;

	sigaction(SIGUSR1, &act, &oact);
}

void mix_thread::CancelHandler(int sig)
{
	pthread_exit(PTHREAD_CANCELED);
}


void mix_thread::SetPriority(int priority)
{
	struct sched_param param;

	int policy = SCHED_RR;
	param.__sched_priority = priority;

	if (priority <= 0)
	{
		policy = SCHED_OTHER;
		param.__sched_priority = 0;
	}

	if (pthread_setschedparam(m_threadId, policy, &param) != 0)
	{
		throw thread_exception(THREAD_NUMBER + THREAD_PTHREAD_SET_SCHED_NO, strerror(errno));
	}
}

int mix_thread::GetPriority()
{
	struct sched_param param;
	int policy;

	if (pthread_getschedparam(m_threadId, &policy, &param) == 0)
	{
		return param.__sched_priority;
	}
	else
	{
		throw thread_exception(THREAD_NUMBER + THREAD_PTHREAD_GET_SCHED_NO, strerror(errno));
	}
}

#endif
}
