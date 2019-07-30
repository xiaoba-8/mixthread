/*
 * main.cpp
 *
 *  Created on: Jul 19, 2019
 *      Author: xiaoba-8
 */

#include <stdio.h>

#include <mix/thread/thread_pool.h>

class DemoThread : public mix::i_runnable
{
private:
	int id;
public:
	DemoThread(int id)
	{
		this->id = id;
	}

	virtual void Run()
	{
		for (int i = 0; i < 3; i++)
		{
			usleep(500);
			printf("Thread %d: loop index %d\n", id, i);
		}
	}

	virtual bool IsDeleteOnFinish()
	{
		return true;
	}

	virtual int Priority()
	{
		return 0;
	}

	virtual void SetPriority(int priority)
	{

	}

	virtual void Cancel()
	{

	}
	virtual bool IsCanceled()
	{
		return false;
	}

	virtual void Pause() {}
	virtual void Resume() {}
	virtual bool IsPaused()
	{
		return false;
	}

	virtual std::string GetTaskId()
	{
		return "";
	}

	virtual void TryPause()
	{

	}
};

int main(int argc, char *argv[])
{
	mix::thread_pool thread_pool(10, 20, 5);

	printf("Mix Thread Begin\n");

	for (int i = 0; i < 5; i++)
	{
		thread_pool.Execute(new DemoThread(i));
	}

	sleep(1);

	while (thread_pool.GetActiveCount() > 0)
	{
		sleep(1);
	}

	printf("Mix Thread End\n");
	return 0;
}
