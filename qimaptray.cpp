#include <stdio.h>

#include "imap.h"

#include <signal.h>


imappp::imap *volatile conn = nullptr;


static void logout(int)
{
	if (conn)
	{
		conn->logout();
	}
	else
	{
		abort();
	}
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		fprintf(stderr, "usage : %s server user pass\n", argv[0]);
		return 1;
	}

	signal(SIGINT, logout);

	try
	{
		imappp::imap i(argv[1], true);

		i.set_message_callback(
			[](unsigned int count)
			{
				printf("Currently we have %u unread messages.\n", count);
			});

		conn = &i;

		if (i.login(argv[2], argv[3]))
		{
			i.select("INBOX");

			i.idle();

			do
			{
			}
			while (i.wait());
		}

		conn = nullptr;
	}
	catch (const char *str)
	{
		fprintf(stderr, "Error: %s\n", str);
	}
	return 0;
}
