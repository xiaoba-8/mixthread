/*
 * thread_proxy.h
 *
 *  Created on: 2019-5-20
 *      Author: xiaoba-8
 */

#ifndef DEFAULTTHREAD_H_
#define DEFAULTTHREAD_H_

#include <set>

#include "mix_thread.h"

namespace mix
{

class thread_proxy : public mix_thread
{
private:
	i_runnable *m_pRunnable;
	bool m_releaseOnExit; // flag to delete m_pRunnable where the instance is destroyed.
public:
	thread_proxy(i_runnable *pRunnable, bool releaseOnExit = false, bool bDeleteOnFinish = false);
	virtual ~thread_proxy();

	virtual void Run();

	virtual void Cancel();
	virtual bool IsCanceled();
	virtual std::string GetTaskId();

	virtual std::string GetRunableName();
};
}

#endif /* DEFAULTTHREAD_H_ */
