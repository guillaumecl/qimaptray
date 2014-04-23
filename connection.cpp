#include "connection.h"

#include <QTimer>

namespace qimaptray
{

connection::connection(const char *host, const char *login,
					   const char *password)
	: imap_connection(host)
	, login_(login)
	, password_(password)
{
	imap_connection.set_message_callback(
		std::bind(&connection::callback, this, std::placeholders::_1));
	imap_connection.set_connection_callback(
		std::bind(&connection::connection_callback, this, std::placeholders::_1));

	QTimer::singleShot(0, this, SLOT(try_connect()));
}

void connection::try_connect()
{
	imap_connection.connect();
}

void connection::login()
{
	if (imap_connection.login(login_.c_str(), password_.c_str()))
		QTimer::singleShot(0, this, SLOT(start()));
	else
		emit cannot_login();
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

void connection::connection_callback(bool is_connected)
{
	if (is_connected)
	{
		login();
		emit connected();
	}
	else
	{
		emit disconnected();
		QTimer::singleShot(0, this, SLOT(try_connect()));
	}
}

void connection::wait_message()
{
	imap_connection.wait();
}

}
