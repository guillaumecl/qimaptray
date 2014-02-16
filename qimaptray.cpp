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


void wait_messages(const char *host, const char *login, const char *password, imappp::imap::receive_message_callback callback)
{
	try
	{
		imappp::imap i(host, true);

		i.set_message_callback(callback);

		conn = &i;

		if (i.login(login, password))
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
}

int main(int argc, char **argv)
{
	if (argc < 4)
	{
		fprintf(stderr, "usage : %s server user pass\n", argv[0]);
		return 1;
	}

	signal(SIGINT, logout);

	wait_messages(argv[1], argv[2], argv[3],
		[](unsigned int count)
		{
			printf("Currently we have %u unread messages.\n", count);
		});

	return 0;
}
