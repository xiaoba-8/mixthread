/*
 * thread_pool.cpp
 *
 *  Created on: 2019-3-31
 *      Author: xiaoba-8
 */

#include <signal.h>

#include <algorithm>
#include <mix/utils/thread_exception.h>
#include <mix/thread/thread_pool.h>
#include <mix/thread_errno.h>

#include <fstream>

#define max(x, y) ((x)>(y)?(x):(y))

namespace mix
{

#define SIGSUSPEND SIGUSR1
#define SIGRESUME SIGUSR2


void suspend_handler(int signum)
{
	sigset_t nset;
	pthread_sigmask(0, NULL, &nset);

	sigdelset(&nset, SIGSUSPEND);
	sigsuspend(&nset);
}

void resume_handler(int signum)
{

}

void InitialThreadStatus()
{
	struct sigaction suspendsa;
	struct sigaction resumesa;

	sigemptyset(&suspendsa.sa_mask);
	sigaddset(&suspendsa.sa_mask, SIGSUSPEND);
	suspendsa.sa_flags = SA_NODEFER;
	suspendsa.sa_handler = suspend_handler;
	sigaction(SIGSUSPEND, &suspendsa, NULL);

	sigemptyset(&resumesa.sa_mask);
	sigaddset(&resumesa.sa_mask, SIGRESUME);
	resumesa.sa_flags = SA_NODEFER;
	resumesa.sa_handler = resume_handler;
	sigaction(SIGRESUME, &resumesa, NULL);
}

thread_pool::Worker::Worker(i_runnable *pFirstTask, thread_pool *pPool) : root_monitor("thread_pool::Worker"),
	m_active(false), m_canceled(false), m_nStatus(10), m_pPool(pPool), m_pThread(NULL)
{
	m_pFirstTask = pFirstTask;
	m_pCurrentTask = m_pFirstTask;
}

thread_pool::Worker::~Worker()
{

}

bool thread_pool::Worker::IsActive()
{
	return m_active;
}

bool thread_pool::Worker::IsTask(i_runnable *pTask)
{
	return m_pFirstTask == pTask || m_pCurrentTask == pTask;
}

/**
 * Runs a single task between before/after methods.
 */
void thread_pool::Worker::RunTask(i_runnable *pTask)
{
	if (pTask->IsCanceled())
		return;

	m_active = true;
	m_nStatus = 2;

	bool ran = false;
	m_pPool->BeforeExecute(m_pThread, pTask);

	m_nStatus = 3;
	pTask->Run();
	m_nStatus = 4;

	ran = true;
	m_pPool->AfterExecute(pTask);
	++m_completedTasks;

	m_active = false;
	m_nStatus = 5;
}

void thread_pool::Worker::InterruptIfIdle()
{
	m_canceled = true;
}

void thread_pool::Worker::InterruptNow()
{
	m_pPool->m_mainLock.lock();
	if (m_pCurrentTask != NULL)
		m_pCurrentTask->Cancel();
	m_canceled = true;
	m_pPool->m_mainLock.unlock();
}

/*
 * monitor
 */

i_info *thread_pool::Worker::Info()
{
	i_info *pInfo = NULL;
	m_pPool->m_mainLock.lock();
	//printf("---------------currenttask info %d\n", (int)m_pCurrentTask);
	if (m_pCurrentTask != NULL && dynamic_cast<i_monitor *>(m_pCurrentTask))
	{
		pInfo = dynamic_cast<i_monitor *>(m_pCurrentTask)->Info();
		pInfo->SetId(this->m_pThread->GetId());
		pInfo->SetPaused(this->m_pThread->IsPaused());
		pInfo->SetSuspend(this->m_pThread->IsSuspend());
		pInfo->SetStatus(m_nStatus);
	}
	else
	{
		pInfo = new thread_info("[NULL]", "-1", 0, false, m_nStatus, false);
	}
	m_pPool->m_mainLock.unlock();
	return pInfo;
}

bool thread_pool::Worker::GetNextTask()
{
	m_pPool->m_mainLock.lock();
	m_pCurrentTask = m_pPool->GetTask();
	m_pPool->m_mainLock.unlock();

	return m_pCurrentTask != NULL;
}

/**
 * Main run loop
 */
void thread_pool::Worker::Run()
{
	m_pCurrentTask = m_pFirstTask;
	m_pFirstTask = NULL;

	m_nStatus = 11;
	while (m_pCurrentTask != NULL || GetNextTask())
	{
		m_nStatus = 12;
		if (this->GetThread()->GetPriority() != m_pCurrentTask->Priority())
		{
			try
			{
				this->GetThread()->SetPriority(m_pCurrentTask->Priority());
			}
			catch (thread_exception &e)
			{

			}
		}

		RunTask(m_pCurrentTask);

		m_pPool->m_mainLock.lock();
		if (m_pCurrentTask->IsDeleteOnFinish())
		{
			i_runnable *pTmpTask = m_pCurrentTask;
			m_pCurrentTask = NULL;
			delete pTmpTask;
		}
		else
		{
			m_pCurrentTask = NULL;
		}
		m_pPool->m_mainLock.unlock();
	}
	m_pPool->WorkerDone(this);
}

void thread_pool::Worker::SetPriority(int priority)
{
	 if (m_pCurrentTask != NULL)
		 m_pCurrentTask->SetPriority(priority);
	 else if (m_pFirstTask != NULL)
		 m_pFirstTask->SetPriority(priority);
	 else
		 return;
}

int thread_pool::Worker::Priority()
{
	 if (m_pCurrentTask != NULL)
		 return m_pCurrentTask->Priority();
	 else if (m_pFirstTask != NULL)
		 return m_pFirstTask->Priority();
	 else
		 return 0;
}

void thread_pool::Worker::Cancel()
{
	InterruptNow();
}

bool thread_pool::Worker::IsCanceled()
{
	return m_canceled;
}

std::string thread_pool::Worker::GetTaskId()
{
	 if (m_pCurrentTask != NULL)
		 return m_pCurrentTask->GetTaskId();
	 else if (m_pFirstTask != NULL)
		 return m_pFirstTask->GetTaskId();
	 else
		 return "0";
}

thread_pool::thread_pool(unsigned int corePoolSize, unsigned int maximumPoolSize,
		unsigned long keepAliveTime)
:m_runState(RUNNING)
,m_allowCoreThreadTimeOut(false)
,m_poolSize(0)
,m_largestPoolSize(0)
,m_completedTaskCount(0)
{
	if (corePoolSize < 0 || maximumPoolSize <= 0 || maximumPoolSize
			< corePoolSize || keepAliveTime < 0)
		throw mix::thread_exception(THREAD_NUMBER + THREAD_THREADPOOL_ILLEGAL_NO, "Illegal Argument");

	m_corePoolSize = corePoolSize;
	m_maximumPoolSize = maximumPoolSize;
	m_keepAliveTime = keepAliveTime;
}

thread_pool::~thread_pool()
{
	ShutdownNow();

	m_mainLock.lock();

	m_workQueue.clear();
	m_pausedQueue.clear();

	m_mainLock.unlock();

	while (this->GetActiveCount() > 0)
	{
		sleep(1);
	}

	m_mainLock.lock();
	std::set<Worker *>::iterator iter = m_workers.begin();

	for (; iter != m_workers.end(); ++iter)
	{
		delete (*iter);
	}

	m_workers.clear();
	m_mainLock.unlock();
}

void thread_pool::Terminate()
{
	ShutdownNow();

	m_mainLock.lock();
	m_workQueue.clear();
	m_mainLock.unlock();

	while (this->GetActiveCount() > 0)
	{
		sleep(1);
	}

	m_mainLock.lock();
	std::set<Worker *>::iterator iter = m_workers.begin();

	for (; iter != m_workers.end(); ++iter)
	{
		delete (*iter);
	}

	m_workers.clear();
	m_mainLock.unlock();
}

void thread_pool::Execute(i_runnable *pCommand)
{
	if (pCommand == NULL)
		throw mix::thread_exception(THREAD_NUMBER + THREAD_THREADPOOL_NULL_NO, "Null Command");
	if (m_poolSize >= m_corePoolSize || !AddIfUnderCorePoolSize(pCommand))
	{
		if (m_runState == RUNNING)
		{
			AddToWorkQueueByPriority(pCommand);

			if (m_runState != RUNNING || m_poolSize == 0)
				EnsureQueuedTaskHandled(pCommand);
		}
		else
		{
			Reject(pCommand, "thread_pool is not running!");
		}
	}
}

int thread_pool::TerminateThread(unsigned long id)
{
	int nResult = -1;
	m_mainLock.lock();

	std::set<Worker *>::iterator itr = m_workers.begin();

	for (; itr != m_workers.end(); ++itr)
	{
		Worker *pWorker = *itr;
		if (pWorker->GetThread()->GetId() == id)
		{
			pWorker->GetThread()->Stop();

			m_workers.erase(pWorker);
			if (--m_poolSize == 0)
				TryTerminate();

			delete pWorker;
			nResult = 0;
			break;
		}
	}

	m_mainLock.unlock();

	return nResult;
}

int thread_pool::CancelThread(unsigned long id)
{
	int nResult = -1;
	m_mainLock.lock();

	std::set<Worker *>::iterator itr = m_workers.begin();

	for (; itr != m_workers.end(); ++itr)
	{
		Worker *pWorker = *itr;
		if (pWorker->GetThread()->GetId() == id)
		{
			pWorker->GetThread()->Cancel();
			nResult = 0;
			break;
		}
	}

	std::deque<i_runnable *>::iterator itr2 = m_workQueue.begin();

	for (; itr2 != m_workQueue.end(); ++itr2)
	{
		i_monitor *pMonitor = dynamic_cast<i_monitor *>(*itr2);

		unsigned long wid = (unsigned long)pMonitor;
		if (wid == id)
		{
			m_workQueue.erase(itr2);
			nResult = 0;
			break;
		}
	}

	m_mainLock.unlock();

	return nResult;
}

int thread_pool::CancelThread(i_runnable *pRunnable)
{
	int nResult = -1;
	m_mainLock.lock();

	std::set<Worker *>::iterator itr = m_workers.begin();

	for (; itr != m_workers.end(); ++itr)
	{
		Worker *pWorker = *itr;
		if (pWorker->GetThread() == pRunnable)
		{
			pWorker->GetThread()->Cancel();
			nResult = 0;
			break;
		}
	}

	std::deque<i_runnable *>::iterator itr2 = m_workQueue.begin();

	for (; itr2 != m_workQueue.end(); ++itr2)
	{
		i_monitor *pMonitor = dynamic_cast<i_monitor *>(*itr2);

		if (pRunnable == pMonitor)
		{
			m_workQueue.erase(itr2);
			nResult = 0;
			break;
		}
	}

	m_mainLock.unlock();

	return nResult;
}

int thread_pool::SetPriority(unsigned long id, int priority)
{
	int nRet = -1;

	if (priority > MAX_PRIORITY)
		return nRet;
	else if (priority < MIN_PRIORITY)
		return nRet;

	m_mainLock.lock();

	std::set<Worker *>::iterator itr = m_workers.begin();

	for (; itr != m_workers.end(); ++itr)
	{
		Worker *pWorker = *itr;
		if (pWorker->GetThread()->GetId() == id)
		{
			try
			{
				pWorker->GetThread()->SetPriority(priority);
				pWorker->SetPriority(priority);
			}
			catch (thread_exception &e)
			{
				m_mainLock.unlock();
				throw e;
			}
			nRet = 0;
			break;
		}
	}

	if (nRet != 0)
	{
		std::deque<i_runnable *>::iterator itr2 = m_workQueue.begin();

		for (; itr2 != m_workQueue.end(); ++itr2)
		{
			i_monitor *pMonitor = dynamic_cast<i_monitor *>(*itr2);

			unsigned long wid = (unsigned long)pMonitor;
			if (wid == id)
			{
				pMonitor->SetPriority(priority);
				std::sort(m_workQueue.begin(), m_workQueue.end(), less_runnable);
				nRet = 0;
				break;
			}
		}
	}

	m_mainLock.unlock();
	return nRet;
}

int thread_pool::PauseThread(unsigned long id)
{
	int nResult = -1;
	m_mainLock.lock();

	std::set<Worker *>::iterator itr = m_workers.begin();

	for (; itr != m_workers.end(); ++itr)
	{
		Worker *pWorker = *itr;
		if (pWorker->GetThread()->GetId() == id)
		{
			pWorker->GetThread()->Pause();
			nResult = 0;
			break;
		}
	}

	m_mainLock.unlock();

	return nResult;
}

int thread_pool::ResumeThread(unsigned long id)
{
	int nResult = -1;
	m_mainLock.lock();

	std::set<Worker *>::iterator itr = m_workers.begin();

	for (; itr != m_workers.end(); ++itr)
	{
		Worker *pWorker = *itr;
		if (pWorker->GetThread()->GetId() == id)
		{
			pWorker->GetThread()->Resume();
			nResult = 0;
			break;
		}
	}

	m_mainLock.unlock();

	return nResult;
}

int thread_pool::SuspendThread(unsigned long id)
{
	int nResult = -1;
	m_mainLock.lock();

	std::set<Worker *>::iterator itr = m_workers.begin();

	for (; itr != m_workers.end(); ++itr)
	{
		Worker *pWorker = *itr;
		if (pWorker->GetThread()->GetId() == id)
		{
			pWorker->GetThread()->Suspend();
			nResult = 0;
			break;
		}
	}

	m_mainLock.unlock();
	if (nResult == 0)
		pthread_kill(id, SIGSUSPEND);

	return nResult;
}

int thread_pool::ContinueThread(unsigned long id)
{
	int nResult = -1;
	m_mainLock.lock();

	std::set<Worker *>::iterator itr = m_workers.begin();

	for (; itr != m_workers.end(); ++itr)
	{
		Worker *pWorker = *itr;
		if (pWorker->GetThread()->GetId() == id)
		{
			pWorker->GetThread()->Continue();
			nResult = 0;
			break;
		}
	}

	m_mainLock.unlock();
	if (nResult == 0)
		pthread_kill(id, SIGRESUME);

	return nResult;
}

void thread_pool::AddToWorkQueueByPriority(i_runnable *pTask)
{
	m_mainLock.lock();

	if (m_workQueue.size() == 0)
	{
		m_workQueue.push_back(pTask);
		m_mainLock.unlock();
		return;
	}

	if (m_workQueue.size() >= m_maximumPoolSize - m_corePoolSize)
	{
		m_mainLock.unlock();
		Reject(pTask);
	}

	m_workQueue.push_back(pTask);
	std::sort(m_workQueue.begin(), m_workQueue.end(), less_runnable);

	m_mainLock.unlock();
}

i_thread *thread_pool::AddThread(i_runnable *pFirstTask)
{
	Worker *pWorker = new Worker(pFirstTask, this);
	i_thread *pThread = new thread_proxy(pWorker, true, true);
	if (pThread != NULL)
	{
		pWorker->SetThread(pThread);
		m_workers.insert(pWorker);
		++m_poolSize;

		if (pThread->Start())
		{
			int nt = m_poolSize;
			if (nt > m_largestPoolSize)
				m_largestPoolSize = nt;
		}
		else
		{
			--m_poolSize;
			m_workers.erase(pWorker);
			//delete pWorker;
			delete pThread;
			pThread = NULL;
		}
	}
	return pThread;
}

/**
 * Creates and starts a new thread running firstTask as its first
 * task, only if fewer than corePoolSize threads are running
 * and the pool is not shut down.
 * @param firstTask the task the new thread should run first (or
 * NULL if none)
 * @return true if successful
 */
bool thread_pool::AddIfUnderCorePoolSize(i_runnable *pFirstTask)
{
	i_thread *pThread = NULL;
	m_mainLock.lock();

	if (m_poolSize < m_corePoolSize && m_runState == RUNNING)
		pThread = AddThread(pFirstTask);

	m_mainLock.unlock();

	if (pThread == NULL)
		return false;

	return true;
}

bool thread_pool::Erase(std::deque<i_runnable *> &queue, i_runnable *pCommand)
{
	std::deque<i_runnable *>::iterator itr = queue.begin();

	while (itr != queue.end())
	{
		if (*itr == pCommand)
		{
			queue.erase(itr, itr + 1);
			return true;
		}
		++itr;
	}

	return false;
}

bool thread_pool::Erase(std::vector<i_runnable *> &queue, i_runnable *pCommand)
{
	std::vector<i_runnable *>::iterator itr = queue.begin();

	while (itr != queue.end())
	{
		if (*itr == pCommand)
		{
			queue.erase(itr, itr + 1);
			return true;
		}
		++itr;
	}

	return false;
}

void thread_pool::EnsureQueuedTaskHandled(i_runnable *pCommand)
{
	m_mainLock.lock();
	bool reject = false;
	i_thread *pThread = NULL;

	int state = m_runState;
	if (state != RUNNING) // && m_workQueue.remove(pCommand))
	{
		Erase(m_workQueue, pCommand);
		reject = true;
	}
	else if (state < STOP && m_poolSize < max(m_corePoolSize, 1)
			&& !m_workQueue.empty())
	{
		pThread = AddThread(NULL);
	}

	m_mainLock.unlock();

	if (reject)
		Reject(pCommand);
	else if (pThread != NULL)
	{
		//pThread->Start();
	}
}

/**
 * Invokes the rejected execution handler for the given command.
 */
void thread_pool::Reject(i_runnable *pCommand, const char *fmt)
{
	if (pCommand->IsDeleteOnFinish())
		delete pCommand;
	throw mix::thread_exception(THREAD_NUMBER + THREAD_THREADPOOL_REJECT_NO, THREAD_THREADPOOL_REJECT_MSG,
			m_poolSize, m_workers.size(), m_maximumPoolSize, fmt);//, cmdName.c_str());
}

i_runnable *thread_pool::GetTask()
{
	for (;;)
	{
		int state = m_runState;
		if (state > SHUTDOWN)
			return NULL;
		i_runnable *pRunnable = NULL;

		if (m_workQueue.size() > 0)
		{
			if (state == SHUTDOWN)
			{
				pRunnable = m_workQueue.front();
				m_workQueue.pop_front();
			}
			else if (m_poolSize > m_corePoolSize || m_allowCoreThreadTimeOut)
			{
				pRunnable = m_workQueue.front();
				m_workQueue.pop_front();
			}
			else
			{
				pRunnable = m_workQueue.front();
				m_workQueue.pop_front();
			}
		}
		else
		{
			usleep(10000);
		}

		if (pRunnable != NULL)
			return pRunnable;

		if (WorkerCanExit())
		{
			if (m_runState >= SHUTDOWN) // Wake up others
				InterruptIdleWorkers();
		}
		return NULL;
	}
}

/**
 * Check whether a worker thread that fails to get a task can
 * exit.  We allow a worker thread to die if the pool is stopping,
 * or the queue is empty, or there is at least one thread to
 * handle possibly non-empty queue, even if core timeouts are
 * allowed.
 */
bool thread_pool::WorkerCanExit()
{
	bool canExit;

	canExit
			= m_runState >= STOP || m_workQueue.empty()
					|| (m_allowCoreThreadTimeOut && m_poolSize
							> max(1, m_corePoolSize));

	return canExit;
}

/**
 * Wakes up all threads that might be waiting for tasks so they
 * can check for termination. Note: this method is also called by
 * ScheduledThreadPoolExecutor.
 */
void thread_pool::InterruptIdleWorkers()
{
	std::set<Worker *>::iterator iter = m_workers.begin();

	for (; iter != m_workers.end(); ++iter)
		(*iter)->InterruptIfIdle();
}

/**
 * Performs bookkeeping for an exiting worker thread.
 * @param w the worker
 */
void thread_pool::WorkerDone(Worker *pWorker)
{
	m_mainLock.lock();

	m_completedTaskCount += pWorker->GetCompletedTasks();
	m_workers.erase(pWorker);
	if (--m_poolSize == 0)
		TryTerminate();

	m_mainLock.unlock();
}

void thread_pool::TryTerminate()
{
	if (m_poolSize == 0)
	{
		int state = m_runState;
		if (state < STOP && !m_workQueue.empty())
		{
			state = RUNNING; // disable termination check below
			i_thread *pThread = AddThread(NULL);
			if (pThread != NULL)
			{
			}
		}
		if (state == STOP || state == SHUTDOWN)
		{
			m_runState = TERMINATED;
			Terminated();
		}
	}
}

void thread_pool::Shutdown()
{
	m_mainLock.lock();

	int state = m_runState;
	if (state < SHUTDOWN)
		m_runState = SHUTDOWN;

	std::set<Worker *>::iterator iter = m_workers.begin();

	for (; iter != m_workers.end(); ++iter)
	{
		(*iter)->InterruptIfIdle();
	}
	TryTerminate(); // Terminate now if pool and queue empty
	m_mainLock.unlock();
}

std::list<i_runnable *> thread_pool::ShutdownNow()
{
	m_mainLock.lock();

	int state = m_runState;
	if (state < STOP)
		m_runState = STOP;

	std::set<Worker *>::iterator iter = m_workers.begin();
	for (; iter != m_workers.end(); ++iter)
	{
		(*iter)->InterruptNow();
	}

	std::list<i_runnable *> tasks = DrainQueue();
	TryTerminate(); // Terminate now if pool and queue empty
	m_mainLock.unlock();
	return tasks;
}

std::list<i_runnable *> thread_pool::DrainQueue()
{
	std::list<i_runnable *> taskList;

	while (!m_workQueue.empty())
	{
		std::deque<i_runnable *>::iterator it = m_workQueue.begin();

		if (it != m_workQueue.end())
		{
			i_runnable *pRunnable = *it;
			if (Erase(m_workQueue, pRunnable))
				taskList.push_back(pRunnable);
			++it;
		}

	}
	return taskList;
}

bool thread_pool::IsShutdown()
{
	return m_runState != RUNNING;
}

/**
 * Returns true if shutdownNow has been invoked but this executor
 * has not completely terminated.
 */
bool thread_pool::IsStopped()
{
	return m_runState == STOP;
}

bool thread_pool::IsTerminating()
{
	int state = m_runState;
	return state == SHUTDOWN || state == STOP;
}

bool thread_pool::IsTerminated()
{
	return m_runState == TERMINATED;
}

bool thread_pool::AwaitTermination(long timeout)
{
	m_mainLock.lock();

	for (;;)
	{
		if (m_runState == TERMINATED)
			return true;
		if (timeout <= 0)
			return false;

		usleep(10000);
		timeout -= 10;
	}

	m_mainLock.unlock();

}

void thread_pool::SetCorePoolSize(unsigned int corePoolSize)
{
	if (corePoolSize < 0)
		throw mix::thread_exception(THREAD_NUMBER + THREAD_THREADPOOL_ILLEGAL_NO, "Illegal Argument");

	m_mainLock.lock();

	int extra = m_corePoolSize - corePoolSize;
	m_corePoolSize = corePoolSize;
	if (extra < 0)
	{
		int n = m_workQueue.size(); // don't add more threads than tasks
		while (extra++ < 0 && n-- > 0 && m_poolSize < corePoolSize)
		{
			i_thread *pThread = AddThread(NULL);
			if (pThread != NULL)
			{
				//pThread->Start();
			}
			else
				break;
		}
	}
	else if (extra > 0 && m_poolSize > corePoolSize)
	{
		std::set<Worker *>::iterator it = m_workers.begin();
		while (it != m_workers.end() && extra-- > 0 && m_poolSize
				> corePoolSize && m_workQueue.size() == m_maximumPoolSize)
		{
			++it;
			(*it)->InterruptIfIdle();
		}
	}

	m_mainLock.unlock();

}

unsigned int thread_pool::GetCorePoolSize()
{
	return m_corePoolSize;
}

bool thread_pool::PrestartCoreThread()
{
	return AddIfUnderCorePoolSize(NULL);
}

int thread_pool::PrestartAllCoreThreads()
{
	int n = 0;
	while (AddIfUnderCorePoolSize(NULL))
		++n;
	return n;
}

bool thread_pool::AllowsCoreThreadTimeOut()
{
	return m_allowCoreThreadTimeOut;
}

void thread_pool::AllowCoreThreadTimeOut(bool value)
{
	if (value && m_keepAliveTime <= 0)
		throw mix::thread_exception(THREAD_NUMBER + THREAD_THREADPOOL_ALIVETIME_NO, "Core threads must have nonzero keep alive times");

	m_allowCoreThreadTimeOut = value;
}

void thread_pool::SetMaximumPoolSize(unsigned int maximumPoolSize)
{
	if (maximumPoolSize <= 0 || maximumPoolSize < m_corePoolSize)
		throw mix::thread_exception(THREAD_NUMBER + THREAD_THREADPOOL_ILLEGAL_NO, "Illegal Argument");

	m_mainLock.lock();

	int extra = m_maximumPoolSize - maximumPoolSize;
	m_maximumPoolSize = maximumPoolSize;
	if (extra > 0 && m_poolSize > maximumPoolSize)
	{

		std::set<Worker *>::iterator it = m_workers.begin();
		while (it != m_workers.end() && extra > 0 && m_poolSize
				> maximumPoolSize)
		{
			(*it)->InterruptIfIdle();
			--extra;
		}

	}

	m_mainLock.unlock();
}

unsigned int thread_pool::GetMaximumPoolSize()
{
	return m_maximumPoolSize;
}

void thread_pool::SetKeepAliveTime(unsigned long time)
{
	if (time < 0)
		throw mix::thread_exception(THREAD_NUMBER + THREAD_THREADPOOL_ILLEGAL_NO, "Illegal Argument");
	if (time == 0 && AllowsCoreThreadTimeOut())
		throw mix::thread_exception(THREAD_NUMBER + THREAD_THREADPOOL_ALIVETIME_NO, "Core threads must have nonzero keep alive times");

	m_keepAliveTime = time;
}

unsigned long thread_pool::GetKeepAliveTime()
{
	return m_keepAliveTime;
}

std::deque<i_runnable *> &thread_pool::GetQueue()
{
	return m_workQueue;
}

std::deque<i_info *> thread_pool::GetQueueInfo()
{
	std::deque<i_info *> qi;
	m_mainLock.lock();

	std::deque<i_runnable *>::iterator itr = m_workQueue.begin();

	for (; itr != m_workQueue.end(); ++itr)
	{
		i_monitor *pMonitor = dynamic_cast<i_monitor *>(*itr);
		if (pMonitor != NULL)
			qi.push_back(pMonitor->Info());
	}

	std::vector<i_runnable *>::iterator vitr = m_pausedQueue.begin();

	for (; vitr != m_pausedQueue.end(); ++vitr)
	{
		i_monitor *pMonitor = dynamic_cast<i_monitor *>(*vitr);
		if (pMonitor != NULL)
		{
			i_info *pInfo = pMonitor->Info();
			pInfo->SetPaused(pMonitor->IsPaused());
			qi.push_back(pInfo);
		}
	}

	m_mainLock.unlock();

	return qi;
}

std::set<i_info *> thread_pool::GetWorkersInfo()
{
	std::set<i_info *> wi;

	m_mainLock.lock();

	std::set<Worker *>::iterator itr = m_workers.begin();

	for (; itr != m_workers.end(); ++itr)
	{
		i_info *pInfo = (*itr)->Info();
		if (pInfo != NULL)
			wi.insert(pInfo);
		else
			wi.insert(new thread_info("[Unknown -1]", "-1", 0, false, 0, false));
	}

	m_mainLock.unlock();

	return wi;
}

bool thread_pool::Have(i_runnable *pTask)
{
	bool bRet = false;

	m_mainLock.lock();
	std::deque<i_runnable *>::iterator itr = m_workQueue.begin();

	for (; itr != m_workQueue.end(); ++itr)
	{
		if (pTask == (*itr))
		{
			bRet = true;
			break;
		}
	}

	m_mainLock.unlock();

	return bRet;
}

bool thread_pool::Remove(i_runnable *pTask)
{
	bool bRet = false;

	m_mainLock.lock();
	bRet = Erase(GetQueue(), pTask);
	if (!bRet)
		bRet = Erase(m_pausedQueue, pTask);

	m_mainLock.unlock();

	return bRet;
}

int thread_pool::PauseTask(i_runnable *pTask)
{
	int nRet = -1;

	m_mainLock.lock();

	bool bRet = Erase(GetQueue(), pTask);
	if (bRet)
	{
		pTask->Pause();
		m_pausedQueue.push_back(pTask);
		nRet = 0;
	}

	m_mainLock.unlock();

	return nRet;
}

int thread_pool::ResumeTask(i_runnable *pTask)
{
	int nRet = -1;

	m_mainLock.lock();

	bool bRet = Erase(m_pausedQueue, pTask);
	if (bRet)
	{
		pTask->Resume();
		GetQueue().push_back(pTask);
		nRet = 0;
	}

	m_mainLock.unlock();

	return nRet;
}

void thread_pool::Purge()
{
	std::deque<i_runnable *>::iterator it = GetQueue().begin();
	while (it != GetQueue().end())
	{
		//		i_runnable *pRunnable = *it;
		/***
		 if (r instanceof Future<?>)
		 {
		 Future<?> c = (Future<?>)r;
		 if (c.isCancelled())
		 it.remove();
		 }
		 ****/
		++it;
	}
}

int thread_pool::GetPoolSize()
{
	return m_poolSize;
}

int thread_pool::GetActiveCount()
{
	m_mainLock.lock();

	int n = 0;

	std::set<Worker *>::const_iterator iter = m_workers.begin();

	for (; iter != m_workers.end(); ++iter)
	{
		if ((*iter)->IsActive())
			++n;
	}

	m_mainLock.unlock();
	return n;
}

int thread_pool::GetLargestPoolSize()
{
	return m_largestPoolSize;
}

long thread_pool::GetTaskCount()
{
	m_mainLock.lock();

	long n = m_completedTaskCount;

	std::set<Worker *>::const_iterator iter = m_workers.begin();

	for (; iter != m_workers.end(); ++iter)
	{
		n += (*iter)->GetCompletedTasks();
		if ((*iter)->IsActive())
			++n;
	}

	m_mainLock.unlock();

	return n + m_workQueue.size();

}

int thread_pool::GetWorkCount()
{
	int nCount = 0;

	m_mainLock.lock();
	nCount = m_workers.size();
	m_mainLock.unlock();

	return nCount;
}

int thread_pool::GetWaitCount()
{
	int nCount = 0;

	m_mainLock.lock();
	nCount = m_workQueue.size();
	m_mainLock.unlock();

	return nCount;
}

bool thread_pool::IsInQueue(i_runnable *pCommand)
{
	m_mainLock.lock();
	std::deque<i_runnable *>::iterator itr = m_workQueue.begin();

	while (itr != m_workQueue.end())
	{
		i_runnable *pLCmd = *itr;
		if (pLCmd == pCommand)
		{
			m_mainLock.unlock();
			return true;
		}
		++itr;
	}

	m_mainLock.unlock();
	return false;
}

bool thread_pool::IsInWorker(i_runnable *pCommand)
{
	m_mainLock.lock();
	std::set<Worker *>::iterator itr = m_workers.begin();

	while (itr != m_workers.end())
	{
		Worker *pWrk = *itr;
		if (pWrk->IsTask(pCommand))
		{
			m_mainLock.unlock();
			return true;
		}
		++itr;
	}

	m_mainLock.unlock();
	return false;
}

void thread_pool::SortQueue()
{
	m_mainLock.lock();

	std::sort(m_workQueue.begin(), m_workQueue.end(), less_runnable);

	m_mainLock.unlock();
}

void thread_pool::SortQueue(i_runnable *pTask)
{
	m_mainLock.lock();

	std::deque<i_runnable *>::iterator itr = m_workQueue.begin();

	bool bHave = false;
	for (; itr != m_workQueue.end(); ++itr)
	{
		if (pTask == (*itr))
		{
			bHave = true;
			break;
		}
	}

	if (bHave)
		std::sort(m_workQueue.begin(), m_workQueue.end(), less_runnable);

	m_mainLock.unlock();
}

bool thread_pool::Join(i_runnable *pTask, unsigned long us_timeout)
{
	unsigned long spead = 0;
	while (IsInQueue(pTask) || IsInWorker(pTask))
	{
		usleep(10000);
		if (us_timeout > 0)
		{
			spead += 10;
			if (spead > us_timeout)
			{
				return false;
			}
		}
	}
	return true;
}

long thread_pool::GetCompletedTaskCount()
{
	m_mainLock.lock();

	long n = m_completedTaskCount;
	std::set<Worker *>::const_iterator iter = m_workers.begin();

	for (; iter != m_workers.end(); ++iter)
	{
		n += (*iter)->GetCompletedTasks();
	}

	m_mainLock.unlock();
	return n;
}

void thread_pool::BeforeExecute(i_thread *pThread, i_runnable *pRunnable)
{
	if (dynamic_cast<i_monitor *>(pRunnable))
	{
	}
}

void thread_pool::AfterExecute(i_runnable *)
{
}

void thread_pool::Terminated()
{
}

}
