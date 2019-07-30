/*
 * mix_thread_container.h
 *
 *  Created on: 2019-11-7
 *      Author: xiaoba-8
 */

#ifndef DEFAULTTHREADCONTAINER_H_
#define DEFAULTTHREADCONTAINER_H_

#include <set>
#include "i_runnable.h"
#include "thread_pool.h"
#include "../mutex/thread_mutex.h"

namespace mix
{
class mix_thread_container
{
private:
	thread_mutex m_mutex;
	std::set<i_runnable *> m_defaultThreadContainer;
	static mix_thread_container *s_pInstance;
	mix_thread_container();

public:
	virtual ~mix_thread_container();
	static mix_thread_container *SGetInstance();

	void Register(i_runnable *pThread);
	void Unregister(i_runnable *pThread);

	bool IsCurrentThreadCanceled();
	bool IsThreadCanceled(unsigned long id);

	bool IsCurrentThreadPaused();
	void TryPause();

	int CancelTask(std::string taskId, std::set<thread_pool *> &);
	int SetPriority(std::string taskId, int priority, std::set<thread_pool *> &pools);

	int PauseTask(std::string taskId, std::set<thread_pool *> &);
	int ResumeTask(std::string taskId, std::set<thread_pool *> &);
private:
	i_runnable *GetThreadById(unsigned long id);
	i_runnable *GetCurrentThread();
};

#define IsCurrentDefaultThreadCanceled() mix_thread_container::SGetInstance()->IsCurrentThreadCanceled()
}

#endif /* DEFAULTTHREADCONTAINER_H_ */
