/*
 * mix_thread_container.cpp
 *
 *  Created on: 2018-11-7
 *      Author: xiaoba-8
 */

#include <mix/thread/thread_proxy.h>
#include <mix/thread/mix_thread_container.h>

namespace mix
{
mix_thread_container *mix_thread_container::s_pInstance = NULL;
mix_thread_container::mix_thread_container()
{

}

mix_thread_container::~mix_thread_container()
{

}
mix_thread_container *mix_thread_container::SGetInstance()
{
	if (s_pInstance == NULL)
	{
		s_pInstance = new mix_thread_container();
	}
	return s_pInstance;
}

void mix_thread_container::Register(i_runnable *pThread)
{
	m_mutex.lock();
	m_defaultThreadContainer.insert(pThread);
	m_mutex.unlock();
}
void mix_thread_container::Unregister(i_runnable *pThread)
{
	m_mutex.lock();
	m_defaultThreadContainer.erase(pThread);
	m_mutex.unlock();
}

i_runnable *mix_thread_container::GetThreadById(unsigned long id)
{
	i_runnable *pRunnable = NULL;

	std::set<i_runnable *>::iterator itr = m_defaultThreadContainer.begin();

	for (; itr != m_defaultThreadContainer.end(); ++itr)
	{
		if (dynamic_cast<root_thread *>(*itr))
		{
			if ((dynamic_cast<root_thread *>(*itr))->GetId() == id)
			{
				pRunnable = *itr;
				break;
			}
		}
	}

	return pRunnable;
}

i_runnable *mix_thread_container::GetCurrentThread()
{
	return GetThreadById(thread_proxy::GetCurrentThreadId());
}

bool mix_thread_container::IsThreadCanceled(unsigned long id)
{
	bool bCanceled;

	m_mutex.lock();
	i_runnable *pCur = GetThreadById(id);
	if (pCur != NULL)
		bCanceled = pCur->IsCanceled();
	else
		bCanceled = false;
	m_mutex.unlock();

	return bCanceled;
}

bool mix_thread_container::IsCurrentThreadCanceled()
{
	bool bCanceled;

	m_mutex.lock();
	i_runnable *pCur = GetCurrentThread();
	if (pCur != NULL)
		bCanceled = pCur->IsCanceled();
	else
		bCanceled = false;
	m_mutex.unlock();

	return bCanceled;
}

bool mix_thread_container::IsCurrentThreadPaused()
{
	bool bPaused;

	m_mutex.lock();
	i_runnable *pCur = GetCurrentThread();
	if (pCur != NULL && dynamic_cast<i_thread *>(pCur))
		bPaused = dynamic_cast<i_thread *>(pCur)->IsPaused();
	else
		bPaused = false;
	m_mutex.unlock();

	return bPaused;
}

void mix_thread_container::TryPause()
{
	i_runnable *pCur;

	m_mutex.lock();
	pCur = GetCurrentThread();
	m_mutex.unlock();

	if (pCur != NULL && dynamic_cast<root_thread *>(pCur))
		dynamic_cast<root_thread *>(pCur)->TryPause();
}

int mix_thread_container::CancelTask(std::string taskId, std::set<thread_pool *> &pools)
{
	int nRet = -1;
	if (taskId.empty())
		return nRet;

	std::vector<i_runnable *> removed;

	m_mutex.lock();
	std::set<i_runnable *>::iterator itr = m_defaultThreadContainer.begin();


	for (; itr != m_defaultThreadContainer.end(); ++itr)
	{
		if ((*itr)->GetTaskId() == taskId)
		{
			nRet = 0;

			std::set<thread_pool *>::iterator itrpl = pools.begin();
			for (; itrpl !=  pools.end(); ++itrpl)
			{
				(*itrpl)->Remove(*itr);
			}
			removed.push_back(*itr);
			//(*itr)->Cancel();
		}
	}
	m_mutex.unlock();

	/****/
	std::vector<i_runnable *>::iterator vitr;
	for (vitr = removed.begin(); vitr != removed.end(); ++vitr)
	{
		std::set<thread_pool *>::iterator itrpl = pools.begin();
		for (; itrpl !=  pools.end(); ++itrpl)
		{
			(*itrpl)->CancelThread(*vitr);
		}
		//(*itr)->Cancel();
	}
	/*****/
	return nRet;
}

int mix_thread_container::PauseTask(std::string taskId, std::set<thread_pool *> &pools)
{
	int nRet = -1;
	if (taskId.empty())
		return nRet;

	m_mutex.lock();
	std::set<i_runnable *>::iterator itr = m_defaultThreadContainer.begin();

	for (; itr != m_defaultThreadContainer.end(); ++itr)
	{
		if ((*itr)->GetTaskId() == taskId)
		{
			nRet = 0;
			std::set<thread_pool *>::iterator itrpl = pools.begin();
			for (; itrpl !=  pools.end(); ++itrpl)
			{
				if ((*itrpl)->PauseTask(*itr) == 0)
					break;
			}

			(*itr)->Pause();
		}
	}
	m_mutex.unlock();

	return nRet;
}

int mix_thread_container::ResumeTask(std::string taskId, std::set<thread_pool *> &pools)
{
	int nRet = -1;
	if (taskId.empty())
		return nRet;

	m_mutex.lock();
	std::set<i_runnable *>::iterator itr = m_defaultThreadContainer.begin();

	for (; itr != m_defaultThreadContainer.end(); ++itr)
	{
		if ((*itr)->GetTaskId() == taskId)
		{
			nRet = 0;
			std::set<thread_pool *>::iterator itrpl = pools.begin();
			for (; itrpl !=  pools.end(); ++itrpl)
			{
				if ((*itrpl)->ResumeTask(*itr) == 0)
					break;
			}

			(*itr)->Resume();
		}
	}
	m_mutex.unlock();

	return nRet;
}

int mix_thread_container::SetPriority(std::string taskId, int priority, std::set<thread_pool *> &pools)
{
	int nRet = -1;
	if (taskId.empty())
		return nRet;

	if (priority > MAX_PRIORITY)
		priority = MAX_PRIORITY;
	else if (priority < MIN_PRIORITY)
		priority = MIN_PRIORITY;

	m_mutex.lock();
	std::set<i_runnable *>::iterator itr = m_defaultThreadContainer.begin();

	for (; itr != m_defaultThreadContainer.end(); ++itr)
	{
		if ((*itr)->GetTaskId() == taskId)
		{
			(*itr)->SetPriority(priority);
			nRet = 0;
		}
	}
	m_mutex.unlock();

	if (nRet == 0)
	{
		std::set<thread_pool *>::iterator itrpl = pools.begin();
		for (; itrpl !=  pools.end(); ++itrpl)
		{
			(*itrpl)->SortQueue();
		}
	}
	return nRet;
}
}
