/*
 * i_monitor.h
 *
 *  Created on: 2019-5-18
 *      Author: xiaoba-8
 */

#ifndef IMONITOR_H_
#define IMONITOR_H_

#include <string>
#include "i_runnable.h"
#include "../mutex/thread_mutex.h"

namespace mix
{

class i_info
{
public:
	virtual ~i_info() {}
	virtual std::string &what() = 0;
	virtual unsigned long id() = 0;
	virtual void SetId(unsigned long id) = 0;
	virtual std::string GetTaskId() = 0;
	virtual int GetIntTaskId() = 0;
	virtual int GetPriority() = 0;
	virtual bool IsCanceled() = 0;
	virtual bool IsPaused() = 0;
	virtual void SetPaused(bool paused) = 0;
	virtual bool IsSuspend() = 0;
	virtual void SetSuspend(bool suspend) = 0;
	virtual void UpdateInfo(std::string info) = 0;
	virtual void UpdateTaskId(std::string taskId) = 0;
	virtual int GetStatus() = 0;
	virtual void SetStatus(int nStatus) = 0;
};

class i_monitor : public i_runnable
{
public:
	i_monitor(std::string name) : i_runnable(name) {}
	virtual ~i_monitor() {}
	virtual i_info *Info() = 0;
	virtual void TryPause() = 0;
};

class thread_info : public i_info
{
public:
	thread_info(std::string info, std::string taskId, int priority,
			bool bCanceled, unsigned long id, int nStatus, bool paused = false);
	thread_info(std::string info, int taskId, int priority,
			bool bCanceled, unsigned long id, int nStatus, bool paused = false);
	virtual ~thread_info();
	virtual std::string &what();
	virtual unsigned long id();
	virtual void SetId(unsigned long id);
	virtual std::string GetTaskId();
	virtual int GetIntTaskId();
	virtual int GetPriority();
	virtual bool IsCanceled();
	virtual bool IsPaused();
	virtual void SetPaused(bool paused);
	virtual bool IsSuspend();
	virtual void SetSuspend(bool suspend);
	virtual void UpdateInfo(std::string info);
	virtual void UpdateTaskId(std::string taskId);
	virtual int GetStatus();
	virtual void SetStatus(int nStatus);
private:
	std::string m_info;
	unsigned long m_id;
	std::string m_taskId;
	int m_priority;
	int m_nStatus;
	bool m_bCanceled;
	bool m_bPaused;
	bool m_bSuspend;
};

class root_monitor : public i_monitor
{
public:
	root_monitor(std::string name);
	virtual ~root_monitor();
	virtual int Priority();
	virtual void SetPriority(int priority);

	virtual void TryPause();

	virtual void Pause();
	virtual void Resume();
	virtual bool IsPaused();

	virtual void Cancel() {  }
	virtual bool IsCanceled() { return false;}

	virtual std::string GetTaskId();

private:
	bool m_bPaused;
	int m_priority;
};

}

#endif /* IMONITOR_H_ */
