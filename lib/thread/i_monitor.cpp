/*
 * i_monitor.cpp
 *
 *  Created on: 2019-5-18
 *      Author: xiaoba-8
 */

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include <mix/thread/i_monitor.h>
#include <mix/thread/mix_thread_container.h>

namespace mix
{
thread_info::thread_info(std::string info, std::string taskId, int priority,
		bool bCanceled, unsigned long id, int nStatus, bool paused) : m_info(info),
		m_id(id), m_taskId(taskId),	m_priority(priority),
		m_nStatus(nStatus),
		m_bCanceled(bCanceled), m_bPaused(paused)
{
}
thread_info::thread_info(std::string info, int taskId, int priority,
		bool bCanceled, unsigned long id, int nStatus, bool paused) : m_info(info),
		m_id(id), 	m_priority(priority),
		m_nStatus(nStatus),
		m_bCanceled(bCanceled), m_bPaused(paused)
{
	char tid[32];
	sprintf(tid, "%d", taskId);
	m_taskId = tid;
}
thread_info::~thread_info()
{}
std::string &thread_info::what()
{
	return m_info;
}
unsigned long thread_info::id()
{
	return m_id;
}
void thread_info::SetId(unsigned long id)
{
	m_id = id;
}
std::string thread_info::GetTaskId()
{
	return m_taskId;
}
int thread_info::GetIntTaskId()
{
	return atoi(m_taskId.c_str());
}
int thread_info::GetPriority()
{
	return m_priority;
}
bool thread_info::IsCanceled()
{
	return m_bCanceled;
}
bool thread_info::IsPaused()
{
	return m_bPaused;
}
void thread_info::SetPaused(bool paused)
{
	m_bPaused = paused;
}
bool thread_info::IsSuspend()
{
	return m_bSuspend;
}
void thread_info::SetSuspend(bool suspend)
{
	m_bSuspend = suspend;
}
int thread_info::GetStatus()
{
	return m_nStatus;
}
void thread_info::SetStatus(int nStatus)
{
	m_nStatus = nStatus;
}
void thread_info::UpdateInfo(std::string info)
{
	m_info = info;
}
void thread_info::UpdateTaskId(std::string taskId)
{
	m_taskId = taskId;
}

root_monitor::root_monitor(std::string name) : i_monitor(name), m_priority(NORM_PRIORITY)
{
	mix_thread_container::SGetInstance()->Register(this);
}
root_monitor::~root_monitor()
{
	mix_thread_container::SGetInstance()->Unregister(this);
}
int root_monitor::Priority()
{
	return m_priority;
}

void root_monitor::SetPriority(int priority)
{
	m_priority = priority;
}
void root_monitor::TryPause()
{
	mix_thread_container::SGetInstance()->TryPause();
}

void root_monitor::Pause()
{
	m_bPaused = true;
}

void root_monitor::Resume()
{
	m_bPaused = false;
}

bool root_monitor::IsPaused()
{
	return m_bPaused;
}

std::string root_monitor::GetTaskId()
{
	return "0";
}

}
