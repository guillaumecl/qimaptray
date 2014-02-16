#include "status.h"

#include <string.h>

using namespace imappp;

status::status() :
	tag_(0),
	num_(0),
	response_(untagged),
	command_(unknown)
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
			current.response_ = untagged;
			p = err;
		}

		while (*p and *p == ' ')
		{
			p++;
		}

		if (current.response_ == untagged and sscanf(p, "%u%n", &current.num_, &read))
		{
			p += read+1;
			const char *command = p;

			while(*p and *p != '\n' and *p != '\r' and *p != ' ')
			{
				p++;
			}

			int len = p - command;

			if (strncmp(command, "EXPUNGE", len) == 0)
			{
				current.command_ = expunge;
			}
			else if (strncmp(command, "RECENT", len) == 0)
			{
				current.command_ = recent;
			}
			else if (strncmp(command, "EXISTS", len) == 0)
			{
				current.command_ = exists;
			}
			else if (strncmp(command, "FETCH", len) == 0)
			{
				current.command_ = fetch;
			}
			else
			{
				current.command_ = unknown;
				current.message_ = std::string(command, p - command);
			}
		}
		else if (strncmp(p, "SEARCH", 6) == 0)
		{
			unsigned int id;
			p += 7;
			current.num_ = 0;
			current.command_ = search;
			while (*p and *p != '\r' and *p != '\n' and  sscanf(p, "%u%n", &id, &read))
			{
				current.num_++;
				p += read;
				while(*p == ' ')
				{
					p++;
				}
			}
		}
		else
		{
			const char *message = p;

			while(*p and *p != '\n' and *p != '\r')
			{
				p++;
			}

			current.message_ = std::string(message, p - message);
		}
		if (visitor)
		{
			visitor(current);
		}

		while (*p and (*p == '\n' or *p == '\r' or *p == ' '))
		{
			p++;
		}
	}
}
