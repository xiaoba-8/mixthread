/*
 * thread_proxy.cpp
 *
 *  Created on: 2019-5-20
 *      Author: xiaoba-8
 */

#include <mix/thread/thread_proxy.h>

#include <stdio.h>

namespace mix
{

thread_proxy::thread_proxy(i_runnable *pRunnable, bool releaseOnExit, bool bDeleteOnFinish)
: mix_thread(true), m_pRunnable(pRunnable), m_releaseOnExit(releaseOnExit)
{
	m_bDeleteOnFinish = bDeleteOnFinish;
}

thread_proxy::~thread_proxy()
{
	if (m_releaseOnExit && m_pRunnable != NULL)
		delete m_pRunnable;
}

void thread_proxy::Run()
{
	if (m_pRunnable != NULL)
	{
		m_pRunnable->Run();
		if (m_pRunnable->IsDeleteOnFinish())
		{
			delete m_pRunnable;
			m_pRunnable = NULL;
		}
	}
	else
	{
		printf("run over\n");
	}
}

void thread_proxy::Cancel()
{
	if (m_pRunnable != NULL)
		m_pRunnable->Cancel();
}

bool thread_proxy::IsCanceled()
{
	if (m_pRunnable != NULL)
		return m_pRunnable->IsCanceled();
	else
		return false;
}

std::string thread_proxy::GetTaskId()
{
	if (m_pRunnable != NULL)
		return m_pRunnable->GetTaskId();
	else
		return "";
}

std::string thread_proxy::GetRunableName()
{
	if (m_pRunnable != NULL)
		return m_pRunnable->GetRunableName();
	else
		return mix_thread::GetRunableName();
}

}
