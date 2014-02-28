#include <stdio.h>

#include "connection.h"
#include "tray.h"

#include <QApplication>
#include <QThread>

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	char *host;
	char *user;
	char *password;

	if (argc == 1)
	{
		host = (char*)alloca(256);
		user = (char*)alloca(256);
		password = (char*)alloca(256);
		fscanf(stdin, "%255s %255s %255s", host, user, password);
	}
	else if (argc < 4)
	{
		fprintf(stderr, "usage : %s server user pass\n", argv[0]);
		return 1;
	}
	else
	{
		host = argv[1];
		user = argv[2];
		password = argv[3];
	}

	QThread thread;

	tray t;
	connection c(host, user, password);

	// don't keep the password stored in RAM
	memset(password, 0, strlen(password));

	c.moveToThread(&thread);
	QObject::connect(&c, SIGNAL(new_unread_count(unsigned int, unsigned int)),
					 &t, SLOT(unread_messages(unsigned int, unsigned int)));

	thread.start();

	int r = app.exec();

	// thread.quit();
	// thread.wait();

	return r;
}
