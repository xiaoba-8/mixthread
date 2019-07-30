/*
 * thread_xception.cpp
 *
 *  Created on: 2019-4-2
 *      Author: xiaoba-8
 */

#include <mix/utils/thread_exception.h>

#include <stdio.h>

namespace mix
{

thread_exception::thread_exception() : std::runtime_error(""), m_errno(0)
{

}

thread_exception::thread_exception(int errorno) : std::runtime_error(""), m_errno(errorno)
{
	char msg[512];
	sprintf(msg, "Error Code : %06d", m_errno);
	m_what.assign(msg);
}

thread_exception::thread_exception(int errorno, const char *format, ...) :
		std::runtime_error(format), m_errno(errorno)
{
	va_list list;
	va_start(list, format);
	char msg[512];
	vsprintf(msg, format, list);
	va_end(list);

	m_errorMsg.assign(msg);

	sprintf(msg, "Error Code : %06d, Error Msg : %s", m_errno, m_errorMsg.c_str());
	m_what.assign(msg);
}

thread_exception::thread_exception(std::string arg) :
		std::runtime_error(arg), m_errno(0)
{
	char msg[512];
	sprintf(msg, "Error Code : %06d, Error Msg : %s", m_errno, arg.c_str());
	m_what.assign(msg);
}

thread_exception::thread_exception(const char *format, ...) :
		std::runtime_error(format), m_errno(0)
{
	va_list list;
	va_start(list, format);
	char msg[512];
	vsprintf(msg, format, list);
	va_end(list);

	m_errorMsg.assign(msg);

	sprintf(msg, "Error Code : %06d, Error Msg : %s", m_errno, m_errorMsg.c_str());
	m_what.assign(msg);
}

thread_exception::~thread_exception() throw ()
{
	// TODO Auto-generated destructor stub
}

const char *thread_exception::what() const throw ()
{
	return m_what.c_str();
}

int thread_exception::GetErrNo()
{
	return m_errno;
}


const char *thread_exception::GetErrorMsg() const throw ()
{
	if (m_errorMsg.size() == 0)
		return std::runtime_error::what();
	else
		return m_errorMsg.c_str();
}

}
