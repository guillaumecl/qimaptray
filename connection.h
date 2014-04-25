#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>

#include "imap.h"

namespace qimaptray
{

class connection: public QObject
{
	Q_OBJECT
public:
	connection(const char *host, const char *login,
			   const char *password);
	void stop();

signals:
	void new_unread_count(unsigned int unread_count, unsigned int total);
	void cannot_login();
	void connected();
	void disconnected();

private slots:
	void wait_message();
	void login();
	void start();
	void try_connect();

private:
	void callback(unsigned int unread_count);
	void connection_callback(bool connected);

	imappp::imap imap_connection;
	std::string login_;
	std::string password_;
};

}

#endif
