/*
 * base_errno.h
 *
 *  Created on: 2019-6-21
 *      Author: xiaoba-8
 */

#ifndef THREAD_ERRNO_H_
#define THREAD_ERRNO_H_

#define THREAD_PREFIX	0
#define THREAD_NUMBER	10000

#define THREAD_PTHREAD_SET_SCHED_NO				1
#define THREAD_PTHREAD_SET_SCHED_MSG				"pthread_setschedparam error [%s]"

#define THREAD_PTHREAD_GET_SCHED_NO				2
#define THREAD_PTHREAD_GET_SCHED_MSG				"pthread_getschedparam error [%s]"

#define THREAD_PTHREAD_SET_PRIO_NO					3
#define THREAD_PTHREAD_SET_PRIO_MSG				"pthread_setschedprio error [%s]"

#define THREAD_THREADPOOL_ILLEGAL_NO				4
#define THREAD_THREADPOOL_ILLEGAL_MSG				"Illegal Argument"

#define THREAD_THREADPOOL_NULL_NO					5
#define THREAD_THREADPOOL_NULL_MSG					"NULL Command"

#define THREAD_THREADPOOL_REJECT_NO				6
#define THREAD_THREADPOOL_REJECT_MSG				"Command is rejected for [%d %d] [%d] [%s]"

#define THREAD_THREADPOOL_ALIVETIME_NO			7
#define THREAD_THREADPOOL_ALIVETIME_MSG			"Core threads must have nonzero keep alive times"

#endif /* BASE_ERRNO_H_ */
