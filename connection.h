#ifndef CONNECTION_H
#define CONNECTION_H

#include <QObject>

#include "imap.h"

class connection: public QObject
{
	Q_OBJECT
public:
	connection(const char *host, const char *login,
			   const char *password);

signals:
	void new_unread_count(unsigned int unread_count, unsigned int total);

private slots:
	void wait_message();
	void start();

private:
	void callback(unsigned int unread_count);

	imappp::imap imap_connection;
};

#endif
