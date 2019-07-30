/*
 * thread_exception.h
 *
 *  Created on: 2019-4-2
 *      Author: xiaoba-8
 */

#ifndef THREAD_EXCEPTION_H_
#define THREAD_EXCEPTION_H_

#include <stdexcept>
#include <stdarg.h>

namespace mix
{

class thread_exception : public std::runtime_error
{
public:
    explicit thread_exception();
    explicit thread_exception(int errorno);
    explicit thread_exception(int errorno, const char *format, ...);
    explicit thread_exception(const char *format, ...);
    explicit thread_exception(std::string arg);
	virtual ~thread_exception() throw ();

	virtual const char * what() const throw();
	virtual int GetErrNo();
	virtual const char * GetErrorMsg() const throw();
protected:
	std::string m_what;
	std::string m_errorMsg;
	int m_errno;
};

}

#endif /* THREAD_EXCEPTION_H_ */
