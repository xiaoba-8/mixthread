/*
 * stringutil.cpp
 *
 *  Created on: Mar 7, 2019
 *      Author: xiaoba-8
 */

#include <vector>
#include <locale>
#include <algorithm>
#include <functional>
#include <sstream>
#include <mix/utils/stringutil.h>

namespace mix
{

void split(std::string s, std::string &delim, std::vector<std::string> &ret)
{
	size_t index = s.find_first_of(delim, 0);

	while (index != std::string::npos)
	{
		std::string ts = s.substr(0, index);
		ts = trim(ts);
		if (ts.length() > 0)
			ret.push_back(ts);
		s = s.substr(index + 1, s.size()-index - 1);
		s = trim(s);
		index = s.find_first_of(delim, 0);
	}

	if (s.size() > 0)
		ret.push_back(s);
}

void split(std::string s, const char *delim, std::vector<std::string> &ret)
{
	size_t index = s.find_first_of(delim, 0);

	while (index != std::string::npos)
	{
		std::string ts = s.substr(0, index);
		ts = trim(ts);
		if (ts.length() > 0)
			ret.push_back(ts);
		s = s.substr(index + 1, s.size()-index - 1);
		s = trim(s);
		index = s.find_first_of(delim, 0);
	}

	if (s.size() > 0)
		ret.push_back(s);
}

std::string combine2string(std::vector<std::string> &ret, const char *delim)
{
	bool first = true;
	std::stringstream ss;
	std::vector<std::string>::iterator itr = ret.begin();
	for (; itr != ret.end(); ++itr)
	{
		if (first)
		{
			first = false;
			ss << *itr;
		}
		else
		{
			ss << delim << *itr;
		}
	}
	return ss.str();
}

std::string replace(std::string src, const char *token, const char *val)
{
	std::stringstream ss;
	size_t index = src.find_first_of(token, 0);

	while (index != std::string::npos)
	{
		std::string ts = src.substr(0, index);
		if (ts.length() > 0)
			ss << ts << val;
		src = src.substr(index + 1, src.size()-index - 1);
		index = src.find_first_of(token, 0);
	}

	if (src.size() > 0)
		ss << src;

	return ss.str();
}

}
