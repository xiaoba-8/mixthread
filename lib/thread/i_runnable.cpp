/*
 * i_runnable.cpp
 *
 *  Created on: 2019-5-20
 *      Author: xiaoba-8
 */

#include <mix/thread/i_runnable.h>
#include <mix/thread/mix_thread_container.h>

#include <string.h>

namespace mix
{

i_runnable::i_runnable()
{
	m_ct = time(NULL);
}

i_runnable::i_runnable(std::string name) : m_name(name)
{
	m_ct = time(NULL);
}
i_runnable::~i_runnable()
{

}
unsigned long i_runnable::GetCreateTime()
{
	return m_ct;
}

root_runnable::root_runnable() : m_priority(NORM_PRIORITY)
{
	mix_thread_container::SGetInstance()->Register(this);
}

root_runnable::root_runnable(std::string name) : i_runnable(name), m_priority(NORM_PRIORITY)
{
	mix_thread_container::SGetInstance()->Register(this);
}

root_runnable::~root_runnable()
{
	mix_thread_container::SGetInstance()->Unregister(this);
}

int root_runnable::Priority()
{
	return m_priority;
}

void root_runnable::SetPriority(int priority)
{
	m_priority = priority;
}

void root_runnable::TryPause()
{
	mix_thread_container::SGetInstance()->TryPause();
}

void root_runnable::Pause()
{
	m_pause = true;
}

void root_runnable::Resume()
{
	m_pause = false;
}

bool root_runnable::IsPaused()
{
	return m_pause;
}

root_thread::root_thread()
: m_priority(NORM_PRIORITY)
, m_pause(false)
, m_suspend(false)
,m_threadId(0)
{
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	memcpy(&m_mutex, &mutex, sizeof(pthread_mutex_t));
	memcpy(&m_cond, &cond, sizeof(pthread_cond_t));

	mix_thread_container::SGetInstance()->Register(this);
}
root_thread::root_thread(std::string name)
: i_thread(name)
, m_priority(NORM_PRIORITY)
, m_pause(false)
, m_suspend(false)
{
	pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
	pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

	memcpy(&m_mutex, &mutex, sizeof(pthread_mutex_t));
	memcpy(&m_cond, &cond, sizeof(pthread_cond_t));

	mix_thread_container::SGetInstance()->Register(this);
}
root_thread::~root_thread()
{
	mix_thread_container::SGetInstance()->Unregister(this);
}

unsigned long int root_thread::GetId()
{
	return m_threadId;
}

int root_thread::Priority()
{
	return m_priority;
}

void root_thread::SetPriority(int priority)
{
	m_priority = priority;
}

void root_thread::Pause()
{
	pthread_mutex_lock(&m_mutex);
	m_pause = true;
	pthread_mutex_unlock(&m_mutex);
}

void root_thread::Resume()
{
	pthread_mutex_lock(&m_mutex);
	m_pause = false;
	pthread_cond_signal(&m_cond);
	pthread_mutex_unlock(&m_mutex);
}

void root_thread::Suspend()
{
	m_suspend = true;
}

void root_thread::Continue()
{
	m_suspend = false;
}

bool root_thread::IsPaused()
{
	return m_pause;
}

bool root_thread::IsSuspend()
{
	return m_suspend;
}

void root_thread::TryPause()
{
	if (!m_pause)
		return;

	pthread_mutex_lock(&m_mutex);
	pthread_cond_wait(&m_cond, &m_mutex);
	pthread_mutex_unlock(&m_mutex);
}

bool less_runnable(i_runnable *m1, i_runnable *m2)
{
	if (m1->Priority() == m2->Priority())
		return m1->GetCreateTime() < m2->GetCreateTime();

	return m1->Priority() < m2->Priority();
}
}
