#include <stdio.h>

#include "connection.h"
#include "tray.h"

#include <QApplication>
#include <QThread>

int main(int argc, char **argv)
{
	QApplication app(argc, argv);

	if (argc < 4)
	{
		fprintf(stderr, "usage : %s server user pass\n", argv[0]);
		return 1;
	}

	QThread thread;

	tray t;
	connection c(argv[1], argv[2], argv[3]);

	c.moveToThread(&thread);
	QObject::connect(&c, SIGNAL(new_unread_count(unsigned int, unsigned int)),
					 &t, SLOT(unread_messages(unsigned int, unsigned int)));

	thread.start();

	int r = app.exec();

	// thread.quit();
	// thread.wait();

	return r;
}
