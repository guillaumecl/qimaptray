#include <stdio.h>
#include <unistd.h>

#include "connection.h"
#include "tray.h"

#include <QApplication>
#include <QThread>
#include <QMessageBox>

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	try
	{
		char *host;
		char *user;
		char *password;

		if (argc == 1)
		{
			host = (char*)alloca(256);
			user = (char*)alloca(256);
			password = (char*)alloca(256);
			if (fscanf(stdin, "%255s %255s %255s", host, user, password) != 3)
				return 1;
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

		qimaptray::tray t;
		qimaptray::connection c(host, user, password);

		// don't keep the password stored in RAM
		memset(password, 0, strlen(password));

		c.moveToThread(&thread);
		QObject::connect(&c, SIGNAL(new_unread_count(unsigned int, unsigned int)),
				 &t, SLOT(unread_messages(unsigned int, unsigned int)));
		QObject::connect(&c, SIGNAL(connected()), &t, SLOT(connected()));
		QObject::connect(&c, SIGNAL(disconnected()), &t, SLOT(disconnected()));
		QObject::connect(&c, SIGNAL(cannot_login()), &t, SLOT(cannot_login()));

		thread.start();
		int ret = app.exec();

		c.stop();

		thread.exit();
		thread.wait();

		return ret;
	}
	catch(const std::exception&e)
	{
		if (not isatty(fileno(stdin)))
		{
			QMessageBox::critical(nullptr, "qimaptray", QString::fromStdString(e.what()));
		}
	}
	catch(const char *e)
	{
		if (not isatty(fileno(stdin)))
		{
			QMessageBox::critical(nullptr, "qimaptray", e);
		}
	}

	return 1;
}
