/*
 * thread_pool.h
 *
 *  Created on: 2019-3-31
 *      Author: xiaoba-8
 */

#ifndef THREAD_POOL_H_
#define THREAD_POOL_H_

#include <queue>
#include <vector>
#include <set>
#include <list>
#include <deque>

#include <cstddef>

#include "i_monitor.h"
#include "thread_proxy.h"
#include "../mutex/thread_mutex.h"

namespace mix
{

void suspend_handler(int signum);
void resume_handler(int signum);
void InitialThreadStatus();

class thread_pool
{
public:
private:
	volatile int m_runState;
	static const int RUNNING = 0;
	static const int SHUTDOWN = 1;
	static const int STOP = 2;
	static const int TERMINATED = 3;

	std::deque<i_runnable *> m_workQueue;
	std::vector<i_runnable *> m_pausedQueue;

	class Worker;
	thread_mutex m_mainLock;

	std::set<Worker *> m_workers;

	volatile unsigned long m_keepAliveTime;
	volatile bool m_allowCoreThreadTimeOut;
	unsigned int m_corePoolSize;
	volatile unsigned int m_maximumPoolSize;
	volatile unsigned int m_poolSize;

	int m_largestPoolSize;
	long m_completedTaskCount;


public:
	thread_pool(unsigned int corePoolSize, unsigned int maximumPoolSize, unsigned long keepAliveTime = 100);
	virtual ~thread_pool();

	void Terminate();

	void Execute(i_runnable *pCommand);

	void SetCorePoolSize(unsigned int corePoolSize);
	unsigned int GetCorePoolSize();

	bool PrestartCoreThread();
	int PrestartAllCoreThreads();
	bool AllowsCoreThreadTimeOut();
	void AllowCoreThreadTimeOut(bool value);
	void SetMaximumPoolSize(unsigned int maximumPoolSize);
	unsigned int GetMaximumPoolSize();
	void SetKeepAliveTime(unsigned long time);
	unsigned long GetKeepAliveTime();

	std::deque<i_runnable *> &GetQueue();

	std::set<i_info *> GetWorkersInfo();
	std::deque<i_info *> GetQueueInfo();

	bool Have(i_runnable *pTask);
	bool Remove(i_runnable *pTask);
	void Purge();
	int GetPoolSize();

	int GetActiveCount();
	int GetLargestPoolSize();
	long GetTaskCount();
	long GetCompletedTaskCount();
	int GetWorkCount();
	int GetWaitCount();

	bool Join(i_runnable *pTask, unsigned long us_timeout = 0);

	int TerminateThread(unsigned long id);
	int CancelThread(unsigned long id);
	int CancelThread(i_runnable *pRuanble);
	int SetPriority(unsigned long id, int priority);

	int PauseThread(unsigned long id);
	int ResumeThread(unsigned long id);

	int SuspendThread(unsigned long id);
	int ContinueThread(unsigned long id);

	void SortQueue();
	void SortQueue(i_runnable *pTask);

	int PauseTask(i_runnable *pTask);
	int ResumeTask(i_runnable *pTask);

private:
	i_thread *AddThread(i_runnable *pFirstTask);

	void AddToWorkQueueByPriority(i_runnable *pTask);
	bool AddIfUnderCorePoolSize(i_runnable *pFirstTask);

	void EnsureQueuedTaskHandled(i_runnable *pFirstTask);

	void Reject(i_runnable *pFirstTask, const char *fmt="Full and Rejected");
	i_runnable *GetTask();

	bool WorkerCanExit();
	void InterruptIdleWorkers();
	void WorkerDone(Worker *pw);
	void TryTerminate();

public:
	void Shutdown();

	std::list<i_runnable *> ShutdownNow();
private:
	std::list<i_runnable *> DrainQueue();
	bool IsShutdown();
	bool IsStopped();

	bool IsTerminating();
	bool IsTerminated();
	bool AwaitTermination(long timeout);

	bool Erase(std::deque<i_runnable *> &, i_runnable *pCommand);
	bool Erase(std::vector<i_runnable *> &, i_runnable *pCommand);
	bool IsInQueue(i_runnable *pCommand);
	bool IsInWorker(i_runnable *pCommand);

protected:
	void BeforeExecute(i_thread *pt, i_runnable *pr);
	void AfterExecute(i_runnable *pr);
	void Terminated();

};

class thread_pool::Worker : public root_monitor
{
private:
	i_runnable *m_pFirstTask;
	i_runnable *m_pCurrentTask;

	volatile long m_completedTasks;
	bool m_active;
	bool m_canceled;
	int m_nStatus;
	thread_pool *m_pPool;

	i_thread *m_pThread;

	/**
	 * Runs a single task between before/after methods.
	 */
	void RunTask(i_runnable *pTask);

	/**
	 * Main run loop
	 */
public:
	Worker(i_runnable *pFirstTask, thread_pool *);
	~Worker();

	void SetThread(i_thread *pThread) { m_pThread = pThread; }
	i_thread *GetThread() { return m_pThread; }
	long GetCompletedTasks() { return m_completedTasks; }

	void InterruptIfIdle();
	void InterruptNow();

	bool IsActive();
	bool IsTask(i_runnable *pTask);
	bool GetNextTask();

	i_info *Info();

	virtual void Run();

	virtual bool IsDeleteOnFinish() { return false; }
	virtual int Priority();
	virtual void SetPriority(int priority);

	virtual void Cancel();
	virtual bool IsCanceled();
	virtual std::string GetTaskId();
};

}
#endif /* THREADPOOL_H_ */
