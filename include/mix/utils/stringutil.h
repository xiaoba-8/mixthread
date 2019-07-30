/*
 * stringutil.h
 *
 *  Created on: Mar 7, 2019
 *      Author: xiaoba-8
 */

#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_

#include <string>
#include <locale>
#include <algorithm>
#include <functional>
#include <vector>

namespace mix
{

inline std::string &leftTrim(std::string &ss)
{
	std::locale loc;

	std::string::iterator iter = std::find_if(ss.begin(), ss.end(),
			std::not1(std::bind2nd(std::ptr_fun(std::isspace<char>), loc)));

	ss.erase(ss.begin(), iter);
	return ss;
}

inline std::string &rightTrim(std::string &ss)
{
	std::locale loc;

	std::string::reverse_iterator iter = std::find_if(ss.rbegin(), ss.rend(),
			std::not1(std::bind2nd(std::ptr_fun(std::isspace<char>), loc)));

	ss.erase(iter.base(), ss.end());
	return ss;
}

inline std::string & trim(std::string &ss)
{
	return leftTrim(rightTrim(ss));
}

inline bool startWith(std::string src, std::string token)
{
	return src.substr(0, token.size()) == token;
}

inline bool endWith(std::string src, std::string token)
{
	return src.size() > token.size() && (src.substr(src.size()-token.size(), src.size()) == token);
}

inline std::string toUpcase(std::string src)
{
	std::string dst = src;
	std::transform(dst.begin(), dst.end(), dst.begin(), toupper);
	return dst;
}

inline std::string toLowcase(std::string src)
{
	std::string dst = src;
	std::transform(dst.begin(), dst.end(), dst.begin(), tolower);
	return dst;
}

void split(std::string s, std::string &delim, std::vector<std::string> &ret);
void split(std::string s, const char *delim, std::vector<std::string> &ret);

std::string combine2string(std::vector<std::string> &ret, const char *delim);

std::string replace(std::string src, const char *token, const char *val);
}

#endif /* STRINGUTIL_H_ */
