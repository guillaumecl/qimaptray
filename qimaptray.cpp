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
	if (argc < 3)
	{
		fprintf(stderr, "usage : %s user pass\n", argv[0]);
		return 1;
	}

	signal(SIGINT, logout);

	try
	{
		imappp::imap i("mail.gandi.net", true);
		conn = &i;

		if (i.login(argv[1], argv[2]))
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
