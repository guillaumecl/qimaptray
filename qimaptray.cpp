#include <stdio.h>

#include "imap.h"

int main(int argc, char **argv)
{
	if (argc < 3)
	{
		fprintf(stderr, "usage : %s user pass\n", argv[0]);
		return 1;
	}

	try
	{
		char buffer[2048];
		imappp::imap i("mail.gandi.net");

		i.login(argv[1], argv[2]);

		i.send_raw("SELECT INBOX");
		i.receive(buffer, sizeof(buffer));
		printf("%s\n", buffer);

		i.send_raw("IDLE");


		int len;

		do
		{
			len = i.receive(buffer, sizeof(buffer));
			buffer[len] = 0;
			printf("%s\n", buffer);
		} while (len > 0);
	}
	catch (const char *str)
	{
		fprintf(stderr, "Error: %s\n", str);
	}
	return 0;
}
