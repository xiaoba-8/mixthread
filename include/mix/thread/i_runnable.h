/*
 * i_runnable.h
 *
 *  Created on: 2019-4-15
 *      Author: xiaoba-8
 *
 */

#ifndef IRUNNABLE_H_
#define IRUNNABLE_H_

#include <string>

namespace mix
{

#define TASK_ID	"task_id"

#define MAX_PRIORITY			20
#define MIN_PRIORITY			0
#define NORM_PRIORITY		0

class i_runnable
{
public:
	i_runnable();
	i_runnable(std::string name);
	virtual ~i_runnable();

	virtual void Run() = 0;

	virtual bool IsDeleteOnFinish() = 0;

	virtual int Priority() = 0;
	virtual void SetPriority(int priority) = 0;

	virtual void Cancel() = 0;
	virtual bool IsCanceled() = 0;

	virtual void Pause() = 0;
	virtual void Resume() = 0;
	virtual bool IsPaused() = 0;

	virtual std::string GetTaskId() = 0;

	virtual void TryPause() = 0;

	virtual unsigned long GetCreateTime();
	virtual std::string GetRunableName() { return m_name; }
private:
	unsigned long m_ct;
	std::string m_name;
};

class i_thread : public i_runnable
{
public:
	i_thread() {}
	i_thread(std::string name) : i_runnable(name) {}
	virtual ~i_thread() {}

	virtual bool Start() = 0;
	virtual void Stop() = 0;

	virtual unsigned long int GetId() = 0;

	virtual void Join() = 0;
	virtual void Join(unsigned long millisTime) = 0;

	virtual void Suspend() = 0;
	virtual void Continue() = 0;
	virtual bool IsSuspend() = 0;

	virtual void SetPriority(int priority) = 0;
	virtual int GetPriority() = 0;

};

class root_runnable : public i_runnable
{
public:
	root_runnable();
	root_runnable(std::string name);
	virtual ~root_runnable();
	virtual int Priority();
	virtual void SetPriority(int priority);

	virtual void TryPause();

	virtual void Pause();
	virtual void Resume();
	virtual bool IsPaused();

	virtual void Cancel() {  }

	virtual bool IsCanceled() { return false;}

	virtual std::string GetTaskId() {return "";}
protected:
	bool m_pause;
	int m_priority;
};

class root_thread : public i_thread
{
public:
	root_thread();
	root_thread(std::string name);
	virtual ~root_thread();
	virtual int Priority();
	virtual void SetPriority(int priority);

	virtual void Pause();
	virtual void Resume();
	virtual bool IsPaused();

	virtual void Suspend();
	virtual void Continue();
	virtual bool IsSuspend();

	virtual void TryPause();

	virtual unsigned long int GetId();

	virtual void Cancel() {  }
	virtual bool IsCanceled() { return false;}

	virtual std::string GetTaskId() {return "";}

protected:

	int m_priority;
	bool m_pause;
	bool m_suspend;
	pthread_mutex_t m_mutex;
	pthread_cond_t m_cond;

	unsigned long int m_threadId;
};

bool less_runnable(i_runnable *m1, i_runnable *m2);

}

#endif /* IRUNNABLE_H_ */
