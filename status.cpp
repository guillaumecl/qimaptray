#include "status.h"

#include <string.h>

using namespace imappp;

status::status() :
	response_(ok)
{
}

void status::parse(const char *p, std::function<void(const status&)> visitor)
{
	status current;
	unsigned int read;

	while (*p)
	{
		current = status();

		if (*p == '*' or *p == '+')
		{
			p++;
			current.tag_ = -1;
		}
		else
		{
			if (sscanf(p, "%u%n", &current.tag_, &read))
			{
				p += read;
			}
		}

		while (*p and *p == ' ')
		{
			p++;
		}

		const char *err = p;
		while (*p and *p != ' ' and *p != '\r' and *p != '\n')
		{
			p++;
		}
		int err_len = p - err;

		if (strncmp(err, "OK", err_len) == 0)
		{
			current.response_ = ok;
		}
		else if (strncmp(err, "NO", err_len) == 0)
		{
			current.response_ = no;
		}
		else if (strncmp(err, "BAD", err_len) == 0)
		{
			current.response_ = bad;
		}
		else
		{
			current.response_ = ok;
			p = err;
		}

		while (*p and *p == ' ')
		{
			p++;
		}

		const char *message = p;

		while(*p and *p != '\n' and *p != '\r')
		{
			p++;
		}

		current.message_ = std::string(message, p - message);
		visitor(current);

		while (*p and (*p == '\n' or *p == '\r' or *p == ' '))
		{
			p++;
		}
	}
}
