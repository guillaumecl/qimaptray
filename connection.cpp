#include "connection.h"

#include <QTimer>

connection::connection(const char *host, const char *login,
					   const char *password)
	: imap_connection(host)
{
	imap_connection.set_message_callback(
		std::bind(&connection::callback, this, std::placeholders::_1));
	if (not imap_connection.login(login, password))
	{
		// todo error or something.
	}
	QTimer::singleShot(0, this, SLOT(start()));
}

void connection::start()
{
	imap_connection.select("INBOX");

	imap_connection.idle();

	QTimer *timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(wait_message()));

	timer->start(0);
}

void connection::callback(unsigned int count)
{
	emit new_unread_count(count,
						  imap_connection.message_count());
}

void connection::wait_message()
{
	imap_connection.wait();
}
